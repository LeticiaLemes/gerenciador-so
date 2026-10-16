#pragma once
#include "core/estado_simulacao.h"
#include "core/comando.h"
#include "gui/gerador_html.h"
#include <functional>
#include <string>

// Servidor HTTP embutido (usando cpp-httplib, header-only).
//
// Rotas:
//  - GET  /          -> retorna o HTML atual do Gantt + controles.
//  - POST /comando   -> recebe um comando JSON, aplica e retorna resultado.
//
// O servidor NÃO contém lógica de simulação. Ele apenas recebe comandos
// e delega para um callback fornecido pelo núcleo 

class ServidorHTTP {
    public:
        // Callback que o núcleo fornece para aplicar comandos
        // Recebe o comando, aplica e retorna o resultado
        using CallbackComando = std::function<ResultadoComando(const Comando&)>;

        // Callback que o núcleo fornece para obter o estado atual
        using CallbackEstado = std::function<EstadoSimulacao()>;

        ServidorHTTP();
        ~ServidorHTTP();

        // Define os callbacks do núcleo
        void setCBComando(CallbackComando cb);
        void setCBEstadi(CallbackEstado cb);

        // Inicia o servidor na porta indicada, e retorna false se falhar
        bool iniciar(int porta);

        // Para o servidor
        void parar();

        // Retorna a URL base (ex.: "http://localhost:8000")
        std::string urlBase() const;

    private:
        struct Impl;
        Impl* impl_;
};