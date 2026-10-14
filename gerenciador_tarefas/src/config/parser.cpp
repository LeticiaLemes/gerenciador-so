#include "config/parser.h"
#include "config/defaults.h"
#include "config/validacao.h"
#include "utils/string_utils.h"
#include "utils/cores.h"
#include <fstream>

namespace Parser
{

    using namespace StringUtils;

    // Remove o ; final, se houver
    // Requisito 3.3.3
    static std::string removerPVFinal(const std::string &linha)
    {
        std::string limpa = trim(linha);
        if (!limpa.empty() && limpa.back() == ';')
            limpa.pop_back();
        return limpa;
    }

    // Primeira linha:algoritmo_escal;quantum;qtde_cpus
    bool parseLinhaSistema(const std::string &linha, ParametrosSistema &saida, std::string &erro)
    {
        std::string limpa = removerPVFinal(linha);
        auto campos = split(limpa, ';');

        // Campo 0 - algoritmo (case-insensitive)
        // Requisito 3.3.2
        std::string alg = campos.empty() ? "" : toUpper(campos[0]);
        if (alg.empty())
            alg = Defaults::ALGORITMO;
        saida.algoritmo = alg;

        // Campo 1 - quantum (opcional)
        if (campos.size() >= 2 && !campos[1].empty())
        {
            int q = 0;
            if (!parseInt(campos[1], q))
            {
                erro = "quantum inválido ('" + campos[1] + "').";
                return false;
            }
            saida.quantum = q;
        }
        else
        {
            saida.quantum = Defaults::QUANTUM;
        }

        // Campo 2 - qtde_cpus (opcional)
        if (campos.size() >= 3 && !campos[2].empty())
        {
            int c = 0;
            if (!parseInt(campos[2], c))
            {
                erro = "quantidade de CPUs inválida ('" + campos[2] + "').";
                return false;
            }
            saida.qtdeCpus = c;
        }
        else
        {
            saida.qtdeCpus = Defaults::QTDE_CPUS;
        }

        // Validação semântica (algoritimo, quantum, cpus)
        return Validacao::validarSistema(saida, erro);
    }

    // Linha de tarefas
    // id; cor; ingresso; duração; período;prazo; lista_eventos
    bool parseLinhaTarefa(const std::string &linha, int numeroLinha, TCB &saida, std::string &erro, std::string &aviso)
    {
        std::string limpa = removerPVFinal(linha);
        auto campos = split(limpa, ';');

        // Mínimo:6 campos
        if (campos.size() < 6)
        {
            erro = "Esperados ao menos 6 campos "
                   "(id; cor;ingresso; duração; período; prazo), encontrados: " +
                   std::to_string(campos.size()) + ".";
            return false;
        }

        // - id -
        if (!parseInt(campos[0], saida.id))
        {
            erro = "id inválido ('" + campos[0] + "').";
            return false;
        }

        // - cor -
        std::string cor = toUpper(campos[1]);
        if (cor.empty())
        {
            cor = Defaults::COR;
        }
        saida.corHex = cor;

        // - ingresso -
        if (!parseInt(campos[2], saida.ingresso))
        {
            erro = "ingresso inválido ('" + campos[2] + "').";
            return false;
        }

        // - duracao -
        if (!parseInt(campos[3], saida.duracao))
        {
            erro = "duração inválida ('" + campos[3] + "').";
            return false;
        }

        // - período - 
        if (!parseInt(campos[4], saida.periodo)) {
            erro = "período inválido ('" + campos[4] + "').";
            return false;
        }

        // - prazo - 
        if (!parseInt(campos[5], saida.prazo)) {
            erro = "prazo inválido('" + campos[5] + "').";
            return false;
        }

        // - lista eventos - 
        if (campos.size() >= 7 && !campos[6].empty()) {
            Evento ev;
            ev.tick = 0;
            ev.tipo = TipoEvento::DESCONHECIDO;
            ev.parametros = campos[6];
            saida.eventos.push_back(ev);
        }

        // Aplica padrões a campos opcionais vazios
        Defaults::aplicarPadroesTarefa(saida);

        // Validação semântica
        if (!Validacao::validarTarefa(saida, erro, aviso)) {
            return false;
        }

        Cores::hexParaRgb(saida.corHex, saida.r, saida.g, saida.b);

        // Se a tarefa é aperiódica, marca como IGNORADA
        // Requisito 4.4
        if (saida.periodo == 0) {
            saida.estado = EstadoTarefa::IGNORADA;
            if (!aviso.empty())
                aviso = "linha " + std::to_string(numeroLinha) + ": " + aviso;
        }

        return true;
    }

    // Leitura completa do arquivo
    ResultadoConfig lerArquivo(const std::string& caminho) {
        ResultadoConfig resultado;

        std::ifstream arquivo(caminho);
        if (!arquivo.is_open()) {
            resultado.erros.push_back("não foi possível abrir o arquivo '" + caminho + "' verifique o caminho e permissões");
            return resultado;
        }

        std::string linha;
        int numeroLinha = 0;
        bool sistemaLido = false;

        while (std::getline(arquivo, linha)) {
            numeroLinha++;

            // Ignora linhas em branco
            // Requisito 3.3.6
            if (isBlank(linha)) continue;

            if (!sistemaLido) {
                // Primeira linha  não vazia = sistema
                std::string erro;
                if (!parseLinhaSistema(linha, resultado.sistema, erro)) {
                    resultado.erros.push_back("linha " + std::to_string(numeroLinha) + ": " + erro);
                    return resultado;
                }
                sistemaLido = true;
            } else {
                // Linhas seguintes = tarefas
                TCB t;
                std::string erro, aviso;
                if(!parseLinhaTarefa(linha, numeroLinha, t, erro, aviso)) {
                    resultado.erros.push_back("linha " + std::to_string(numeroLinha) + ": " + erro);
                    continue;
                }
                bool idRepetido = false;
                for (const auto& existente : resultado.tarefas) {
                    if (existente.id == t.id) {
                        idRepetido = true;
                        break;
                    }
                }
                if (idRepetido) {
                    resultado.erros.push_back("linha " + std::to_string(numeroLinha) + ": id " + std::to_string(t.id) + " repetido.");
                    continue;
                }
                if (!aviso.empty())
                    resultado.avisos.push_back(aviso);
                resultado.tarefas.push_back(t);
            }
        }

        if (!sistemaLido) {
            resultado.erros.push_back("arquivo vazio ou sem linha de parâmetros de sistema");
            return resultado;
        }

        // Verifica se há ao menos uma tarefa válida (não IGNORADA)
        bool temTarefaValida = false;
        for (const auto& t : resultado.tarefas) {
            if(t.estado != EstadoTarefa:: IGNORADA) {
                temTarefaValida = true;
                break;
            }
        }
        if (!temTarefaValida) 
            resultado.avisos.push_back("nenhuma tarefa periódica encontrada. " "A simulação não terá o que executar");

        resultado.sucesso = resultado.erros.empty();
        return resultado;
    }
}