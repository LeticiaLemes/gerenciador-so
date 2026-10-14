#pragma once
#include <string>

// Abre arquivos e URLs no programa padrão do sistema operacional.

namespace Navegador {

    // Abre um arquivo local com o aplicativo padrão.
    bool abrirArquivo(const std::string& caminho);

    // Abre uma URL no navegador padrão.
    bool abrirUrl(const std::string& url);
}
