#include "escalonadores/desempate.h"
#include <cstddef>

namespace Desempate {

    bool preferir(const TCB& a, const TCB& b) {
        // Critério 1: tarefa que já está numa CPU.
        bool aExecutando = (a.cpuAtual >= 0);
        bool bExecutando = (b.cpuAtual >= 0);
        if (aExecutando != bExecutando) return aExecutando;

        // Critério 2: prazo (deadlineAtual menor vence).
        if (a.deadlineAtual != b.deadlineAtual) {
            return a.deadlineAtual < b.deadlineAtual;
        }

        // Critério 3: ingresso (quem chegou antes vence).
        if (a.ingresso != b.ingresso) {
            return a.ingresso < b.ingresso;
        }

        // Critério 4: duração (menor vence).
        if (a.duracao != b.duracao) {
            return a.duracao < b.duracao;
        }

        // Empate nos critérios 1-4.
        return false;
    }

    TCB* escolherMelhor(const std::vector<TCB*>& candidatas,
                        unsigned semente,
                        int& empatadas) {
        empatadas = 0;
        if (candidatas.empty()) return nullptr;

        // Mantém só as candidatas que ninguém supera (conjunto dos melhores).
        std::vector<TCB*> melhores;
        for (TCB* c : candidatas) {
            bool superada = false;
            for (TCB* o : candidatas) {
                if (o != c && preferir(*o, *c)) { superada = true; break; }
            }
            if (!superada) melhores.push_back(c);
        }

        empatadas = static_cast<int>(melhores.size());
        if (melhores.size() == 1) return melhores[0];

        // Critério 5: sorteio determinístico entre as empatadas.
        unsigned h = semente * 2654435761u;
        std::size_t idx = (h >> 16) % melhores.size();
        return melhores[idx];
    }

} // namespace Desempate