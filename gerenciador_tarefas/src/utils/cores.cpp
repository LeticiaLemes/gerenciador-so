#include "cores.h"
#include "string_utils.h"
#include <cctype>

namespace Cores {

    bool hexValido(const std::string& hex) {
        if (hex.size() != 6) return false;
        for (char c: hex) {
            if (!std::isxdigit(static_cast<unsigned char>(c)))
                return false;
        }
        return true;
    }

    bool hexParaRgb(const std::string& hex, uint8_t& r, uint8_t& g, uint8_t& b) {
        if (!hexValido(hex))
            return false;
        
        r = static_cast<uint8_t>(std::stoi(hex.substr(0, 2), nullptr, 16));
        g = static_cast<uint8_t>(std::stoi(hex.substr(2, 2), nullptr, 16));
        b = static_cast<uint8_t>(std::stoi(hex.substr(4, 2), nullptr, 16));
        return true;
    }

}