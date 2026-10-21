#include "gui/servidor_http.h"
#include "gui/gerador_html.h"
#include <thread>
#include <atomic>
#include <sstream>
#include <cstring>
#include <iostream>

// Servidor construído sem dependências em bibliotecas de terceiros
// Usa apenas sockets POSIX/Winsock

// Detecção de plataforma para sockets.

#if defined(_WIN32) || defined(_WIN64)
    #define PLATAFORMA_WINDOWS
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    using SocketType = SOCKET;
    constexpr SocketType SOCKET_INVALIDO = INVALID_SOCKET;
    #define FECHAR_SOCKET closesocket
#else
    #define PLATAFORMA_POSIX
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
    using SocketType = int;
    constexpr SocketType SOCKET_INVALIDO = -1;
    #define FECHAR_SOCKET close
#endif

#include <chrono>


// Inicialização/finalização do Winsock (Windows).

#ifdef PLATAFORMA_WINDOWS
static bool inicializarWinsock() {
    WSADATA wsa;
    return WSAStartup(MAKEWORD(2, 2), &wsa) == 0;
}
static void finalizarWinsock() {
    WSACleanup();
}
#else
static bool inicializarWinsock() { return true; }
static void finalizarWinsock() {}
#endif


// Helpers de parsing HTTP.

namespace {

// Lê a requisição HTTP do socket até encontrar "\r\n\r\n" (fim dos headers).
// Se houver corpo (Content-Length), lê também.
std::string lerRequisicao(SocketType sock) {
    std::string dados;
    char buffer[4096];
    size_t posFimHeaders = std::string::npos;

    // Lê os headers.
    while (true) {
        int n = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0) return "";
        buffer[n] = '\0';
        dados += buffer;

        posFimHeaders = dados.find("\r\n\r\n");
        if (posFimHeaders != std::string::npos) break;

        if (dados.size() > 1024 * 1024) return "";  // limite de 1 MB
    }

    // Verifica se há Content-Length.
    size_t posCL = dados.find("Content-Length:");
    if (posCL == std::string::npos) {
        posCL = dados.find("content-length:");
    }

    if (posCL != std::string::npos && posCL < posFimHeaders) {
        size_t posFimLinha = dados.find("\r\n", posCL);
        std::string linhaCL = dados.substr(posCL, posFimLinha - posCL);
        size_t posDoisPontos = linhaCL.find(':');
        if (posDoisPontos != std::string::npos) {
            std::string valor = linhaCL.substr(posDoisPontos + 1);
            // Remove espaços.
            size_t inicio = valor.find_first_not_of(" \t");
            if (inicio != std::string::npos) {
                int contentLength = std::stoi(valor.substr(inicio));
                size_t inicioCorpo = posFimHeaders + 4;
                size_t jaLido = dados.size() - inicioCorpo;
                while (static_cast<int>(jaLido) < contentLength) {
                    int n = recv(sock, buffer, sizeof(buffer) - 1, 0);
                    if (n <= 0) break;
                    buffer[n] = '\0';
                    dados += buffer;
                    jaLido += n;
                }
            }
        }
    }

    return dados;
}

// Extrai o método e o caminho da primeira linha da requisição.
// Ex.: "GET / HTTP/1.1" → método="GET", caminho="/"
void extrairMetodoCaminho(const std::string& req, std::string& metodo,
                          std::string& caminho) {
    size_t fimLinha = req.find("\r\n");
    if (fimLinha == std::string::npos) fimLinha = req.size();
    std::string primeira = req.substr(0, fimLinha);

    std::istringstream ss(primeira);
    ss >> metodo >> caminho;
}

// Extrai o corpo da requisição (após "\r\n\r\n").
std::string extrairCorpo(const std::string& req) {
    size_t pos = req.find("\r\n\r\n");
    if (pos == std::string::npos) return "";
    return req.substr(pos + 4);
}

// Envia uma resposta HTTP completa pelo socket.
void enviarResposta(SocketType sock, int status, const std::string& contentType,
                    const std::string& corpo) {
    std::ostringstream resp;
    resp << "HTTP/1.1 " << status << " OK\r\n"
         << "Content-Type: " << contentType << "\r\n"
         << "Content-Length: " << corpo.size() << "\r\n"
         << "Connection: close\r\n"
         << "Access-Control-Allow-Origin: *\r\n"
         << "\r\n"
         << corpo;

    std::string s = resp.str();
    send(sock, s.c_str(), static_cast<int>(s.size()), 0);
}

// Parse manual de JSON (simplificado).
// Espera-se: {"tipo":"AVANCAR","valor":0,"idTarefa":-1,
//             "campo":"","valorCampo":""}
std::string extrairStringJson(const std::string& body, const std::string& chave) {
    std::string busca = "\"" + chave + "\"";
    size_t pos = body.find(busca);
    if (pos == std::string::npos) return "";
    pos = body.find(':', pos);
    if (pos == std::string::npos) return "";
    pos = body.find('"', pos);
    if (pos == std::string::npos) return "";
    size_t fim = body.find('"', pos + 1);
    if (fim == std::string::npos) return "";
    return body.substr(pos + 1, fim - pos - 1);
}

int extrairIntJson(const std::string& body, const std::string& chave) {
    std::string busca = "\"" + chave + "\"";
    size_t pos = body.find(busca);
    if (pos == std::string::npos) return 0;
    pos = body.find(':', pos);
    if (pos == std::string::npos) return 0;
    pos++;
    while (pos < body.size() && std::isspace(static_cast<unsigned char>(body[pos]))) pos++;
    size_t fim = pos;
    while (fim < body.size() &&
           (std::isdigit(static_cast<unsigned char>(body[fim])) || body[fim] == '-')) {
        fim++;
    }
    if (fim == pos) return 0;
    try { return std::stoi(body.substr(pos, fim - pos)); }
    catch (...) { return 0; }
}

// Escapa aspas e barras em strings para JSON.
std::string escaparJson(const std::string& s) {
    std::string r;
    for (char c : s) {
        if (c == '"') r += "\\\"";
        else if (c == '\\') r += "\\\\";
        else if (c == '\n') r += "\\n";
        else r += c;
    }
    return r;
}

} 


// Implementação da classe.

struct ServidorHTTP::Impl {
    CallbackComando cbComando;
    CallbackEstado cbEstado;
    int porta = 8080;
    SocketType socketServidor = SOCKET_INVALIDO;
    std::thread threadServidor;
    std::atomic<bool> rodando{false};
};

ServidorHTTP::ServidorHTTP() : impl_(new Impl()) {}

ServidorHTTP::~ServidorHTTP() {
    parar();
    delete impl_;
}

void ServidorHTTP::setCBComando(CallbackComando cb) {
    impl_->cbComando = std::move(cb);
}

void ServidorHTTP::setCBEstado(CallbackEstado cb) {
    impl_->cbEstado = std::move(cb);
}

std::string ServidorHTTP::urlBase() const {
    return "http://localhost:" + std::to_string(impl_->porta);
}

bool ServidorHTTP::iniciar(int porta) {
    impl_->porta = porta;

    if (!inicializarWinsock()) {
        std::cerr << "Falha ao inicializar Winsock.\n";
        return false;
    }

    // Cria o socket do servidor.
    impl_->socketServidor = socket(AF_INET, SOCK_STREAM, 0);
    if (impl_->socketServidor == SOCKET_INVALIDO) {
        std::cerr << "Falha ao criar socket.\n";
        return false;
    }

    // Permite reutilizar a porta (evita "Address already in use").
    int opt = 1;
    setsockopt(impl_->socketServidor, SOL_SOCKET, SO_REUSEADDR,
               reinterpret_cast<const char*>(&opt), sizeof(opt));

    // Associa à porta.
    sockaddr_in endereco{};
    endereco.sin_family = AF_INET;
    endereco.sin_addr.s_addr = INADDR_ANY;  // aceita de qualquer interface
    endereco.sin_port = htons(static_cast<unsigned short>(porta));

    if (bind(impl_->socketServidor,
             reinterpret_cast<sockaddr*>(&endereco),
             sizeof(endereco)) != 0) {
        std::cerr << "Falha ao associar à porta " << porta << ".\n";
        FECHAR_SOCKET(impl_->socketServidor);
        impl_->socketServidor = SOCKET_INVALIDO;
        return false;
    }

    // Escuta conexões.
    if (listen(impl_->socketServidor, 5) != 0) {
        std::cerr << "Falha ao escutar.\n";
        FECHAR_SOCKET(impl_->socketServidor);
        impl_->socketServidor = SOCKET_INVALIDO;
        return false;
    }

    impl_->rodando = true;

    // Thread que aceita conexões.
    impl_->threadServidor = std::thread([this]() {
        while (impl_->rodando) {
            sockaddr_in enderecoCliente{};
            socklen_t tamCliente = sizeof(enderecoCliente);
            SocketType socketCliente = accept(
                impl_->socketServidor,
                reinterpret_cast<sockaddr*>(&enderecoCliente),
                &tamCliente);

            if (socketCliente == SOCKET_INVALIDO) {
                if (!impl_->rodando) break;
                continue;
            }

            // Lê a requisição.
            std::string req = lerRequisicao(socketCliente);
            if (req.empty()) {
                FECHAR_SOCKET(socketCliente);
                continue;
            }

            std::string metodo, caminho;
            extrairMetodoCaminho(req, metodo, caminho);

            // Rota GET / → HTML atual.
            if (metodo == "GET" && (caminho == "/" || caminho == "/index.html")) {
                if (!impl_->cbEstado) {
                    enviarResposta(socketCliente, 500, "text/plain",
                                   "Callback de estado nao configurado.");
                } else {
                    EstadoSimulacao estado = impl_->cbEstado();
                    GeradorHTML gerador;
                    std::string html = gerador.gerar(estado);
                    enviarResposta(socketCliente, 200,
                                   "text/html; charset=utf-8", html);
                }
                FECHAR_SOCKET(socketCliente);
                continue;
            }

            // Rota POST /comando → aplica comando.
            if (metodo == "POST" && caminho == "/comando") {
                if (!impl_->cbComando) {
                    enviarResposta(socketCliente, 500, "application/json",
                                   "{\"sucesso\":false,\"mensagem\":\"Callback nao configurado.\"}");
                    FECHAR_SOCKET(socketCliente);
                    continue;
                }

                std::string body = extrairCorpo(req);

                Comando cmd;
                std::string tipoStr = extrairStringJson(body, "tipo");
                if (tipoStr == "AVANCAR")                cmd.tipo = TipoComando::AVANCAR;
                else if (tipoStr == "RETROCEDER")        cmd.tipo = TipoComando::RETROCEDER;
                else if (tipoStr == "AVANCAR_N")         cmd.tipo = TipoComando::AVANCAR_N;
                else if (tipoStr == "RETROCEDER_N")      cmd.tipo = TipoComando::RETROCEDER_N;
                else if (tipoStr == "EXECUTAR_COMPLETO") cmd.tipo = TipoComando::EXECUTAR_COMPLETO;
                else if (tipoStr == "IR_TICK")      cmd.tipo = TipoComando::IR_TICK;
                else if (tipoStr == "MOD_TAREFA")  cmd.tipo = TipoComando::MOD_TAREFA;
                else if (tipoStr == "RESET")             cmd.tipo = TipoComando::RESET;
                else                                     cmd.tipo = TipoComando::DESCONHECIDO;

                cmd.valor = extrairIntJson(body, "valor");
                cmd.idTarefa = extrairIntJson(body, "idTarefa");
                cmd.campo = extrairStringJson(body, "campo");
                cmd.valorCampo = extrairStringJson(body, "valorCampo");

                ResultadoComando r = impl_->cbComando(cmd);

                std::ostringstream json;
                json << "{\"sucesso\":" << (r.sucesso ? "true" : "false")
                     << ",\"mensagem\":\"" << escaparJson(r.msg) << "\"}";
                enviarResposta(socketCliente, 200, "application/json", json.str());
                FECHAR_SOCKET(socketCliente);
                continue;
            }

            // Rota não encontrada.
            enviarResposta(socketCliente, 404, "text/plain", "404 Not Found");
            FECHAR_SOCKET(socketCliente);
        }
    });

    // Aguarda o servidor subir (timeout curto).
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    return true;
}

void ServidorHTTP::parar() {
    if (!impl_ || !impl_->rodando) return;

    impl_->rodando = false;

    // Fecha o socket do servidor para desbloquear o accept().
    if (impl_->socketServidor != SOCKET_INVALIDO) {
        FECHAR_SOCKET(impl_->socketServidor);
        impl_->socketServidor = SOCKET_INVALIDO;
    }

    if (impl_->threadServidor.joinable()) {
        impl_->threadServidor.join();
    }

    finalizarWinsock();
}