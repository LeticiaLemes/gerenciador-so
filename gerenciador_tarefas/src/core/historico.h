#pragma once
#include "core/tcb.h"
#include "core/cpu.h"
#include "estado_simulacao.h"
#include <vector>
#include <deque>


// Snapshot do estado completo do sistema em um tick.

// Armazena valores (cópias), não ponteiros:
//  - tarefas: cópia dos TCBs
//  - cpus: CPUSnapshot (guarda o id da tarefa em vez do ponteiro)
//  - eventosVisuais: cópia dos eventos
//  - segmentos: cópia dos segmentos

// A fila de prontos NÃO é armazenada: é reconstruída a partir dos estados
// dos TCBs ao restaurar.

struct CPUSnapshot {
    int id = 0;
    int idTarefaAtual = -1;   // -1 = ociosa
    bool ligada = false;
    int ticksLigada = 0;
    int ticksDesligada = 0;
    int ticksNoQuantum = 0;   // ticks contínuos da tarefa atual (preempção)
};

struct Snapshot {
    int tick = 0;
    ParametrosSistema sistema;
    std::vector<TCB> tarefas;
    std::vector<CPUSnapshot> cpus;          // ATENÇÃO: tarefaAtual é ponteiro; ver abaixo
    std::vector<EventoVisual> eventosVisuais;
    std::vector<SegmentoGantt> segmentos;
    bool simulacaoTerminada = false;
};

// Histórico de snapshots.
//
// Funciona como uma pilha dupla:
//  - snapshots passados (já executados)
//  - o snapshot atual é o último da pilha
//
// Retroceder = desempilhar o atual e restaurar o anterior.
// Avançar = executar novamente o tick (recalcular).

class Historico {
public:
    // Limite de snapshots para evitar consumo excessivo de memória.
    // Se ultrapassar, o snapshot mais antigo é descartado.
    explicit Historico(std::size_t limite = 10000);

    // Salva um snapshot. Se ultrapassar o limite, descarta o mais antigo.
    void salvar(const Snapshot& s);

    // Retorna o snapshot mais recente (sem remover).
    // Retorna nullptr se vazio.
    const Snapshot* atual() const;

    // Remove e retorna o snapshot mais recente.
    // Retorna nullptr se vazio.
    Snapshot desempilhar();

    // Retorna true se há pelo menos um snapshot.
    bool vazio() const;

    // Retorna o número de snapshots.
    std::size_t tamanho() const;

    // Limpa todo o histórico.
    void limpar();

    // Retorna o tick do snapshot mais antigo (útil para debug).
    int tickMaisAntigo() const;

private:
    std::deque<Snapshot> pilha_;
    std::size_t limite_;
};
