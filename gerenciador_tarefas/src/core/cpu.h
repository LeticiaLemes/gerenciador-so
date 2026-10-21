#pragma once
#include "core/tcb.h"


// Representa um processador do sistema simulado.

// Responsabilidade única: saber qual tarefa está executando (se houver) e
// registrar quanto tempo ficou ligada/desligada.

// NÃO decide escalonamento, NÃO conhece a fila global e NÃO verifica eventos.

struct CPU {
    int id = 0;                     // identificador da CPU (0..N-1)
    TCB* tarefaAtual = nullptr;     // tarefa executando, ou nullptr se ociosa
    bool ligada = false;            // true se está executando alguma tarefa

    // Contadores para o requisito 1.2 (tempo de CPU desligada)
    int ticksLigada = 0;
    int ticksDesligada = 0;

    // Ticks contínuos da tarefa atual. Quando chega ao quantum, a CPU é um
    // ponto de preempção (o escalonador pode trocar a tarefa).
    int ticksNoQuantum = 0;

    // Retorna true se a CPU está ociosa (sem tarefa).

    bool ociosa() const { 
        return tarefaAtual == nullptr; 
    }


    // Atribui uma tarefa à CPU.

    void atribuir(TCB* t) {
        tarefaAtual = t;
        ligada = (t != nullptr);
        if (t != nullptr) {
            t->cpuAtual = id;
            t->estado = EstadoTarefa::EXECUTANDO;
        }
    }


    // Libera a CPU (tarefa terminou, foi suspensa ou foi removida).

    void liberar() {
        if (tarefaAtual != nullptr) {
            tarefaAtual->cpuAtual = -1;
        }
        tarefaAtual = nullptr;
        ligada = false;
    }


    // Avança um tick de execução.
    // Retorna true se a tarefa atual concluiu sua ativação neste tick.

    bool executarUmTick() {
        if (tarefaAtual == nullptr) {
            ++ticksDesligada;
            return false;
        }
        ++ticksLigada;
        ++ticksNoQuantum;
        ++tarefaAtual->tempoExecutado;
        return tarefaAtual->tempoExecutado >= tarefaAtual->duracao;
    }
};
