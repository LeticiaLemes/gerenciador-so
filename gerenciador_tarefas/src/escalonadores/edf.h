#pragma once
#include "escalonadores/escalonador.h"

// Earliest Deadline First (EDF) preemptivo.
//
// Prioridade pelo menor deadline absoluto (deadlineAtual):
//  - menor deadlineAtual -> maior prioridade.
//  - tarefas com mesmo deadline -> critérios de desempate.

class EscalonadorEDF : public Escalonador {
public:
    std::string nome() const override { return "EDF"; }

    int chavePrioridade(const TCB& t) const override { return t.deadlineAtual; }
};