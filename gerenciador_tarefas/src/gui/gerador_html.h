#pragma once
#include "core/estado_simulacao.h"
#include "gui/gantt_svg.h"
#include "gui/legenda_svg.h"
#include "gui/cpu_status_svg.h"
#include "gui/controles_html.h"
#include "gui/inspecao_html.h"
#include <string>

// -----------------------------------------------------------------------------
// Gera um arquivo HTML autocontido com o Gantt em SVG e a legenda.
//
// O HTML inclui:
//  - O SVG do Gantt.
//  - O SVG da legenda.
//  - CSS para layout.
//  - avaScript para controles de simulação.
//
// O arquivo é salvo em disco e aberto no navegador padrão.
// -----------------------------------------------------------------------------
class GeradorHTML {
public:
    GeradorHTML() = default;

    // Gera o HTML completo a partir do estado e retorna como string.
    std::string gerar(const EstadoSimulacao& estado) const;

    // Gera o HTML e salva no caminho indicado.
    bool salvar(const EstadoSimulacao& estado, const std::string& caminho) const;

    // Gera, salva e abre no navegador padrão.
    bool gerarEAbrir(const EstadoSimulacao& estado, const std::string& caminho) const;

private:
    GanttSVG gantt_;
    LegendaSVG legenda_;
    CpuStatusSVG cpuStatus_;
    CpuStatusSVG cpuStatus_;
    ControlesHTML controles_;
    InspecaoHTML inspecao_;

    // Cabeçalho HTML (DOCTYPE, head, CSS).
    std::string >gerarCabecalho(const EstadoSimulacao& estado) const;

    // Rodapé HTML.
    std::string gerarRodape() const;
};