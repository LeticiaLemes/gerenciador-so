#include "string_utils.h"
#include <algorithm>
#include <cctype>
#include <sstream>

namespace StringUtils {

    std::string trim(const std::string& s) {
        size_t inicio = s.find_first_not_of(" \t\r\n");
        if (inicio == std::string::npos) return "";
        size_t fim = s.find_last_not_of(" \t\r\n");
        return s.substr(inicio, fim - inicio + 1);
    }

    std::string toUpper(const std::string& s) {
        std::string r = s;
        std::transform(r.begin(),r.end(), r.begin(), [](unsigned char c){ return std::toupper(c); });
        return r;
    }

    bool equalsIgnoreCase (const std::string& a, const std::string& b) {
        return toUpper(a) == toUpper(b);
    }

    std::vector<std::string> split(const std::string& s, char delim) {
        std::vector<std::string> partes;
        std::stringstream ss(s);
        std::string item;
        while (std::getline(ss, item, delim)) {
            partes.push_back(trim(item));
        }
        return partes;
    }

    bool isBlank(const std::string& s) {
        return trim(s).empty();
    }

    bool parseInt(const std::string& s, int& out)  {
        std::string t = trim(s);
        if (t.empty()) return false;
        try {
            size_t pos = 0;
            int valot = std::stol(t, &pos);
            if (pos != t.size()) return false;   // sobrou lixo após o número
            out = valot;
            return true;
        } catch (...) {
            return false;
        }
    }

}