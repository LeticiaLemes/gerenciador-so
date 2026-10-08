#pragma once
#include "core/tcb.h"
#include <string>
#include <vector>
#include <optional>

// Resultado de leitura de arquivo de configuração
struct ResultadoConfig {
    bool sucesso = false;
    ParamSistema sistema;
    std::vector<TCB> tarefas;
    std::vector<std::string> erros;   // mensagens de erro
    std::vector<std::string> avisos;  // mensagens de aviso
};

// Valores padrão
// Requisito 3.2
namespace Defaults {
    constexpr const char* ALGORITMO = "RM";
    constexpr int QUANTUM = 1;
    constexpr int QTD_CPUS = 1;
    constexpr const char* COR = "FFFFFF";
    constexpr int INGRESSO = 0;
    constexpr int DURACAO = 1;
    constexpr int PERIODO = 0;
    constexpr int PRAZO = 0;
}