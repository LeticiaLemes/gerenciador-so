#pragma once
#include "config/config.h"
#include <string>

// Leitura do arquivo de configuração (requisito 3.3).
// Formato:
//   Linha 1: algoritmo;quantum;qtde_cpus
//   Linhas 2+: id;cor;ingresso;duracao;periodo;prazo;lista_eventos
//
// Regras implementadas:
//  - Qualquer caminho de arquivo (requisito 3.3.4)
//  - ';' final opcional (requisito 3.3.3)
//  - Case-insensitive (requisito 3.3.2)
//  - Linhas em branco ignoradas (requisito 3.3.6)
//  - Erros claros com número de linha (requisitos 3.3.6 e 5)
//  - Tarefas aperiódicas ignoradas e reportadas (requisito 4.4)

namespace Parser {

// Lê o arquivo completo. Retorna ResultadoConfig com sucesso, avisos e erros.
ResultadoConfig lerArquivo(const std::string& caminho);

// Exposta para testes: processa a primeira linha (sistema).
bool parseLinhaSistema(const std::string& linha, ParametrosSistema& saida, std::string& erro);

// Exposta para testes: processa uma linha de tarefa.
// 'aviso' é preenchido para casos não fatais (ex.: aperiódica).
bool parseLinhaTarefa(const std::string& linha, int numeroLinha, TCB& saida, std::string& erro, std::string& aviso);

}