#pragma once
#include "core/relogio.h"
#include "core/tcb.h"
#include "core/fila_prontos.h"
#include "core/cpu.h"
#include "core/estado_simulacao.h"
#include <vector>
#include <string>
#include "escalonadores/escalonador.h"
#include <memory>
#include "core/historico.h"
#include "core/comando.h"


// Simulador do sistema multitarefa preemptivo.

// Responsável por:
//  - verificar eventos a cada tick (chegada, ativação periódica, deadline)
//  - escalonar tarefas para CPUs livres
//  - executar um tick em cada CPU
//  - registrar eventos visuais para o Gantt
//  - expor o estado atual para a GUI (EstadoSimulacao)

// NÃO lê arquivo de configuração (isso é da Pessoa 2).
// NÃO desenha Gantt (isso é da Pessoa 2).

class Simulador {
public:
    Simulador() = default;

    // Inicializa a simulação com os parâmetros e tarefas carregados do arquivo.
    // Cria as CPUs e prepara o estado.
    void inicializar(const ParametrosSistema& sistema,
        const std::vector<TCB>& tarefas);

    // Avança exatamente 1 tick.
    void avancarUmTick();

    // Avança N ticks (chama avancarUmTick N vezes).
    void avancarNTicks(int n);

    // Retrocede 1 tick (restaura snapshot anterior).
    // Retorna true se conseguiu retroceder.
    bool retrocederUmTick();

    // Retrocede N Ticks
    bool retrocederNTicks(int n);

    // Vai para o tick N (avança ou retrocede conforme necessário).
    bool irParaTick(int tick);

    // Executa até a simulação terminar (todas as tarefas periódicas
    // atingirem 10 ativações ou não haver mais eventos).
    void executarCompleto();


    // Modifica um campo da tarefa.
    // Retorna struct com sucesso e mensagem.
    ResultadoComando modificarTarefa(int idTarefa, const std::string& campo, const std::string& valor);

    // Retorna o estado atual para a GUI.
    EstadoSimulacao obterEstado() const;

    // Retorna true se a simulação terminou.
    bool terminou() const;

    // Retorna o tick atual.
    int tickAtual() const;

    // Modifica um campo de uma tarefa. Retorna false se o campo for inválido.
    bool modificarTarefa(int id, const std::string& campo, const std::string& valor, std::string& erro);


private:
    
    
    // Etapas do tick (chamadas em ordem por avancarUmTick)
    

    // 1. Verifica chegada de tarefas (ingresso) e ativações periódicas.
    void verificarAtivacoes(int tick);

    // 2. Verifica deadlines estourados.
    void verificarDeadlines(int tick);

    // 3. Escalona (preemptivo, fila global): ocupa as CPUs ociosas e, nas CPUs
    //    cujo quantum expirou, troca a tarefa se houver outra mais prioritária.
    void escalonar(int tick);

    // 4. Registra o estado de cada tarefa neste tick nos segmentos do Gantt.
    void registrarSegmentos(int tick);

    // 5. Executa um tick em cada CPU.
    void executarCpus(int tick);

    // 6. Pós-execução: tarefas que concluíram a ativação ficam INATIVAS
    //    (aguardam o próximo período) ou TERMINADAS (10 ativações).
    void posExecucao(int tick);

    // 7. Verifica se a simulação terminou (todas as tarefas terminaram).
    bool verificarTermino() const;

    std::unique_ptr<Escalonador>escalonador_;
    
    // Auxiliares
    

    // Registra um evento visual para o Gantt.
    void registrarEvento(int tick, int idTarefa, EventoVisual::Tipo tipo);

    // Retorna ponteiro para TCB pelo id.
    TCB* buscarTarefa(int id);

    
    // Estado interno
    
    Relogio relogio_;
    ParametrosSistema sistema_;
    std::vector<TCB> tarefas_;          // todos os TCBs (fixo após carga)
    FilaProntos filaProntos_;           // fila global
    std::vector<CPU> cpus_;             // N CPUs
    std::vector<EventoVisual> eventosVisuais_;
    std::vector<SegmentoGantt> segmentos_;
    bool simulacaoIniciada_ = false;
    bool simulacaoTerminada_ = false;

    Historico historico_;

    void salvarSnapshot();

    void restaurarSnapshot(const Snapshot& s);

    Snapshot construirSnapshot()const;


};