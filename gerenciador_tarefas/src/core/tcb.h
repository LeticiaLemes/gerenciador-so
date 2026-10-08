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
    PRONTA,
    EXECUTANDO,
    SUSPENSA, 
    TERMINADA,
    IGNORADA        // Tarefas aperiódicas
};

// Task Control Block (TCB) - Estrutura que representa uma tarefa
// requisto 1.3

struct TCB {
    // Campos lidos de arquivos de configuação
    int id = 0;
    std::string corHex= "#FFFFFF";
    int ingresso = 0;
    int deadline = 0;   // instante de criação
    int duracao = 0;   // tempo total de execução
    int periodo = 0;    // >0 periódica, 0 aperiódica, <0 erro
    int prazo = 0;     // instante de deadline
    std::vector<Evento> eventos; // eventos associados a tarefa

    // Estágio dinâmico (preenchido na simulação)
    EstadoTarefa estado = EstadoTarefa::PRONTA;
    int tempoExecutado =0;   // quanto já executou
    int ativações = 0;   // quantas vezes foi ativada
    int deadlineAtual = 0;   // instante de deadline atual
    int cpuAtual = -1;   // -1 = nenhuma
    bool perdeuDeadline = false;   // se perdeu deadline

    // Coresjá invertidas para uso GUI
    uint8_t r = 255, g = 255, b = 255;
};

// Parâmetros gerais do sistema (primeira linha do arquivo)
struct ParamSistema {
    std::string algoritmo = "RM";   //"RM"ou "EDF"
    int quantum = 1;   // período máximo de execução
    int qtdCpus = 1;   // 1.. N
};
