#pragma once
#include "core/tcb.h"
#include <vector>
#include <cstddef>

// Fila de prontos GLOBAL do sistema (requisito 1.2).
//
// Armazena ponteiros para TCBs que estão no estado PRONTA e aguardam CPU.
// NÃO decide escalonamento: apenas armazena e fornece acesso.
// O escalonador (Dia 3) é quem escolhe qual tarefa sai da fila.
//
// A fila é global: não é individual por CPU. Todas as CPUs consomem da mesma
// fila, garantindo que não haja tarefas prontas com CPUs ociosas.

class FilaProntos {
public:
    FilaProntos() = default;

    // Insere uma tarefa no fim da fila.
    // Ignora se o ponteiro for nulo.
    void inserir(TCB* tarefa);

    // Remove e retorna a primeira tarefa da fila.
    // Retorna nullptr se a fila estiver vazia.
    TCB* removerPrimeira();

    // Remove uma tarefa específica (pelo id).
    // Retorna true se removeu, false se não encontrou.
    bool removerPorId(int id);

    // Retorna a primeira tarefa sem remover.
    // Retorna nullptr se vazia.
    TCB* espiarPrimeira() const;

    // Retorna true se a fila está vazia.
    bool vazia() const;

    // Retorna o número de tarefas na fila.
    std::size_t tamanho() const;

    // Remove todas as tarefas da fila.
    void limpar();

    // Retorna uma cópia dos ponteiros (para o histórico ou depuração).
    std::vector<TCB*> copiar() const;

    // Verifica se uma tarefa com o id está na fila.
    bool contem(int id) const;

private:
    std::vector<TCB*> fila_;   // ponteiros para TCBs no estado PRONTA
};