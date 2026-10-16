#pragma once
#include "core/estado_simulacao.h"
#include <string>

// Geração do painel de status das CPUs em SVG.

// Mostra, para cada CPU:
//  - Quantos ticks ficou ligada executando tarefas.
//  - Quantos ticks ficou desligada (sem tarefas prontas).
// Requisito 1.2

class CpuStatusSVG {
public:
    CpuStatusSVG() = default;

    // Gera o SVG do painel de status.
    std::string gerar(const EstadoSimulacao& estado) const;

private:
    int larguraPorCpu_ = 150;
    int altura_ = 60;
};