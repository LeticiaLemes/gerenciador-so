#pragma once
#include "core/estado_simulacao.h"
#include <string>
#include <vector>

// Geração do gráfico de Gantt em SVG
// Regras (requisitos 2.1, 2.2 e 2.5):
//  - Eixo X: tempo (ticks).
//  - Eixo Y: tarefas em ordem decrescente de ID.
//  - Executando: retângulo com a cor da tarefa.
//  - Pronta: retângulo sem preenchimento (só borda).
//  - Suspensa: retângulo preto com padrão de pontos.
//  - Eventos: setas, X, círculos.
//  - CPU indicada em cada segmento.

class GanttSVG {
    public:

    GanttSVG();

    // Gera o SVG completo do Gantt e retorna como string
    // 'larguratotal' calculada a partir do maior tick e margem
    std::string gerar (const EstadoSimulacao& estado) const;

    // Configura a esscala
    void setEscala(int pixelsTick, int alturaLinha);

    private:

    int pixelsPorTick_ = 25;
    int alturaLinha_ = 35;
    int margemEsquerda_ = 70;
    int margemInferior_ = 40;
    int margemTopo_ = 20;
    int margemDireita_ = 40;

    // Calcula a largura total do SVG (base no maior tick)
    int calculaLargura(const EstadoSimulacao& estado) const;

    // Calcula a altura total do SVG (base no número de tarefas)
    int calculaAltura(const EstadoSimulacao& estado) const;

    // Ordena as tarefas por ID decrescente 
    // Requisito 2.5
    std::vector<TCB> tarefasOrdem(const EstadoSimulacao&estado) const;

    // Gera eixo X (tempo)com marcações
    std::string geraX(const EstadoSimulacao&estado,int altura) const;

    // Gera eixo Y (IDs) e as linhas separadoras
    std::string geraY(const EstadoSimulacao& estado, int largura) const;
    
    // Gera segmentos de cada tarefa
    std::string geraSegmentos(const EstadoSimulacao& estado) const;

    // Gera os eventos visuais
    std::string geraEventos(const EstadoSimulacao& estado) const;

    // Converte tick em coordenada X
    int tickParaX(int tick) const;

    // Converte índice em coordenada Y (0 = mais prox X)
    int indiceParaY(int indice, int alturaTotal) const;

    // Formata número como string
    static std::string num(int v);

    // Cor
    static std::string corHexDoTcb(const TCB& t);
};