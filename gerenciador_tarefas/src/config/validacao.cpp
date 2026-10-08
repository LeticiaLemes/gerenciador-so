#include "config/validacao.h"
#include "utils/cores.h"
#include "utils/string_utils.h"

namespace Validacao {
    using namespace StringUtils;

    // Valida uma terafa já parseada
    // REGRAS:
    //  -  id >= 0
    //  -  cor hex válida (6 dígitos hexadecimais)
    //  -  ingresso >= 0
    //  -  duração > 0
    //  -  prazo >= 0
    //  -  período >= 0 (0 = aperiódica -> aviso, não erro)
    bool validarTarefa(const TCB& t, std::string& erro, std::string& aviso) {
        if (t.id < 0) {
            erro = "id deve ser >= 0.";
            return false;
        }

        if (!Cores::hexValido(t.corHex)) {
            erro = "cor inválida (" + t.corHex + "). Deve ser no formato #RRGGBB.";
            return false;
        }

        if (t.ingresso < 0) {
            erro = "ingresso deve ser >= 0.";
            return false;
        }

        if (t.duracao <= 0) {
            erro = "duração deve ser > 0.";
            return false;
        }

        if (t.prazo < 0) {
            erro = "prazo deve ser >= 0.";
            return false;
        }

        if (t.periodo < 0) {
            // requisito 3.3
            erro = "período deve ser >= 0.";
            return false;
        }

        if (t.periodo == 0) {
            // Requisito 4.4
            aviso = "tarefa id=" + std::to_string(t.id) +
                " é aperiódica (período=0) e será IGNORADA no Projeto A.";
            // Não é erro; o chamador decide marcar estado IGNORADA.
        }

        return true;
    }


    // Valida parâmetros do sistema
    // REGRAS:
    //  -  algoritmo deve ser "RM" ou "EDF"
    //  -  quantum > 0
    //  -  qtdeCpus >= 0
    bool validarSistema(const ParametrosSistema& s, std::string& erro) {
            if (!equalsIgnoreCase(s.algoritmo, "RM") &&
        !equalsIgnoreCase(s.algoritmo, "EDF")) {
        erro = "algoritmo desconhecido ('" + s.algoritmo +
               "'). Use 'RM' ou 'EDF'.";
        return false;
    }

    if (s.quantum <= 0) {
        erro = "quantum deve ser > 0.";
        return false;
    }

    if (s.qtdeCpus < 1) {
        erro = "qtde_cpus deve ser >= 1.";
        return false;
    }

    return true;
    }
}