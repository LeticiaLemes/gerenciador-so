#include "gui/gerador_html.h"
#include "gui/navegador.h"
#include <fstream>
#include <sstream>

std::string GeradorHTML::gerarCabecalho(const EstadoSimulacao& estado) const {
    std::ostringstream s;
    s << "<!DOCTYPE html>\n"
      << "<html lang='pt-BR'>\n"
      << "<head>\n"
      << "  <meta charset='UTF-8'>\n"
      << "  <title>Simulador SO - Gantt</title>\n"
      << "  <style>\n"
      << "    body { font-family: Arial, sans-serif; margin: 20px; "
      << "background: #FFF; color: #222; }\n"
      << "    h1 { font-size: 20px; margin-bottom: 5px; }\n"
      << "    .info { font-size: 14px; color: #555; margin-bottom: 15px; }\n"
      << "    .gantt { border: 1px solid #CCC; overflow: auto; "
      << "background: #FAFAFA; padding: 10px; }\n"
      << "    .legenda { margin-top: 15px; }\n"
      << "    .controles { margin-top: 15px; }\n"
      << "    button { padding: 8px 14px; margin-right: 8px; "
      << "font-size: 14px; cursor: pointer; }\n"
      << "  </style>\n"
      << "</head>\n"
      << "<body>\n"
      << "  <h1>Simulador de Sistema Operacional Multitarefa</h1>\n"
      << "  <div class='info'>\n"
      << "    Algoritmo: <b>" << estado.sistema.algoritmo << "</b> | "
      << "Quantum: <b>" << estado.sistema.quantum << "</b> | "
      << "CPUs: <b>" << estado.sistema.qtdeCpus << "</b> | "
      << "Tick atual: <b>" << estado.tickAtual << "</b>\n"
      << "  </div>\n";
    return s.str();
}

std::string GeradorHTML::gerarRodape() const {
    std::ostringstream s;
    s << "  <div class='controles'>\n"
      << "    <button onclick='window.print()'>Imprimir / Salvar como PDF</button>\n"
      << "  </div>\n"
      << "</body>\n"
      << "</html>\n";
    return s.str();
}

std::string GeradorHTML::gerar(const EstadoSimulacao& estado) const {
    std::ostringstream s;
    s << gerarCabecalho(estado);

    // Painel de status das CPUs
    s << "  <div class='cpu-status'>\n";
    s << cpuStatus_.gerar(estado);
    s << "  </div>\n";

    // Gantt
    s << "  <div class='gantt'>\n";
    s << gantt_.gerar(estado);
    s << "  </div>\n";

    // Legenda
    s << "  <div class='legenda'CpuStatusSVG cpuStatus_;>\n";
    s << legenda_.gerar();
    s << "  </div>\n";

    s << gerarRodape();
    return s.str();
}

bool GeradorHTML::salvar(const EstadoSimulacao& estado, const std::string& caminho) const {
    std::ofstream arquivo(caminho);
    if (!arquivo.is_open()) return false;
    arquivo << gerar(estado);
    return arquivo.good();
}

bool GeradorHTML::gerarEAbrir(const EstadoSimulacao& estado, const std::string& caminho) const {
    if (!salvar(estado, caminho)) return false;
    return Navegador::abrirArquivo(caminho);
}