#pragma once
#include "core/tcb.h"
#include "core/fila_prontos.h"
#include <string>
#include <memory>

// Interface abstrata para algoritmos de escalonamento (requisito 4.2).
//
// Um algoritmo concreto (RM, EDF, ...) só precisa dizer qual é a chave de
// prioridade de uma tarefa. A escolha em si (incluindo os critérios de
// desempate do requisito 4.3) é feita aqui, igual para todos.
//
// O escalonador NÃO remove a tarefa da fila nem decide preempção; apenas
// escolhe. O Simulador é quem remove, atribui à CPU e preempta.

class Escalonador {
public:
    virtual ~Escalonador() = default;

    // Nome do algoritmo (ex.: "RM", "EDF").
    virtual std::string nome() const = 0;

    // Chave de prioridade: MENOR valor = MAIOR prioridade.
    virtual int chavePrioridade(const TCB& t) const = 0;

    // Escolhe, dentre as tarefas de 'fila', a de maior prioridade aplicando
    // os critérios de desempate. Retorna nullptr se a fila estiver vazia.
    //  - tick: usado como semente do sorteio (critério 5).
    //  - empatadas (opcional): quantas tarefas empataram em tudo (>1 = sorteio).
    TCB* escolher(const FilaProntos& fila, int tick,
                  int* empatadas = nullptr) const;
};

// Fábrica de escalonadores: cria o escalonador a partir do nome.
// Retorna nullptr se o nome for desconhecido.
std::unique_ptr<Escalonador> criarEscalonador(const std::string& nome);