#pragma once
#include "core/tcb.h"
#include <vector>
#include <string>

// Interface entre o núcleo e a GUI
// O núcleo preenche e a GUI consome para produzir SVG

struct SegmentoGantt {
    int idTarefa = 0;
    int cpu = -1;   // -1 = nenhuma (pronta/suspensa)
    int inicio = 0;
    int fim = 0;
    EstadoTarefa estado= EstadoTarefa::PRONTA;
};

struct EventoVisual{
    int tick = 0;
    int idTarefa = 0;
    enum class Tipo{
        CHEGADA,
        TERMINO,
        PERDEU_DEADLINE,
        SORTEIO
    } tipo = Tipo::CHEGADA;
};

// Estado completo do sistema em um dado tick
struct EstadoSimulacao {
    int tickAtual = 0;
    ParametrosSistema sistema;
    std::vector<TCB> tarefas;
    std::vector<SegmentoGantt> segmentos;
    std::vector<EventoVisual> eventosVisuais;
    std::vector<int> tempoCpuDesligada; // Índice = CPU, valor = ticks
    bool simulacaoIniciada = false;
    bool simulacaoTerminada = false;
};