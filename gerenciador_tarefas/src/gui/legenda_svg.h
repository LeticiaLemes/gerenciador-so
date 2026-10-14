#pragma once
#include <string>


// Legenda em SVG (requisito 2: legenda dos elementos gráficos).

class LegendaSVG {
public:
    LegendaSVG() = default;

    // Gera a legenda como string SVG.
    std::string gerar() const;

private:
    int largura_ = 1000;
    int altura_ = 70;
};