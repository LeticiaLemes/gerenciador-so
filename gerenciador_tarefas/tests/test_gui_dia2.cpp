#include "core/estado_simulacao.h"
#include "gui/gerador_html.h"
#include <iostream>

// -----------------------------------------------------------------------------
// Teste isolado da GUI no Dia 2.
// Preenche um EstadoSimulacao fictício, gera o HTML e abre no navegador.
// Não depende do núcleo da Pessoa 1.
// -----------------------------------------------------------------------------

static EstadoSimulacao criarEstadoFicticio() {
    EstadoSimulacao estado;

    estado.sistema.algoritmo = "RM";
    estado.sistema.quantum = 2;
    estado.sistema.qtdeCpus = 2;
    estado.tickAtual = 30;

    // Tarefas fictícias
    TCB t1;
    t1.id = 1; t1.corHex = "FF0000"; t1.r = 255; t1.g = 0; t1.b = 0;
    t1.ingresso = 0; t1.duracao = 5; t1.periodo = 10; t1.prazo = 10;

    TCB t2;
    t2.id = 2; t2.corHex = "00FF00"; t2.r = 0; t2.g = 255; t2.b = 0;
    t2.ingresso = 0; t2.duracao = 3; t2.periodo = 15; t2.prazo = 15;

    TCB t3;
    t3.id = 3; t3.corHex = "0000FF"; t3.r = 0; t3.g = 0; t3.b = 255;
    t3.ingresso = 5; t3.duracao = 2; t3.periodo = 20; t3.prazo = 20;

    estado.tarefas = { t1, t2, t3 };

    // Segmentos fictícios (id, cpu, inicio, fim, estado)
    estado.segmentos = {
        { 1, 0, 0, 5, EstadoTarefa::EXECUTANDO },
        { 2, 1, 0, 3, EstadoTarefa::EXECUTANDO },
        { 1, 0, 5, 10, EstadoTarefa::EXECUTANDO },
        { 3, 1, 5, 7, EstadoTarefa::EXECUTANDO },
        { 1, -1, 10, 12, EstadoTarefa::SUSPENSA },
        { 2, 0, 12, 15, EstadoTarefa::EXECUTANDO },
        { 1, 1, 15, 20, EstadoTarefa::EXECUTANDO },
        { 3, 0, 20, 22, EstadoTarefa::EXECUTANDO },
        { 2, -1, 22, 25, EstadoTarefa::PRONTA },
        { 1, 0, 25, 30, EstadoTarefa::EXECUTANDO },
    };

    // Eventos fictícios
    estado.eventosVisuais = {
        { 0, 1, EventoVisual::Tipo::CHEGADA },
        { 0, 2, EventoVisual::Tipo::CHEGADA },
        { 5, 3, EventoVisual::Tipo::CHEGADA },
        { 10, 1, EventoVisual::Tipo::PERDEU_DEADLINE },
        { 15, 2, EventoVisual::Tipo::SORTEIO },
        { 20, 3, EventoVisual::Tipo::TERMINO },
    };

    estado.tempoCpuDesligada = { 0, 0 };
    estado.simulacaoIniciada = true;
    estado.simulacaoTerminada = false;

    return estado;
}

int main() {
    EstadoSimulacao estado = criarEstadoFicticio();

    GeradorHTML gerador;
    std::string caminho = "bin/gantt_teste.html";

    if (!gerador.gerarEAbrir(estado, caminho)) {
        std::cerr << "Falha ao gerar ou abrir o HTML.\n";
        return 1;
    }

    std::cout << "HTML gerado em: " << caminho << "\n";
    std::cout << "O navegador deve abrir automaticamente.\n";
    return 0;
}