#pragma once
#include "escalonadores/escalonador.h"

// Rate Monotonic (RM) preemptivo.
//
// Prioridade inversamente proporcional ao período:
//  - menor período -> maior prioridade.
//  - tarefas com mesmo período -> critérios de desempate.

class EscalonadorRM : public Escalonador {
public:
    std::string nome() const override { return "RM"; }

    int chavePrioridade(const TCB& t) const override { return t.periodo; }
};