#include "config/defaults.h"

namespace Defaults {

// Preenche campos opcionais da TCB com valores padrão.
// Não valida; apenas completa o que estiver faltando.
bool aplicarPadroesTarefa(TCB& t) {
    bool aplicou = false;

    if (t.corHex.empty()) {
        t.corHex = COR;
        aplicou = true;
    }
    // ingresso e periodo já são validados no parser; não sobrescrevemos.

    return aplicou;
}

// Preenche campos opcionais dos parâmetros do sistema.
bool aplicarPadroesSistema(ParametrosSistema& s) {
    bool aplicou = false;

    if (s.algoritmo.empty()) {
        s.algoritmo = ALGORITMO;
        aplicou = true;
    }
    if (s.quantum <= 0) {
        s.quantum = QUANTUM;
        aplicou = true;
    }
    if (s.qtdeCpus < 1) {
        s.qtdeCpus = QTDE_CPUS;
        aplicou = true;
    }

    return aplicou;
}

// Fim dos valores padrão do sistema.

}