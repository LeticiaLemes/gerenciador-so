#include "escalonadores/escalonador.h"
#include "escalonadores/rm.h"
#include "escalonadores/edf.h"
#include "escalonadores/desempate.h"
#include "utils/string_utils.h"
#include <algorithm>
#include <vector>

TCB* Escalonador::escolher(const FilaProntos& fila, int tick,
                           int* empatadas) const {
    if (empatadas) *empatadas = 1;

    std::vector<TCB*> candidatas = fila.copiar();
    if (candidatas.empty()) return nullptr;

    // Só concorrem ao desempate as tarefas com a melhor chave de prioridade.
    int melhorChave = chavePrioridade(*candidatas[0]);
    for (const TCB* t : candidatas) {
        melhorChave = std::min(melhorChave, chavePrioridade(*t));
    }
    std::vector<TCB*> grupo;
    for (TCB* t : candidatas) {
        if (chavePrioridade(*t) == melhorChave) grupo.push_back(t);
    }

    int n = 1;
    TCB* escolhida = Desempate::escolherMelhor(
        grupo, static_cast<unsigned>(tick), n);
    if (empatadas) *empatadas = n;
    return escolhida;
}

std::unique_ptr<Escalonador> criarEscalonador(const std::string& nome) {
    if (StringUtils::equalsIgnoreCase(nome, "RM")) {
        return std::make_unique<EscalonadorRM>();
    }
    if (StringUtils::equalsIgnoreCase(nome, "EDF")) {
        return std::make_unique<EscalonadorEDF>();
    }
    return nullptr;
}