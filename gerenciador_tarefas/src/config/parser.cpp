#include "config/parser.h"
#include "config/defaults.h"
#include "config/validacao.h"
#include "utils/string_utils.h"
#include "utils/cores.h"
#include <fstream>

namespace Parser
{

    using namespace Stringutils;

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

        if (campos.empty() || campos[0].empty())
        {
            erro = "linha do sistema vazia ou sem algoritmo";
            return false;
        }

        // Campo 0 - algoritmo (case-insensitive)
        // Requisito 3.3.2
        std::string alg = toUpper(campos[0]);
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
        if (parseInt(campos[0], saida.id))
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
        if (campos.size() >= 7 && !campos[0].empty()) {
            Evento ev;
            ev.tick = 0;
            ev.tipo = TipoEvento::DESCONHECIDO;
            ev.parametros = campos[6];
            saida.eventos.push_back(ev);
        }
    }