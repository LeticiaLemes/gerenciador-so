#include "gui/legenda_svg.h"
#include <sstream>

std::string LegendaSVG::gerar() const {
    std::ostringstream s;
    s << "<svg xmlns='http://www.w3.org/2000/svg' "
      << "width='" << largura_ << "' height='" << altura_ << "'>\n";

    s << "  <rect width='100%' height='100%' fill='#F0F0F0' stroke='#CCC'/>\n";

    // Título
    s << "  <text x='10' y='20' font-size='14' font-weight='bold' fill='#222'>"
      << "Legenda</text>\n";

    int y1 = 40;
    int y2 = 60;

    // Linha 1: estados
    int x = 10;

    // Executando
    s << "  <rect x='" << x << "' y='" << (y1 - 10)
      << "' width='20' height='12' fill='#3498DB' stroke='#222'/>\n";
    s << "  <text x='" << (x + 25) << "' y='" << y1
      << "' font-size='12' fill='#222'>Executando</text>\n";
    x += 110;

    // Pronta
    s << "  <rect x='" << x << "' y='" << (y1 - 10)
      << "' width='20' height='12' fill='none' stroke='#888'/>\n";
    s << "  <text x='" << (x + 25) << "' y='" << y1
      << "' font-size='12' fill='#222'>Pronta (não executando)</text>\n";
    x += 200;

    // Suspensa
    s << "  <defs><pattern id='legPontos' width='6' height='6' "
      << "patternUnits='userSpaceOnUse'>"
      << "<rect width='6' height='6' fill='#000'/>"
      << "<circle cx='3' cy='3' r='1' fill='#FFF'/>"
      << "</pattern></defs>\n";
    s << "  <rect x='" << x << "' y='" << (y1 - 10)
      << "' width='20' height='12' fill='url(#legPontos)' stroke='#000'/>\n";
    s << "  <text x='" << (x + 25) << "' y='" << y1
      << "' font-size='12' fill='#222'>Suspensa</text>\n";

    // Linha 2: eventos
    x = 10;

    // Chegada
    s << "  <polygon points='" << (x + 10) << "," << (y2 + 6) << " "
      << x << "," << (y2 - 4) << " " << (x + 20) << "," << (y2 - 4)
      << "' fill='#2E8B57'/>\n";
    s << "  <text x='" << (x + 25) << "' y='" << y2
      << "' font-size='12' fill='#222'>Chegada</text>\n";
    x += 100;

    // Término
    s << "  <polygon points='" << (x + 10) << "," << (y2 - 6) << " "
      << x << "," << (y2 + 4) << " " << (x + 20) << "," << (y2 + 4)
      << "' fill='#C0392B'/>\n";
    s << "  <text x='" << (x + 25) << "' y='" << y2
      << "' font-size='12' fill='#222'>Término</text>\n";
    x += 100;

    // Perda de deadline
    s << "  <line x1='" << x << "' y1='" << (y2 - 6)
      << "' x2='" << (x + 12) << "' y2='" << (y2 + 6)
      << "' stroke='#C0392B' stroke-width='2'/>\n";
    s << "  <line x1='" << x << "' y1='" << (y2 + 6)
      << "' x2='" << (x + 12) << "' y2='" << (y2 - 6)
      << "' stroke='#C0392B' stroke-width='2'/>\n";
    s << "  <text x='" << (x + 20) << "' y='" << y2
      << "' font-size='12' fill='#222'>Perda de deadline</text>\n";
    x += 170;

    // Sorteio
    s << "  <circle cx='" << (x + 8) << "' cy='" << y2
      << "' r='6' fill='#F1C40F' stroke='#333'/>\n";
    s << "  <text x='" << (x + 20) << "' y='" << y2
      << "' font-size='12' fill='#222'>Sorteio (desempate)</text>\n";

    s << "</svg>\n";
    return s.str();
}