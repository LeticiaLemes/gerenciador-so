#include "gui/controles_html.h"
#include <sstream>

std::string ControlesHTML::gerar(const EstadoSimulacao& estado) const {
    std::ostringstream s;
    s << "<div class='controles'>\n";
    s << gerarBotoes();
    s << gerarIrTick(estado);
    s << gerarFormMod(estado);
    s << "</div>\n";
    s << gerarJavaScript();
    return s.str();
}

std::string ControlesHTML::gerarBotoes() const {
    std::ostringstream s;
    s << "  <div class='linha-botoes'>\n"
      << "    <button onclick=\"enviarComando({tipo:'RETROCEDER'})\">"
      << "◀ Retroceder</button>\n"
      << "    <button onclick=\"enviarComando({tipo:'AVANCAR'})\">"
      << "Avançar ▶</button>\n"
      << "    <button onclick=\"enviarComando({tipo:'AVANCAR_N', valor:10})\">"
      << "Avançar 10 ▶▶</button>\n"
      << "    <button onclick=\"enviarComando({tipo:'RETROCEDER_N', valor:10})\">"
      << "◀◀ Retroceder 10</button>\n"
      << "    <button onclick=\"enviarComando({tipo:'EXECUTAR_COMPLETO'})\">"
      << "⏩ Executar completo</button>\n"
      << "    <button onclick=\"enviarComando({tipo:'RESET'})\">"
      << "⟲ Reset</button>\n"
      << "  </div>\n";
    return s.str();
}

std::string ControlesHTML::gerarIrTick(const EstadoSimulacao& estado) const {
    std::ostringstream s;
    s << "  <div class='linha-ir-para'>\n"
      << "    <label>Ir para tick: </label>\n"
      << "    <input type='number' id='tickDestino' min='0' max='"
      << estado.tickAtual << "' value='" << estado.tickAtual << "'>\n"
      << "    <button onclick=\"irTick()\">Ir</button>\n"
      << "  </div>\n";
    return s.str();
}

std::string ControlesHTML::gerarFormMod(
        const EstadoSimulacao& estado) const {
    std::ostringstream s;
    s << "  <div class='linha-modificar'>\n"
      << "    <h3>Modificar tarefa</h3>\n"
      << "    <label>Tarefa: </label>\n"
      << "    <select id='tarefaSel'>\n";
    for (const auto& t : estado.tarefas) {
        s << "      <option value='" << t.id << "'>T" << t.id << "</option>\n";
    }
    s << "    </select>\n"
      << "    <label>Campo: </label>\n"
      << "    <select id='campoSel'>\n"
      << "      <option value='duracao'>duração</option>\n"
      << "      <option value='periodo'>período</option>\n"
      << "      <option value='prazo'>prazo</option>\n"
      << "      <option value='ingresso'>ingresso</option>\n"
      << "      <option value='cor'>cor</option>\n"
      << "      <option value='estado'>estado</option>\n"
      << "    </select>\n"
      << "    <label>Novo valor: </label>\n"
      << "    <input type='text' id='valorCampo' placeholder='ex.: 10'>\n"
      << "    <button onclick=\"modificarTarefa()\">Aplicar</button>\n"
      << "    <div id='msgModificacao' class='msg'></div>\n"
      << "  </div>\n";
    return s.str();
}

std::string ControlesHTML::gerarJavaScript() const {
    std::ostringstream s;
    s << "<script>\n"
      << "async function enviarComando(cmd) {\n"
      << "  try {\n"
      << "    const resp = await fetch('/comando', {\n"
      << "      method: 'POST',\n"
      << "      headers: {'Content-Type': 'application/json'},\n"
      << "      body: JSON.stringify(cmd)\n"
      << "    });\n"
      << "    const data = await resp.json();\n"
      << "    if (data.sucesso) {\n"
      << "      window.location.reload();\n"
      << "    } else {\n"
      << "      alert('Erro: ' + data.mensagem);\n"
      << "    }\n"
      << "  } catch (e) {\n"
      << "    alert('Erro de comunicação: ' + e.message);\n"
      << "  }\n"
      << "}\n"
      << "\n"
      << "function irTick() {\n"
      << "  const t = parseInt(document.getElementById('tickDestino').value);\n"
      << "  enviarComando({tipo:'IR_TICK', valor: t});\n"
      << "}\n"
      << "\n"
      << "function modificarTarefa() {\n"
      << "  const id = parseInt(document.getElementById('tarefaSel').value);\n"
      << "  const campo = document.getElementById('campoSel').value;\n"
      << "  const valor = document.getElementById('valorCampo').value;\n"
      << "  enviarComando({\n"
      << "    tipo: 'MOD_TAREFA',\n"
      << "    idTarefa: id,\n"
      << "    campo: campo,\n"
      << "    valorCampo: valor\n"
      << "  });\n"
      << "}\n"
      << "</script>\n";
    return s.str();
}