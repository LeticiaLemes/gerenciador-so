#pragma once
#include <string>
#include <vector>
#include <cstdint>

// Tipos de evento que uma tarefa pode ter
enum class TipoEvento {
    DESCONHECIDO,
    MUTEX_SOLICITA,
    MUTEX_LIBERA,
    E_S,
    ENVIO_DADOS,
    RECEBIMENTO_DADO
};

// Um evento associado a uma tarefa (instante + tipo+ parâmetros)
struct Evento {
    int tick = 0;
    TipoEvento tipo = TipoEvento::DESCONHECIDO;
    std::string parametros;     //ex: mutext1 ou dispositivo2
};

// Estados possíveis de uma tarefa
enum class EstadoTarefa {
    PRONTA,         // ativação em andamento, esperando CPU
    EXECUTANDO,     // ativação em andamento, ocupando uma CPU
    SUSPENSA,       // bloqueada por evento (mutex/E-S) - Projeto B
    TERMINADA,      // concluiu todas as ativações
    IGNORADA,       // Tarefas aperiódicas
    INATIVA         // sem ativação em andamento (antes do ingresso ou
                    // aguardando o próximo período)
};

// Task Control Block (TCB) - Estrutura que representa uma tarefa
// requisto 1.3

struct TCB {
    // Campos lidos de arquivos de configuação
    int id = 0;
    std::string corHex = "FFFFFF";
    int ingresso = 0;
    int deadline = 0;   // instante de criação
    int duracao = 0;   // tempo total de execução
    int periodo = 0;    // >0 periódica, 0 aperiódica, <0 erro
    int prazo = 0;     // instante de deadline
    std::vector<Evento> eventos; // eventos associados a tarefa

    // Estágio dinâmico (preenchido na simulação)
    EstadoTarefa estado = EstadoTarefa::PRONTA;
    int tempoExecutado =0;   // quanto já executou
    int ativacoes = 0;   // quantas vezes foi ativada
    int deadlineAtual = 0;   // instante de deadline atual
    int cpuAtual = -1;   // -1 = nenhuma
    bool perdeuDeadline = false;   // se perdeu deadline

    // Coresjá invertidas para uso GUI
    uint8_t r = 255, g = 255, b = 255;
};

// Parâmetros gerais do sistema (primeira linha do arquivo)
struct ParametrosSistema {
    std::string algoritmo = "RM";   //"RM"ou "EDF"
    int quantum = 1;   // período máximo de execução
    int qtdeCpus = 1;   // 1.. N
};

// Operações sobre o TCB usadas pelo Simulador.
namespace TCBUtils {

    // Projeto A: tarefas periódicas terminam após 10 ativações.
    constexpr int MAX_ATIVACOES = 10;

    inline bool ehPeriodica(const TCB& t) { return t.periodo > 0; }

    inline bool podeAtivar(const TCB& t) {
        return t.estado != EstadoTarefa::TERMINADA &&
               t.estado != EstadoTarefa::IGNORADA;
    }

    inline bool atingiuLimiteAtivacoes(const TCB& t) {
        return t.ativacoes >= MAX_ATIVACOES;
    }

    inline bool ativacaoConcluida(const TCB& t) {
        return t.tempoExecutado >= t.duracao;
    }

    // Prazo relativo efetivo: prazo=0 significa "igual ao período".
    inline int prazoRelativo(const TCB& t) {
        return t.prazo > 0 ? t.prazo : t.periodo;
    }

    // Começa uma nova ativação no instante 'tick'.
    inline void iniciarNovaAtivacao(TCB& t, int tick) {
        ++t.ativacoes;
        t.tempoExecutado = 0;
        t.estado = EstadoTarefa::PRONTA;
        t.cpuAtual = -1;
        t.perdeuDeadline = false;
        t.deadlineAtual = tick + prazoRelativo(t);
    }

    inline void terminar(TCB& t) {
        t.estado = EstadoTarefa::TERMINADA;
        t.cpuAtual = -1;
    }
}