// Converte hex para RGB

#pragma once
#include <cstdint>
#include <string>

namespace Cores {

    // Converte RRGGBB para RGB normal
    // Retorna false se a string não for válida
    bool hexParaRgb(const std::string& hex, uint8_t& r, uint8_t& g, uint8_t& b);

    // Verifica se a string é um hex de 6 dígitos válido
    bool hexValido(const std::string& hex);
}