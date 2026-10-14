#include "gui/navegador.h"
#include <cstdlib>

namespace Navegador {

bool abrirArquivo(const std::string& caminho) {
#if defined(_WIN32) || defined(_WIN64)
    // Windows: 'start' abre com o programa padrão
    std::string cmd = "start \"\" \"" + caminho + "\"";
#elif defined(__APPLE__)
    // macOS: 'open' abre com o app padrão
    std::string cmd = "open \"" + caminho + "\"";
#else
    // Linux/Unix: 'xdg-open' abre com o app padrão
    std::string cmd = "xdg-open \"" + caminho + "\" > /dev/null 2>&1 &";
#endif
    int ret = std::system(cmd.c_str());
    return ret == 0;
}

bool abrirUrl(const std::string& url) {
#if defined(_WIN32) || defined(_WIN64)
    std::string cmd = "start \"\" \"" + url + "\"";
#elif defined(__APPLE__)
    std::string cmd = "open \"" + url + "\"";
#else
    std::string cmd = "xdg-open \"" + url + "\" > /dev/null 2>&1 &";
#endif
    int ret = std::system(cmd.c_str());
    return ret == 0;
}

} 