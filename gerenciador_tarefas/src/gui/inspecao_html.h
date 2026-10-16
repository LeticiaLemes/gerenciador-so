#pragma once
#include "core/estado_simulacao.h"
#include <string>

// Painel de inspeção de tarefas (requisito 1.5.1).
// Mostra o TCB atual de cada tarefa em formato de tabela.

class InspecaoHTML {
    public: 
        InspecaoHTML() = default;

        // Gera o HTML da tabela de inspeção
        std::string gerar(const EstadoSimulacao& estado) const;

    private:
        // Converte EstadoTarefa para string legível.
        static std::string estadoParaString(EstadoTarefa e);

        // Converte TipoEvento para string legível.
        static std::string tipoEventoParaString(TipoEvento t);
};