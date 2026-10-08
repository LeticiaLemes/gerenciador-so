#pragma once
#include <string>
#include <vector>

namespace Stringutils {
    // Remove espaços/tabs do início e fim
    std::string trim(const std::string& str);

    // Converte para maiúsculas (comparações case-sensistive)
    std::string toUpper(const std::string& s);

    // Compara duas strings ignorando maiúsculas/minúsculas
    bool equalsIgnoreCase(const std::string& a, const std::string& b);

    // Divide uma string for um delimitador específico
    // Remove espaços de cada parte e ignora partes vazias no final
    std::vector<std::string> split(const std::string& s, char delim);

    // Verifica de a linha é vazia ou se só tem espaços
    bool isBlank(const std::string& s);

    // Converte string para inteiro, e retorna false se inválido
    bool parseInt(const std::string& s, int& out);
}