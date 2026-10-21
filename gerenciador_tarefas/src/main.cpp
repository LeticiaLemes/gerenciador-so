// Ponto de entrada do simulador com servidor HTTP interativo.
// Uso:  bin/gerenciador <arquivo_de_configuracao> [porta] [--abrir]

#include "config/parser.h"
#include "core/simulador.h"
#include "core/comando.h"
#include "gui/servidor_http.h"
#include "gui/navegador.h"
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    std::string caminhoConfig;
    int porta = 8080;
    bool abrir = false;

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--abrir") {
            abrir = true;
        } else if (caminhoConfig.empty()) {
            caminhoConfig = a;
        } else {
            try { porta = std::stoi(a); } catch (...) {}
        }
    }

    if (caminhoConfig.empty()) {
        std::cerr << "Uso: " << argv[0]
                  << " <arquivo_de_configuracao> [porta] [--abrir]\n";
        return 2;
    }

    // 1. Lê a configuração.
    ResultadoConfig cfg = Parser::lerArquivo(caminhoConfig);
    for (const auto& aviso : cfg.avisos) std::cerr << "aviso: " << aviso << "\n";
    for (const auto& erro : cfg.erros)   std::cerr << "erro: " << erro << "\n";
    if (!cfg.sucesso) return 1;

    // 2. Cria e inicializa o núcleo.
    Simulador sim;
    sim.inicializar(cfg.sistema, cfg.tarefas);

    // 3. Configura o servidor HTTP.
    ServidorHTTP servidor;
    servidor.setCBEstado([&]() { return sim.obterEstado(); });
    servidor.setCBComando([&](const Comando& cmd) -> ResultadoComando {
        ResultadoComando r;
        switch (cmd.tipo) {
            case TipoComando::AVANCAR:
                sim.avancarNTicks(1);
                r.sucesso = true;
                r.msg = "Avançou 1 tick.";
                break;
            case TipoComando::RETROCEDER:
                if (sim.retrocederUmTick()) {
                    r.sucesso = true;
                    r.msg = "Retrocedeu 1 tick.";
                } else {
                    r.sucesso = false;
                    r.msg = "Não há histórico para retroceder.";
                }
                break;
            case TipoComando::AVANCAR_N:
                sim.avancarNTicks(cmd.valor);
                r.sucesso = true;
                r.msg = "Avançou " + std::to_string(cmd.valor) + " ticks.";
                break;
            case TipoComando::RETROCEDER_N:
                if (sim.retrocederNTicks(cmd.valor)) {
                    r.sucesso = true;
                    r.msg = "Retrocedeu " + std::to_string(cmd.valor) + " ticks.";
                } else {
                    r.sucesso = false;
                    r.msg = "Não foi possível retroceder " +
                                 std::to_string(cmd.valor) + " ticks.";
                }
                break;
            case TipoComando::EXECUTAR_COMPLETO:
                sim.executarCompleto();
                r.sucesso = true;
                r.msg = "Simulação concluída.";
                break;
            case TipoComando::IR_TICK:
                if (sim.irParaTick(cmd.valor)) {
                    r.sucesso = true;
                    r.msg = "Foi para o tick " + std::to_string(cmd.valor) + ".";
                } else {
                    r.sucesso = false;
                    r.msg = "Não foi possível ir para o tick " +
                                 std::to_string(cmd.valor) + ".";
                }
                break;
            case TipoComando::MOD_TAREFA: {
                std::string erro;
                bool ok = sim.modificarTarefa(cmd.idTarefa, cmd.campo,
                                              cmd.valorCampo, erro);
                r.sucesso = ok;
                r.msg = ok ? "Tarefa modificada." : erro;
                break;
            }
            case TipoComando::RESET:
                sim.inicializar(cfg.sistema, cfg.tarefas);
                r.sucesso = true;
                r.msg = "Simulação reiniciada.";
                break;
            default:
                r.sucesso = false;
                r.msg = "Comando desconhecido.";
                break;
        }
        return r;
    });

    // 4. Inicia o servidor.
    if (!servidor.iniciar(porta)) {
        std::cerr << "erro: não foi possível iniciar o servidor na porta "
                  << porta << ".\n";
        return 1;
    }

    std::cout << "Servidor rodando em " << servidor.urlBase() << "\n";
    std::cout << "Abra essa URL no navegador para interagir.\n";
    std::cout << "Pressione Enter para encerrar.\n";

    if (abrir) {
        Navegador::abrirUrl(servidor.urlBase());
    }

    std::cin.get();
    servidor.parar();
    return 0;
}