#pragma once
#include "core/tcb.h"
#include "config/config.h"

// Aplica valores pad5rão para estruturas incompletas do parser
// Requisito 3.2

namespace Defaults {

    // Aplica padrões a uma TCB. True se algo foi aplicado
    // Usado qnd arquivo omite campos opcionais
    bool aplicarPadroesTarefa(TCB& t);

    // Aplica padrões aos parâmetros do sistema
    bool aplicarPadroesSistema(ParametrosSistema& s);
}