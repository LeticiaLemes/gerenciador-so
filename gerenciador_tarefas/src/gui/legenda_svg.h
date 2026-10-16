#pragma once
#include <string>


// Legenda em SVG 
// requisito 2: legenda dos elementos gráficos.

class LegendaSVG {
public:
    LegendaSVG() = default;

    // Gera a legenda como string SVG.
    std::string gerar() const;

private:
    int largura_ = 1100;
    int altura_ = 80;
};