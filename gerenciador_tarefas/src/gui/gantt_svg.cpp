#include "gui/gantt_svg.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

GanttSVG::GanttSVG() = default;

void GanttSVG::setEscala(int pixelsTick, int alturaLinha)
{
    pixelsPorTick_ = pixelsTick;
    alturaLinha_ = alturaLinha;
}

std::string GanttSVG::num(int v)
{
    return std::to_string(v);
}

// Converte cor para string hexadecimal
std::string GanttSVG::corHexDoTcb(const TCB& t) {
    std::ostringstream s;
    // Fluxo de string que monta no formato "AA000"
    s << "#" << std::hex << std::setw(2) << std::setfill('0')
      << static_cast<int>(t.r)
      << std::setw(2) << std::setfill('0') << static_cast<int>(t.g)
      << std::setw(2) << std::setfill('0') << static_cast<int>(t.b);

    // Junta tudo que foi montado e retorna a string final
    return s.str();
}

std::vector<TCB> GanttSVG::tarefasOrdem(const EstadoSimulacao &estado) const
{
    std::vector<TCB> ordenadas = estado.tarefas;
    std::sort(ordenadas.begin(), ordenadas.end(), [](const TCB &a, const TCB &b)
              {
                  return a.id > b.id; // decrescente -> req 2.5
              });
    return ordenadas;
}

int GanttSVG::calculaLargura(const EstadoSimulacao &estado) const
{
    int maiorTick = estado.tickAtual;
    for (const auto &seg : estado.segmentos)
    {
        if (seg.fim > maiorTick)
            maiorTick = seg.fim;
    }
    return margemEsquerda_ + (maiorTick + 5) * pixelsPorTick_ + margemDireita_;
}

int GanttSVG::calculaAltura(const EstadoSimulacao &estado) const
{
    int n = static_cast<int>(estado.tarefas.size());
    return margemTopo_ + n * alturaLinha_ + margemInferior_;
}

int GanttSVG::tickParaX(int tick) const
{
    return margemEsquerda_ + tick * pixelsPorTick_;
}

int GanttSVG::indiceParaY(int indice, int alturaTotal) const
{
    int baseY = alturaTotal - margemInferior_;
    return baseY - (indice + 1) * alturaLinha_;
}

// Geração de SVG completo

// descobre tamanho final, abre tag <svg> e injeta fundo 
// concatena retornos de funções que desenham
// fecha tag e retorna a string de texto
std::string GanttSVG::gerar(const EstadoSimulacao &estado) const
{
    int largura = calculaLargura(estado);
    int altura = calculaAltura(estado);

    std::ostringstream svg;
    svg << "<svg xmlns='http://www.w3.org/2000/svg' "
        << "width='" << largura << "' height='" << altura << "' "
        << "viewBox='0 0 " << largura << " " << altura << "'>\n";

    svg << "  <rect width='100%' height='100%' fill='#FAFAFA'/>\n";

    svg << geraX(estado, altura);
    svg << geraY(estado, largura);
    svg << geraSegmentos(estado);
    svg << geraEventos(estado);

    svg << "</svg>\n";
    return svg.str();
}

// Eixo X: linha do tempo com marcações

std::string GanttSVG::geraX(const EstadoSimulacao &estado, int altura) const
{
    std::ostringstream s;
    int yEixo = altura - margemInferior_;
    int larguraTotal = calculaLargura(estado);

    s << "  <line x1='" << margemEsquerda_ << "' y1='" << yEixo
      << "' x2='" << (calculaLargura(estado) - margemDireita_)
      << "' y2='" << yEixo << "' stroke='#333' stroke-width='1.5'/>\n";

    int maiorTick = estado.tickAtual;
    for (const auto &seg : estado.segmentos)
    {
        if (seg.fim > maiorTick)
            maiorTick = seg.fim;
    }

    // Marcações a cada 5 ticks
    // Se tempoforcurto,a cada 1
    int passo = (maiorTick <= 20) ? 1 : 5;
    for (int t = 0; t <= maiorTick + 5; t += passo)
    {
        int x = tickParaX(t);
        s << "  <line x1='" << x << "' y1='" << yEixo
          << "' x2='" << x << "' y2='" << (yEixo + 5)
          << "' stroke='#333' stroke-width='1'/>\n";
        s << "  <text x='" << x << "' y='" << (yEixo + 18)
          << "' font-size='11' text-anchor='middle' fill='#333'>"
          << t << "</text>\n";
    }

    s << "  <text x='" << (margemEsquerda_ + 5) << "' y='" << (yEixo + 32)
      << "' font-size='12' fill='#555'>Tempo (ticks)</text>\n";

    return s.str();
}

// Eixo Y: IDs das tarefas (decrescente)

std::string GanttSVG::geraY(const EstadoSimulacao &estado, int largura) const
{
    std::ostringstream s;
    int alturaTotal = calculaAltura(estado);
    auto ordenadas = tarefasOrdem(estado);

    for (size_t i = 0; i < ordenadas.size(); ++i)
    {
        int y = indiceParaY(static_cast<int>(i), alturaTotal);
        int yCentro = y + alturaLinha_ / 2;

        s << "  <line x1='" << margemEsquerda_ << "' y1='" << yCentro
          << "' x2='" << (largura - margemDireita_)
          << "' y2='" << yCentro << "' stroke='#DDD' stroke-width='1'/>\n";

        s << "  <text x='" << (margemEsquerda_ - 10) << "' y='" << (yCentro + 4)
          << "' font-size='13' text-anchor='end' fill='#222'>T"
          << ordenadas[i].id << "</text>\n";
    }
    s << "  <line x1='" << margemEsquerda_ << "' y1='" << margemTopo_
      << "' x2='" << margemEsquerda_ << "' y2='" << (alturaTotal - margemInferior_)
      << "' stroke='#333' stroke-width='1.5'/>\n";

    return s.str();
}

// Segmentos -> um retângulo por segmento

std::string GanttSVG::geraSegmentos(const EstadoSimulacao &estado) const
{
    std::ostringstream s;
    auto ordenadas = tarefasOrdem(estado);
    int alturaTotal = calculaAltura(estado);

    // localizador de posição para
    auto indice = [&](int id) -> int
    {
        for (size_t i = 0; i < ordenadas.size(); ++i)
        {
            if (ordenadas[i].id == id)
                return static_cast<int>(i); // retorna a posição atual, convertendo size_t em int
        }
        return -1;
    };

    // Definição do padrão de pontos para tarefas suspensas.
    // É declarado uma vez e reutilizado por todos os segmentos suspensos.
    s << "  <defs>\n"
      << "    <pattern id='pontos' width='6' height='6' "
      << "patternUnits='userSpaceOnUse'>\n"
      << "      <rect width='6' height='6' fill='#000'/>\n"
      << "      <circle cx='3' cy='3' r='1' fill='#FFF'/>\n"
      << "    </pattern>\n"
      << "  </defs>\n";

    for (const auto &seg : estado.segmentos)
    {
        int idx = indice(seg.idTarefa);
        if (idx < 0)
            continue;

        const TCB *t = nullptr;
        for (const auto &tcb : ordenadas)
        {
            if (tcb.id == seg.idTarefa)
            {
                t = &tcb;
                break;
            }
        }
        if (!t)
            continue;

        int x = tickParaX(seg.inicio);
        int largura = (seg.fim - seg.inicio) * pixelsPorTick_;
        int y = indiceParaY(idx, alturaTotal);
        int alturaBarra = alturaLinha_ - 4;

        switch (seg.estado)
        {
        case EstadoTarefa::EXECUTANDO:
        {
            s << "  <rect x='" << x << "' y='" << y
                  << "' width='" << largura << "' height='" << alturaBarra
                  << "' fill='" << corHexDoTcb(*t)
                  << "' stroke='#222' stroke-width='1'/>\n";
                // CPU no canto superior esquerdo do segmento.
                if (seg.cpu >= 0) {
                    s << "  <text x='" << (x + 3) << "' y='" << (y + 12)
                      << "' font-size='10' fill='#000'>CPU"
                      << seg.cpu << "</text>\n";
                }
                break;
        }
        case EstadoTarefa::PRONTA:
        {
            // Ausência de cor: só a borda
            s << "  <rect x='" << x << "' y='" << y
              << "' width='" << largura << "' height='" << alturaBarra
              << "' fill='none' stroke='#888' stroke-width='1'/>\n";
            break;
        }
        case EstadoTarefa::SUSPENSA:
        {
            // Preto com padrão de pontos (requisito 2.1)
            Etapas projeto simulador SO
Análise dos Erros de Compilação

Os erros se dividem em 3 categorias. Vou explicar cada uma e dar a correção.
Categoria 1: Erros no estado_simulacao.h (o mais crítico)
Erro
text

src/core/estado_simulacao.h:37:2: error: expected ';' after struct definition
   37 | }
      |  ^
      |  ;

E depois:
text

src/gui/gantt_svg.h:19:5: error: ISO C++ forbids declaration of 'GanttSVG' with no type
src/gui/gantt_svg.h:38:24: error: 'cont' has not been declared

Causa

O estado_simulacao.h não está fechando a struct EventoVisual com ;. O compilador viu } e esperava };. Isso quebrou todo o resto em cascata: como EstadoSimulacao não foi declarada corretamente, o GanttSVG em gantt_svg.h não conseguiu ser declarado.

Além disso, o arquivo tem erros de digitação que o compilador está reclamando:

    cont em vez de const

    tickAtual foi escrito como ticketAtual em algum lugar

    eventosVisuais foi escrito como eventosVisuals

    GanttSVGc em vez de GanttSVG

    ind em vez de int

    pixelsTick em vez de pixelsPorTick

Correção

Abra src/core/estado_simulacao.h e verifique se está exatamente assim:
cpp

#pragma once
#include "core/tcb.h"
#include <vector>
#include <string>

struct SegmentoGantt {
    int idTarefa = 0;
    int cpu = -1;
    int inicio = 0;
    int fim = 0;
    EstadoTarefa estado = EstadoTarefa::PRONTA;
};   // <-- TEM que ter ';'

struct EventoVisual {
    int tick = 0;
    int idTarefa = 0;
    enum class Tipo {
        CHEGADA,
        TERMINO,
        PERDEU_DEADLINE,
        SORTEIO
    } tipo = Tipo::CHEGADA;
};   // <-- TEM que ter ';'  (é aqui que está faltando)

struct EstadoSimulacao {
    int tickAtual = 0;
    ParametrosSistema sistema;
    std::vector<TCB> tarefas;
    std::vector<SegmentoGantt> segmentos;
    std::vector<EventoVisual> eventosVisuais;
    std::vector<int> tempoCpuDesligada;
    bool simulacaoIniciada = false;
    bool simulacaoTerminada = false;
};   // <-- TEM que ter ';'

Atenção especial: o erro aponta a linha 37. Abra o arquivo, vá até a linha 37 e confira se o }; está lá. O mais provável é que você tenha escrito } sem o ; no fim de EventoVisual.
Categoria 2: Erros em gantt_svg.h e gantt_svg.cpp
Erros
text

src/gui/gantt_svg.h:38:24: error: 'cont' has not been declared
   38 |     int calculaLargura(cont EstadoSimulacao& estado) const;

text

src/gui/gantt_svg.cpp:14:13: error: 'GanttSVG' has not been declared
src/gui/gantt_svg.cpp:37:12: error: 'margemEsquerda_' was not declared in this scope
src/gui/gantt_svg.cpp:74:12: error: 'geraX' was not declared in this scope

Causa

Vários erros de digitação nos seus arquivos:
Onde	Você escreveu	Deveria ser
gantt_svg.h:38	cont EstadoSimulacao&	const EstadoSimulacao&
gantt_svg.h:38	calculaLargura	calcularLargura
gantt_svg.h:19	(falta o tipo)	GanttSVG();
gantt_svg.cpp:6	GanttSVGc	GanttSVG
gantt_svg.cpp:19	tarefasOrdem	tarefasOrdenadas
gantt_svg.cpp:29	calculaLargura	calcularLargura
gantt_svg.cpp:40	calculaAltura	calcularAltura
gantt_svg.cpp:74	geraX	gerarEixoX
gantt_svg.cpp:75	geraY	gerarEixoY
gantt_svg.cpp:76	geraSegmentos	gerarSegmentos
gantt_svg.cpp:77	geraEventos	gerarEventos
gantt_svg.cpp:85	geraX	gerarEixoX
gantt_svg.cpp:121	geraY	gerarEixoY
gantt_svg.cpp:149	geraSegmentos	gerarSegmentos
gantt_svg.cpp:241	geraEventos	gerarEventos
gantt_svg.cpp:156	ind id	int id
gantt_svg.cpp:10	pixelsTick	pixelsPorTick_
gantt_svg.cpp	eventosVisuals	eventosVisuais
gantt_svg.cpp	ticketAtual	tickAtual
Correção

O nome das funções tem que bater exatamente entre o .h e o .cpp. O C++ é case-sensitive e não perdoa erros de digitação.

A forma mais segura é copiar a declaração do .h e colar no .cpp, trocando só o ; por { ... }. Por exemplo:

No gantt_svg.h:
cpp

std::string gerarEixoX(const EstadoSimulacao& estado, int altura) const;

No gantt_svg.cpp:
cpp

std::string GanttSVG::gerarEixoX(const EstadoSimulacao& estado, int altura) const {
    // ...
}

Não invente nomes diferentes. Se no .h está gerarEixoX, no .cpp tem que ser GanttSVG::gerarEixoX.
Categoria 3: Arquivo navegador.h não encontrado
Erro
text

src/gui/navegador.cpp:1:10: fatal error: gui/navegador.h: No such file or directory
    1 | #include "gui/navegador.h"
      |          ^~~~~~~~~~~~~~~~~
compilation terminated.

Causa

O compilador não encontrou src/gui/navegador.h. Isso acontece por um de dois motivos:

    O arquivo não existe. Você pode ter esquecido de criar src/gui/navegador.h.

    O arquivo existe, mas o #include está errado. Como você compilou com -I src, o include correto é #include "gui/navegador.h" — que é o que está lá. Então o problema é o item 1.

Correção

Verifique se o arquivo existe:
bash

ls -la src/gui/navegador.h

Se não existir, crie-o com o conteúdo:
cpp

#pragma once
#include <string>

namespace Navegador {

bool abrirArquivo(const std::string& caminho);
bool abrirUrl(const std::string& url);

} // namespace Navegador

E confirme que src/gui/navegador.cpp existe também. Se não, crie-o com o conteúdo que passei no Dia 2.
Checklist de Verificação Antes de Recompilar

Antes de tentar compilar de novo, faça estas verificações:
1. src/core/estado_simulacao.h

Abra o arquivo e confira:

    □

    struct SegmentoGantt { ... }; — termina com };
    □

    struct EventoVisual { ... }; — termina com };
    □

    struct EstadoSimulacao { ... }; — termina com };
    □

    Todos os campos estão escritos corretamente:

        tickAtual (não ticketAtual)

        eventosVisuais (não eventosVisuals)

        tempoCpuDesligada

        simulacaoIniciada

        simulacaoTerminada

2. src/gui/gantt_svg.h

    □

    A classe se chama GanttSVG (não GanttSVGc)
    □

    O construtor é GanttSVG();
    □

    Todos os const estão escritos corretamente (não cont)
    □

    Os nomes das funções são:

        calcularLargura (não calculaLargura)

        calcularAltura (não calculaAltura)

        tarefasOrdenadas (não tarefasOrdem)

        gerarEixoX (não geraX)

        gerarEixoY (não geraY)

        gerarSegmentos (não geraSegmentos)

        gerarEventos (não geraEventos)

3. src/gui/gantt_svg.cpp

    □

    Todos os métodos começam com GanttSVG::
    □

    Os nomes batem exatamente com o .h
    □

    Os membros usam os nomes do .h:

        pixelsPorTick_ (não pixelsTick)

        alturaLinha_

        margemEsquerda_, margemDireita_, margemTopo_, margemInferior_
    □

    A lambda usa int id (não ind id)
    □

    Os acessos ao estado usam:

        estado.tickAtual (não ticketAtual)

        estado.eventosVisuais (não eventosVisuals)

4. src/gui/navegador.h e src/gui/navegador.cpp

    □

    Ambos existem em src/gui/
    □

    O .cpp inclui #include "gui/navegador.h"

5. Estrutura de pastas

Confirme que a estrutura está assim:
bash

find src -name '*.h' -o -name '*.cpp' | sort

Deve listar:
text

src/core/estado_simulacao.h
src/core/tcb.h
src/gui/gantt_svg.cpp
src/gui/gantt_svg.h
src/gui/gerador_html.cpp
src/gui/gerador_html.h
src/gui/legenda_svg.cpp
src/gui/legenda_svg.h
src/gui/navegador.cpp
src/gui/navegador.h
src/utils/cores.cpp
src/utils/cores.h
src/utils/string_utils.cpp
src/utils/string_utils.h

Dica: Use o Pré-processador para Debugar

Se quiser ver o que o compilador está vendo, rode:
bash

g++ -std=c++17 -I src -E src/gui/gantt_svg.cpp | head -50

Isso mostra o arquivo após o pré-processamento (includes expandidos). Ajuda a identificar se o estado_simulacao.h está sendo incluído corretamente.
Dica: Compile Arquivo por Arquivo

Em vez de compilar tudo de uma vez, compile cada .cpp isoladamente para isolar erros:
bash

g++ -std=c++17 -Wall -Wextra -I src -c src/gui/gantt_svg.cpp -o /tmp/gantt_svg.o
g++ -std=c++17 -Wall -Wextra -I src -c src/gui/navegador.cpp -o /tmp/navegador.o

Isso mostra exatamente qual arquivo tem problema, sem a confusão de múltiplos erros em cascata.
Resumo das Ações

    Corrigir estado_simulacao.h: adicionar ; após struct EventoVisual { ... }.

    Corrigir gantt_svg.h: trocar cont por const, corrigir nomes das funções.

    Corrigir gantt_svg.cpp: corrigir todos os nomes para bater com o .h, corrigir ind → int, pixelsTick → pixelsPorTick_, eventosVisuals → eventosVisuais, ticketAtual → tickAtual.

    Criar src/gui/navegador.h se não existir.

    Recompilar e verificar se os erros diminuíram.

Se quiser, posso reescrever os arquivos completos e corretos de estado_simulacao.h, gantt_svg.h, gantt_svg.cpp e navegador.h/.cpp para você substituir diretamente. É só pedir.
como fica o dia 3
Dia 3 da Pessoa 2 — Escalonadores? Não. Eventos e Gantt Completo

Relembrando o cronograma:

    Dia 3 (Quarta) — Escalonadores

    Pessoa 1: RM, EDF, critérios de desempate.

    Pessoa 2:

        Finalizar o Gantt com todos os estados e cores.

        Implementar marcação de eventos no Gantt:

            Chegada de tarefa (ícone/seta).

            Término de tarefa (ícone/seta).

            Perda de deadline (marcação de erro).

            Sorteio (elemento gráfico no momento da (re)execução).

        Mostrar em qual CPU cada tarefa está executando.

    Juntos:

        Testar RM e EDF com um arquivo de configuração simples (2–3 tarefas, 1 CPU).

O que já foi feito no Dia 2

No Dia 2, a Pessoa 2 entregou:

    gantt_svg.h/.cpp — Gantt básico com segmentos e eventos (esqueleto).

    legenda_svg.h/.cpp — legenda.

    navegador.h/.cpp — abrir HTML no navegador.

    gerador_html.h/.cpp — HTML autocontido.

    tests/test_gui_dia2.cpp — teste isolado com dados fictícios.

Problema: a versão do Dia 2 tinha erros de compilação (nomes de funções inconsistentes, ; faltando, etc.). O Dia 3 começa corrigindo isso e finalizando o Gantt.
Objetivos do Dia 3

    Corrigir os erros de compilação do Dia 2 (se ainda houver).

    Finalizar o Gantt com todos os estados e cores:

        Executando: cor sólida da tarefa.

        Pronta: sem preenchimento (só borda).

        Suspensa: preto com padrão de pontos.

        Terminada: não desenha mais.

        Ignorada: não desenha.

    Implementar a marcação de eventos no Gantt:

        Chegada: seta verde para baixo.

        Término: seta vermelha para cima.

        Perda de deadline: X vermelho.

        Sorteio: círculo amarelo.

    Mostrar a CPU em cada segmento de execução.

    Mostrar o tempo de CPU desligada (requisito 1.2).

    Melhorar a legenda para incluir todos os elementos.

    Testar com dados fictícios que cubram todos os casos.

Estrutura de arquivos ao final do Dia 3
text

simulador-so/
├── src/
│   ├── core/
│   │   └── estado_simulacao.h        (corrigido)
│   ├── gui/
│   │   ├── gantt_svg.h / .cpp        (finalizado)
│   │   ├── legenda_svg.h / .cpp      (finalizado)
│   │   ├── navegador.h / .cpp        (corrigido)
│   │   ├── gerador_html.h / .cpp     (finalizado)
│   │   └── cpu_status_svg.h / .cpp   (NOVO: tempo de CPU desligada)
│   └── utils/
│       ├── string_utils.h / .cpp
│       └── cores.h / .cpp
└── tests/
    └── test_gui_dia3.cpp             (NOVO: cobre todos os casos)

1. Correção do src/core/estado_simulacao.h

Este arquivo é o contrato entre núcleo e GUI. Corrija os erros do Dia 2:
cpp

#pragma once
#include "core/tcb.h"
#include <vector>
#include <string>

// -----------------------------------------------------------------------------
// Interface entre o núcleo de simulação (Pessoa 1) e a GUI (Pessoa 2).
// O núcleo preenche; a GUI consome para produzir o SVG/HTML.
// -----------------------------------------------------------------------------

// Um segmento do Gantt: intervalo [inicio, fim) em que uma tarefa esteve
// em um determinado estado, possivelmente em uma CPU específica.
struct SegmentoGantt {
    int idTarefa = 0;
    int cpu = -1;               // -1 = nenhuma (pronta/suspensa)
    int inicio = 0;
    int fim = 0;
    EstadoTarefa estado = EstadoTarefa::PRONTA;
};   // <-- NÃO ESQUECER O ';'

// Um evento visual a ser marcado no Gantt.
struct EventoVisual {
    int tick = 0;
    int idTarefa = 0;
    enum class Tipo {
        CHEGADA,
        TERMINO,
        PERDEU_DEADLINE,
        SORTEIO
    } tipo = Tipo::CHEGADA;
};   // <-- NÃO ESQUECER O ';'

// Estado completo do sistema em um dado tick.
struct EstadoSimulacao {
    int tickAtual = 0;
    ParametrosSistema sistema;
    std::vector<TCB> tarefas;
    std::vector<SegmentoGantt> segmentos;
    std::vector<EventoVisual> eventosVisuais;
    std::vector<int> tempoCpuDesligada;   // índice = CPU, valor = ticks
    bool simulacaoIniciada = false;
    bool simulacaoTerminada = false;
};   // <-- NÃO ESQUECER O ';'

Checklist de nomes:

    tickAtual (não ticketAtual)

    eventosVisuais (não eventosVisuals)

    tempoCpuDesligada

    simulacaoIniciada

    simulacaoTerminada

2. src/gui/gantt_svg.h (finalizado)
cpp

#pragma once
#include "core/estado_simulacao.h"
#include <string>
#include <vector>

// -----------------------------------------------------------------------------
// Geração do gráfico de Gantt em SVG.
//
// Regras (requisitos 2.1, 2.2 e 2.5):
//  - Eixo X: tempo (ticks).
//  - Eixo Y: tarefas em ordem decrescente de ID.
//  - Executando: retângulo com a cor da tarefa.
//  - Pronta: retângulo sem preenchimento (só borda).
//  - Suspensa: retângulo preto com padrão de pontos.
//  - Eventos: setas, X, círculos.
//  - CPU indicada em cada segmento de execução.
// -----------------------------------------------------------------------------
class GanttSVG {
public:
    GanttSVG();

    // Gera o SVG completo do Gantt e retorna como string.
    std::string gerar(const EstadoSimulacao& estado) const;

    // Configura a escala.
    void setEscala(int pixelsPorTick, int alturaLinha);

private:
    int pixelsPorTick_ = 25;
    int alturaLinha_ = 35;
    int margemEsquerda_ = 70;
    int margemInferior_ = 40;
    int margemTopo_ = 20;
    int margemDireita_ = 40;

    // --- Cálculos de dimensão ---
    int calcularLargura(const EstadoSimulacao& estado) const;
    int calcularAltura(const EstadoSimulacao& estado) const;

    // --- Ordenação ---
    std::vector<TCB> tarefasOrdenadas(const EstadoSimulacao& estado) const;

    // --- Geração de partes do SVG ---
    std::string gerarEixoX(const EstadoSimulacao& estado, int altura) const;
    std::string gerarEixoY(const EstadoSimulacao& estado, int largura) const;
    std::string gerarSegmentos(const EstadoSimulacao& estado) const;
    std::string gerarEventos(const EstadoSimulacao& estado) const;

    // --- Conversões de coordenadas ---
    int tickParaX(int tick) const;
    int indiceParaY(int indice, int alturaTotal) const;

    // --- Helpers ---
    static std::string num(int v);
    static std::string corHexDoTcb(const TCB& t);
};

Pontos importantes:

    Todos os nomes batem com o .cpp.

    const está escrito corretamente.

    Nenhuma função se chama calculaLargura ou geraX.

3. src/gui/gantt_svg.cpp (finalizado)
cpp

#include "gui/gantt_svg.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

// -----------------------------------------------------------------------------
// Implementação da geração de SVG.
// -----------------------------------------------------------------------------

GanttSVG::GanttSVG() = default;

void GanttSVG::setEscala(int pixelsPorTick, int alturaLinha) {
    pixelsPorTick_ = pixelsPorTick;
    alturaLinha_ = alturaLinha;
}

std::string GanttSVG::num(int v) {
    return std::to_string(v);
}

std::string GanttSVG::corHexDoTcb(const TCB& t) {
    std::ostringstream s;
    s << "#" << std::hex << std::setw(2) << std::setfill('0')
      << static_cast<int>(t.r)
      << std::setw(2) << std::setfill('0') << static_cast<int>(t.g)
      << std::setw(2) << std::setfill('0') << static_cast<int>(t.b);
    return s.str();
}

std::vector<TCB> GanttSVG::tarefasOrdenadas(const EstadoSimulacao& estado) const {
    std::vector<TCB> ordenadas = estado.tarefas;
    std::sort(ordenadas.begin(), ordenadas.end(),
              [](const TCB& a, const TCB& b) {
                  return a.id > b.id;  // decrescente (requisito 2.5)
              });
    return ordenadas;
}

int GanttSVG::calcularLargura(const EstadoSimulacao& estado) const {
    int maiorTick = estado.tickAtual;
    for (const auto& seg : estado.segmentos) {
        if (seg.fim > maiorTick) maiorTick = seg.fim;
    }
    return margemEsquerda_ + (maiorTick + 5) * pixelsPorTick_ + margemDireita_;
}

int GanttSVG::calcularAltura(const EstadoSimulacao& estado) const {
    int n = static_cast<int>(estado.tarefas.size());
    return margemTopo_ + n * alturaLinha_ + margemInferior_;
}

int GanttSVG::tickParaX(int tick) const {
    return margemEsquerda_ + tick * pixelsPorTick_;
}

int GanttSVG::indiceParaY(int indice, int alturaTotal) const {
    int baseY = alturaTotal - margemInferior_;
    return baseY - (indice + 1) * alturaLinha_;
}

// -----------------------------------------------------------------------------
// Geração do SVG completo.
// -----------------------------------------------------------------------------
std::string GanttSVG::gerar(const EstadoSimulacao& estado) const {
    int largura = calcularLargura(estado);
    int altura = calcularAltura(estado);

    std::ostringstream svg;
    svg << "<svg xmlns='http://www.w3.org/2000/svg' "
        << "width='" << largura << "' height='" << altura << "' "
        << "viewBox='0 0 " << largura << " " << altura << "'>\n";

    svg << "  <rect width='100%' height='100%' fill='#FAFAFA'/>\n";

    svg << gerarEixoX(estado, altura);
    svg << gerarEixoY(estado, largura);
    svg << gerarSegmentos(estado);
    svg << gerarEventos(estado);

    svg << "</svg>\n";
    return svg.str();
}

// -----------------------------------------------------------------------------
// Eixo X: linha do tempo com marcações.
// -----------------------------------------------------------------------------
std::string GanttSVG::gerarEixoX(const EstadoSimulacao& estado, int altura) const {
    std::ostringstream s;
    int yEixo = altura - margemInferior_;
    int larguraTotal = calcularLargura(estado);

    s << "  <line x1='" << margemEsquerda_ << "' y1='" << yEixo
      << "' x2='" << (larguraTotal - margemDireita_)
      << "' y2='" << yEixo << "' stroke='#333' stroke-width='1.5'/>\n";

    int maiorTick = estado.tickAtual;
    for (const auto& seg : estado.segmentos) {
        if (seg.fim > maiorTick) maiorTick = seg.fim;
    }

    // Marcações a cada 5 ticks; se o tempo for curto, a cada 1.
    int passo = (maiorTick <= 20) ? 1 : 5;
    for (int t = 0; t <= maiorTick + 5; t += passo) {
        int x = tickParaX(t);
        s << "  <line x1='" << x << "' y1='" << yEixo
          << "' x2='" << x << "' y2='" << (yEixo + 5)
          << "' stroke='#333' stroke-width='1'/>\n";
        s << "  <text x='" << x << "' y='" << (yEixo + 18)
          << "' font-size='11' text-anchor='middle' fill='#333'>"
          << t << "</text>\n";
    }

    s << "  <text x='" << (margemEsquerda_ + 5) << "' y='" << (yEixo + 32)
      << "' font-size='12' fill='#555'>Tempo (ticks)</text>\n";

    return s.str();
}

// -----------------------------------------------------------------------------
// Eixo Y: IDs das tarefas (ordem decrescente).
// -----------------------------------------------------------------------------
std::string GanttSVG::gerarEixoY(const EstadoSimulacao& estado, int largura) const {
    std::ostringstream s;
    int alturaTotal = calcularAltura(estado);
    auto ordenadas = tarefasOrdenadas(estado);

    for (size_t i = 0; i < ordenadas.size(); ++i) {
        int y = indiceParaY(static_cast<int>(i), alturaTotal);
        int yCentro = y + alturaLinha_ / 2;

        s << "  <line x1='" << margemEsquerda_ << "' y1='" << yCentro
          << "' x2='" << (largura - margemDireita_)
          << "' y2='" << yCentro << "' stroke='#DDD' stroke-width='1'/>\n";

        s << "  <text x='" << (margemEsquerda_ - 10) << "' y='" << (yCentro + 4)
          << "' font-size='13' text-anchor='end' fill='#222'>T"
          << ordenadas[i].id << "</text>\n";
    }

    s << "  <line x1='" << margemEsquerda_ << "' y1='" << margemTopo_
      << "' x2='" << margemEsquerda_ << "' y2='" << (alturaTotal - margemInferior_)
      << "' stroke='#333' stroke-width='1.5'/>\n";

    return s.str();
}

// -----------------------------------------------------------------------------
// Segmentos: um retângulo por segmento, com estilo conforme o estado.
// -----------------------------------------------------------------------------
std::string GanttSVG::gerarSegmentos(const EstadoSimulacao& estado) const {
    std::ostringstream s;
    auto ordenadas = tarefasOrdenadas(estado);
    int alturaTotal = calcularAltura(estado);

    auto indiceDe = [&](int id) -> int {
        for (size_t i = 0; i < ordenadas.size(); ++i) {
            if (ordenadas[i].id == id) return static_cast<int>(i);
        }
        return -1;
    };

    // Definição do padrão de pontos para tarefas suspensas.
    // É declarado uma vez e reutilizado por todos os segmentos suspensos.
    s << "  <defs>\n"
      << "    <pattern id='pontos' width='6' height='6' "
      << "patternUnits='userSpaceOnUse'>\n"
      << "      <rect width='6' height='6' fill='#000'/>\n"
      << "      <circle cx='3' cy='3' r='1' fill='#FFF'/>\n"
      << "    </pattern>\n"
      << "  </defs>\n";

    for (const auto& seg : estado.segmentos) {
        int idx = indiceDe(seg.idTarefa);
        if (idx < 0) continue;

        const TCB* t = nullptr;
        for (const auto& tcb : ordenadas) {
            if (tcb.id == seg.idTarefa) { t = &tcb; break; }
        }
        if (!t) continue;

        int x = tickParaX(seg.inicio);
        int largura = (seg.fim - seg.inicio) * pixelsPorTick_;
        int y = indiceParaY(idx, alturaTotal);
        int alturaBarra = alturaLinha_ - 4;

        switch (seg.estado) {
            case EstadoTarefa::EXECUTANDO: {
                s << "  <rect x='" << x << "' y='" << y
                  << "' width='" << largura << "' height='" << alturaBarra
                  << "' fill='" << corHexDoTcb(*t)
                  << "' stroke='#222' stroke-width='1'/>\n";
                // CPU no canto superior esquerdo do segmento.
                if (seg.cpu >= 0) {
                    s << "  <text x='" << (x + 3) << "' y='" << (y + 12)
                      << "' font-size='10' fill='#000'>CPU"
                      << seg.cpu << "</text>\n";
                }
                break;
            }
            case EstadoTarefa::PRONTA: {
                // Sem cor: só borda.
                s << "  <rect x='" << x << "' y='" << y
                  << "' width='" << largura << "' height='" << alturaBarra
                  << "' fill='none' stroke='#888' stroke-width='1'/>\n";
                break;
            }
            case EstadoTarefa::SUSPENSA: {
                // Preto com padrão de pontos.
                s << "  <rect x='" << x << "' y='" << y
                  << "' width='" << largura << "' height='" << alturaBarra
                  << "' fill='url(#pontos)' stroke='#000' stroke-width='1'/>\n";
                break;
        }
         case EstadoTarefa::TERMINADA:
        case EstadoTarefa::IGNORADA:
            // Não desenha.
            break;
        }
    }

    return s.str();
}

// Eventos -> setas, X e círculos

std::string GanttSVG::geraEventos(const EstadoSimulacao& estado) const {
    std::ostringstream s;
    auto ordenadas = tarefasOrdem(estado);
    int alturaTotal = calculaAltura(estado);

    auto indice = [&](int id) -> int {
        for (size_t i = 0; i < ordenadas.size(); ++i) {
            if (ordenadas[i].id == id)
                return static_cast<int>(i);
        }
        return -1;
    };

        for (const auto& ev : estado.eventosVisuais) {
        int idx = indice(ev.idTarefa);
        if (idx < 0) continue;

        int x = tickParaX(ev.tick);
        int y = indiceParaY(idx, alturaTotal);
        int yCentro = y + alturaLinha_ / 2;

        switch (ev.tipo) {
            case EventoVisual::Tipo::CHEGADA: {
                // Seta verde para baixo
                s << "  <polygon points='" << x << "," << (yCentro + 8) << " "
                  << (x - 6) << "," << (yCentro - 2) << " "
                  << (x + 6) << "," << (yCentro - 2)
                  << "' fill='#2E8B57'/>\n";
                break;
            }
            case EventoVisual::Tipo::TERMINO: {
                // Seta vermelha para cima (término)
                s << "  <polygon points='" << x << "," << (yCentro - 8) << " "
                  << (x - 6) << "," << (yCentro + 2) << " "
                  << (x + 6) << "," << (yCentro + 2)
                  << "' fill='#C0392B'/>\n";
                break;
            }
            case EventoVisual::Tipo::PERDEU_DEADLINE: {
                // X vermelho (perda de deadline)
                s << "  <line x1='" << (x - 5) << "' y1='" << (yCentro - 5)
                  << "' x2='" << (x + 5) << "' y2='" << (yCentro + 5)
                  << "' stroke='#C0392B' stroke-width='2'/>\n";
                s << "  <line x1='" << (x - 5) << "' y1='" << (yCentro + 5)
                  << "' x2='" << (x + 5) << "' y2='" << (yCentro - 5)
                  << "' stroke='#C0392B' stroke-width='2'/>\n";
                break;
            }
            case EventoVisual::Tipo::SORTEIO: {
                // Círculo amarelo (sorteio de desempate)
                s << "  <circle cx='" << x << "' cy='" << yCentro
                  << "' r='5' fill='#F1C40F' stroke='#333' stroke-width='1'/>\n";
                break;
            }
        }
    }

    return s.str();
}