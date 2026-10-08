#include "config/parser.h"
#include <cassert>
#include <iostream>

// Testes do Dia 1 da Pessoa 2.
// Cobrem: linha do sistema, linha de tarefa, case-insensitive, ';' opcional,
// período negativo, tarefa aperiódica e valores padrão.

static void testarLinhaSistemaValida() {
    ParametrosSistema p;
    std::string erro;
    bool ok = Parser::parseLinhaSistema("RM;2;4", p, erro);
    assert(ok);
    assert(p.algoritmo == "RM");
    assert(p.quantum == 2);
    assert(p.qtdeCpus == 4);
    std::cout << "[OK] linha do sistema válida\n";
}

static void testarCaseInsensitive() {
    ParametrosSistema p;
    std::string erro;
    bool ok = Parser::parseLinhaSistema("rm;1;1", p, erro);
    assert(ok);
    assert(p.algoritmo == "RM");  // normalizado para maiúsculas
    std::cout << "[OK] case-insensitive\n";
}

static void testarPontoEVirgulaOpcional() {
    ParametrosSistema p;
    std::string erro;
    bool ok = Parser::parseLinhaSistema("EDF;1;2;", p, erro);
    assert(ok);
    assert(p.algoritmo == "EDF");
    std::cout << "[OK] ';' final opcional\n";
}

static void testarLinhaSistemaIncompletaUsaPadroes() {
    ParametrosSistema p;
    std::string erro;
    bool ok = Parser::parseLinhaSistema("RM", p, erro);
    assert(ok);
    assert(p.quantum == Defaults::QUANTUM);
    assert(p.qtdeCpus == Defaults::QTDE_CPUS);
    std::cout << "[OK] linha do sistema incompleta usa padrões\n";
}

static void testarLinhaTarefaValida() {
    TCB t;
    std::string erro, aviso;
    bool ok = Parser::parseLinhaTarefa("1;FF0000;0;5;10;10", 2, t, erro, aviso);
    assert(ok);
    assert(t.id == 1);
    assert(t.corHex == "FF0000");
    assert(t.ingresso == 0);
    assert(t.duracao == 5);
    assert(t.periodo == 10);
    assert(t.prazo == 10);
    assert(t.estado != EstadoTarefa::IGNORADA);
    std::cout << "[OK] linha de tarefa válida\n";
}

static void testarPeriodoNegativo() {
    TCB t;
    std::string erro, aviso;
    bool ok = Parser::parseLinhaTarefa("1;FF0000;0;5;-3;10", 2, t, erro, aviso);
    assert(!ok);
    assert(erro.find("negativo") != std::string::npos);
    std::cout << "[OK] período negativo rejeitado\n";
}

static void testarAperiodicaIgnorada() {
    TCB t;
    std::string erro, aviso;
    bool ok = Parser::parseLinhaTarefa("1;FF0000;0;5;0;10", 2, t, erro, aviso);
    assert(ok);
    assert(t.estado == EstadoTarefa::IGNORADA);
    assert(!aviso.empty());
    std::cout << "[OK] tarefa aperiódica ignorada e reportada\n";
}

static void testarCorInvalida() {
    TCB t;
    std::string erro, aviso;
    bool ok = Parser::parseLinhaTarefa("1;XYZ;0;5;10;10", 2, t, erro, aviso);
    assert(!ok);
    assert(erro.find("cor") != std::string::npos);
    std::cout << "[OK] cor inválida rejeitada\n";
}

static void testarCamposInsuficientes() {
    TCB t;
    std::string erro, aviso;
    bool ok = Parser::parseLinhaTarefa("1;FF0000;0;5", 2, t, erro, aviso);
    assert(!ok);
    assert(erro.find("campos") != std::string::npos);
    std::cout << "[OK] campos insuficientes rejeitados\n";
}

int main() {
    testarLinhaSistemaValida();
    testarCaseInsensitive();
    testarPontoEVirgulaOpcional();
    testarLinhaSistemaIncompletaUsaPadroes();
    testarLinhaTarefaValida();
    testarPeriodoNegativo();
    testarAperiodicaIgnorada();
    testarCorInvalida();
    testarCamposInsuficientes();

    std::cout << "\nTodos os testes do Dia 1 passaram.\n";
    return 0;
}