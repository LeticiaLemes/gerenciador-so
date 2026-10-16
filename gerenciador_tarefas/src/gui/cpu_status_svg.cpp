#include "gui/cpu_status_svg.h"
#include <sstream>

std::string CpuStatusSVG::gerar(const EstadoSimulacao &estado) const
{
    int nCpus = estado.sistema.qtdeCpus;
    int larguraTotal = nCpus * larguraPorCpu_ + 20;

    std::ostringstream s;
    s << "<svg xmlns='http://www.w3.org/2000/svg' "
      << "width='" << larguraTotal << "' height='" << altura_ << "'>\n";

    s << "  <rect width='100%' height='100%' fill='#F8F8F8' stroke='#CCC'/>\n";
    s << "  <text x='10' y='18' font-size='13' font-weight='bold' fill='#222'>"
      << "Status das CPUs</text>\n";

    int totalTicks = estado.tickAtual;

    for (int cpu = 0; cpu < nCpus; ++cpu)
    {
        int x = 10 + cpu * larguraPorCpu_;
        int y = 28;

        int tempoDesligada = 0;
        if (cpu < static_cast<int>(estado.tempoCpuDesligada.size()))
            tempoDesligada = estado.tempoCpuDesligada[cpu];

        int tempoLigada = totalTicks - tempoDesligada;

        // Rótulo da CPU
        s << "  <text x='" << x << "' y='" << y
          << "' font-size='12' font-weight='bold' fill='#222'>CPU "
          << cpu << "</text>\n";

        // Barra proporcional
        int larguraBarra = larguraPorCpu_ - 30;
        int alturaBarra = 14;

        // Fundo (tempo ligada)
        s << "  <rect x='" << (x + 5) << "' y='" << (y + 5)
          << "' width='" << larguraBarra << "' height='" << alturaBarra
          << "' fill='#3498DB'/>\n";

        // Parte desligada (proporcional)
        int larguraDesligada = 0;
        if (totalTicks > 0)
            larguraDesligada = (tempoDesligada * larguraBarra) / totalTicks;

        if (larguraDesligada > 0)
        {
            s << "  <rect x='" << (x + 5 + larguraBarra - larguraDesligada)
              << "' y='" << (y + 5)
              << "' width='" << larguraDesligada << "' height='" << alturaBarra
              << "' fill='#E74C3C'/>\n";
        }

        // Borda
        s << "  <rect x='" << (x + 5) << "' y='" << (y + 5)
          << "' width='" << larguraBarra << "' height='" << alturaBarra
          << "' fill='none' stroke='#333'/>\n";

        // Texto: ligada / desligada
        s << "  <text x='" << (x + 5) << "' y='" << (y + alturaBarra + 18)
          << "' font-size='10' fill='#222'>Ligada: " << tempoLigada
          << " ticks</text>\n";
        s << "  <text x='" << (x + 5) << "' y='" << (y + alturaBarra + 30)
          << "' font-size='10' fill='#C0392B'>Desligada: " << tempoDesligada
          << " ticks</text>\n";
    }
    s << "</svg>\n";
    return s.str();
}