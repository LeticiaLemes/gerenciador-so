#pragma once
#include "core/tcb.h"
#include <vector>

// Critérios de desempate para escolha de tarefa (requisito 4.3).
//
// Ordem exigida:
//  1. Tarefa que está executando imediatamente antes (evita troca desnecessária).
//  2. Prazo da tarefa (menor prazo vence).
//  3. Instante de ingresso (quem chegou antes vence).
//  4. Duração da tarefa (menor duração vence).
//  5. Sorteio (quando todos os critérios anteriores empatarem).
//
// "Estava executando antes" = a tarefa ainda ocupa uma CPU (cpuAtual >= 0)
// no momento da decisão.

namespace Desempate {

    // Compara duas tarefas pelos critérios 1-4.
    // Retorna true se 'a' é ESTRITAMENTE preferível a 'b'.
    bool preferir(const TCB& a, const TCB& b);

    // Escolhe a melhor tarefa entre as candidatas.
    // 'empatadas' recebe quantas candidatas ficaram empatadas nos critérios 1-4
    // (1 = sem empate; > 1 = houve sorteio entre elas).
    // O sorteio é determinístico (depende só de 'semente'), para que avançar,
    // retroceder e avançar de novo reproduza exatamente a mesma simulação.
    TCB* escolherMelhor(const std::vector<TCB*>& candidatas,
                        unsigned semente,
                        int& empatadas);

} // namespace Desempate