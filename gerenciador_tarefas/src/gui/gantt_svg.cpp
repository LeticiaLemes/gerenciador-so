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
            s << "  <rect x='" << x << "' y='" << y
              << "' width='" << largura << "' height='" << alturaBarra
              << "' fill='url(#pontos)' stroke='#000' stroke-width='1'/>\n";
            break;
        }
        case EstadoTarefa::TERMINADA:
        case EstadoTarefa::IGNORADA:
        case EstadoTarefa::INATIVA:
            // Sem ativação em andamento: não desenha.
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