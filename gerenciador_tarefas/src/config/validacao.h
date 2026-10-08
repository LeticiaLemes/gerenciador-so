#pragma once
#include "core/tcb.h"
#include "config/config.h"
#include <string>

// Validação semântica de parâmetros já parseados
// Separada parse para ser testável isoladamente e reutilizável para mod parâm.
// Requisito 3.4

namespace Validacao {

    // Valida TCB. False e preenche erro se inválida
    // Preence=he aviso para casos não fatais
    bool validarTarefa(const TCB& t, std::string&erro, std::string& aviso);

    // Valida os parâmetros sistema. False e preenche erro
    bool validarSistema(const ParametrosSistema& s, std::string& erro, std::string& aviso);
}