#include "gui/inspecao_html.h"
#include <sstream>

std::string InspecaoHTML::estadoParaString(EstadoTarefa e)
{
    switch (e)
    {
    case EstadoTarefa::PRONTA:
        return "Pronta";
    case EstadoTarefa::EXECUTANDO:
        return "Executando";
    case EstadoTarefa::SUSPENSA:
        return "Suspensa";
    case EstadoTarefa::TERMINADA:
        return "Terminada";
    case EstadoTarefa::IGNORADA:
        return "Ignorada";
    case EstadoTarefa::INATIVA:
        return "Inativa";
    }
    return "?";
}

std::string InspecaoHTML::tipoEventoParaString(TipoEvento t)
{
    switch (t)
    {
    case TipoEvento::MUTEX_SOLICITA:
        return "Mutex solicita";
    case TipoEvento::MUTEX_LIBERA:
        return "Mutex libera";
    case TipoEvento::E_S:
        return "E/S";
    case TipoEvento::ENVIO_DADOS:
        return "Envio de dados";
    case TipoEvento::RECEBIMENTO_DADO:
        return "Recebimento de dados";
    default:
        return "Desconhecido";
    }
}

std::string InspecaoHTML::gerar(const EstadoSimulacao &estado) const
{
    std::ostringstream s;
    s << "<div  class='inspecao'>\n"
      << " <h3> Inspeção de Tarefas (Tick " << estado.tickAtual << ")</h3>\n"
      << " <table>\n"
      << " <thead>\n"
      << "  <tr>\n"
      << "      <th>ID</th>\n"
      << "      <th>Cor</th>\n"
      << "      <th>Ingresso</th>\n"
      << "      <th>Duração</th>\n"
      << "      <th>Período</th>\n"
      << "      <th>Prazo</th>\n"
      << "      <th>Estado</th>\n"
      << "      <th>Executado</th>\n"
      << "      <th>Ativações</th>\n"
      << "      <th>Deadline</th>\n"
      << "      <th>CPU</th>\n"
      << "      <th>Perdeu Deadline</th>\n"
      << "      <th>Eventos</th>\n"
      << "  </tr>\n"
      << " </thead>\n"
      << " <tbody>\n";

    for (const auto &t : estado.tarefas)
    {
        s << " <tr>\n"
          << " <td>T" << t.id << "</td>\n"
          << " <td><span class='cor' style='background:#"
          << std::hex
          << (static_cast<int>(t.r) < 16 ? "0" : "") << static_cast<int>(t.r)
          << (static_cast<int>(t.g) < 16 ? "0" : "") << static_cast<int>(t.g)
          << (static_cast<int>(t.b) < 16 ? "0" : "") << static_cast<int>(t.b)
          << std::dec << "'></span> " << t.corHex << "</td>\n"
          << "        <td>" << t.ingresso << "</td>\n"
          << "        <td>" << t.duracao << "</td>\n"
          << "        <td>" << t.periodo << "</td>\n"
          << "        <td>" << t.prazo << "</td>\n"
          << "        <td>" << estadoParaString(t.estado) << "</td>\n"
          << "        <td>" << t.tempoExecutado << "</td>\n"
          << "        <td>" << t.ativacoes << "</td>\n"
          << "        <td>" << t.deadlineAtual << "</td>\n"
          << "        <td>" << (t.cpuAtual >= 0 ? std::to_string(t.cpuAtual) : "-")
          << "</td>\n"
          << "        <td>" << (t.perdeuDeadline ? "Sim" : "Não") << "</td>\n"
          << "        <td>";
        if (t.eventos.empty()) {
            s << "-";
        } else {
            for (size_t i = 0; i < t.eventos.size(); ++i) {
                if (i > 0) s << ", ";
                s << tipoEventoParaString(t.eventos[i].tipo)
                  << "@" << t.eventos[i].tick;
            }
        }
        s << "</td>\n"
          << "      </tr>\n";
    }

    s << "    </tbody>\n"
      << "  </table>\n"
      << "</div>\n";
    return s.str();
}