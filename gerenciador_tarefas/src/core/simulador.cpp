#include "core/simulador.h"
#include "escalonadores/rm.h"
#include <algorithm>


// Inicialização


void Simulador::inicializar(const ParametrosSistema& sistema,
                            const std::vector<TCB>& tarefas) {
    
    sistema_ = sistema;
    tarefas_ = tarefas;

    // Estado dinâmico limpo: tarefas periódicas começam INATIVAS (o parser
    // deixa o estado padrão PRONTA, que aqui significaria "na fila").
    for (auto& t : tarefas_) {
        t.tempoExecutado = 0;
        t.ativacoes = 0;
        t.deadlineAtual = 0;
        t.cpuAtual = -1;
        t.perdeuDeadline = false;
        if (t.estado != EstadoTarefa::IGNORADA) {
            t.estado = EstadoTarefa::INATIVA;
        }
    }

    //Cria o escalonador conforme  o algoritmo configurado
    escalonador_ = criarEscalonador(sistema_.algoritmo);
    if (!escalonador_) {
        // Erro: algoritmo desconhecido (já validado no parser, mas por segurança)
       // O parser garante que só chega "RM" ou "EDF".
        escalonador_ = std::make_unique<EscalonadorRM>();  // fallback
    }


    // Cria as CPUs (0..N-1).
    cpus_.clear();
    cpus_.reserve(sistema_.qtdeCpus);
    for (int i = 0; i < sistema_.qtdeCpus; ++i) {
        CPU c;
        c.id = i;
        cpus_.push_back(c);
    }

    // Limpa fila e eventos.
    filaProntos_.limpar();
    eventosVisuais_.clear();
    segmentos_.clear();

    // Reinicia o relógio.
    relogio_.reset();

    simulacaoIniciada_ = true;
    simulacaoTerminada_ = false;

    historico_.limpar();
    salvarSnapshot();
}


// Avanço de ticks


void Simulador::avancarUmTick() {
    if (simulacaoTerminada_) return;

    int tick = static_cast<int>(relogio_.agora());

    // 1. Verifica chegada e ativações periódicas.
    verificarAtivacoes(tick);

    // 2. Verifica deadlines estourados.
    verificarDeadlines(tick);

    // 3. Escalona tarefas para CPUs livres.
    escalonar(tick);

    // 4. Registra o que cada tarefa faz neste tick (segmentos do Gantt).
    registrarSegmentos(tick);

    // 5. Executa um tick em cada CPU.
    executarCpus(tick);

    // 6. Pós-execução: libera CPUs de tarefas que concluíram a ativação.
    posExecucao(tick);

    // 7. Verifica se terminou.
    simulacaoTerminada_ = verificarTermino();

    // 8. Avança o relógio.
    relogio_.avancar();

    salvarSnapshot();
}

void Simulador::avancarNTicks(int n) {
    for (int i = 0; i < n && !simulacaoTerminada_; ++i) {
        avancarUmTick();
    }
}

bool Simulador::retrocederUmTick() {
    // Precisa de pelo menos 2 snapshots: o atual e o anterior.
    if (historico_.tamanho() < 2) return false;

    // Descarta o snapshot atual.
    historico_.desempilhar();

    // O novo atual é o anterior.
    const Snapshot* s = historico_.atual();
    if (s == nullptr) return false;

    restaurarSnapshot(*s);
    return true;
}

bool Simulador::retrocederNTicks(int n) {
    bool ok = true;
    for (int i = 0; i < n; ++i) {
        if (!retrocederUmTick()) {
            ok = false;
            break;
        }
    }
    return ok;
}

bool Simulador::irParaTick(int tick) {
    int atual = static_cast<int>(relogio_.agora());
    if (tick < 0) return false;

    if (tick < atual) {
        return retrocederNTicks(atual - tick);
    }
    else if (tick > atual) {
        avancarNTicks(tick - atual);
        return true;
    }
    return true;  // já está no tick
}


void Simulador::executarCompleto() {
    // Limite de segurança para evitar loop infinito em caso de bug.
    const int LIMITE_TICKS = 1000000;
    int ticks = 0;
    while (!simulacaoTerminada_ && ticks < LIMITE_TICKS) {
        avancarUmTick();
        ++ticks;
    }
}

Snapshot Simulador::construirSnapshot() const {
    Snapshot s;
    s.tick = static_cast<int>(relogio_.agora());
    s.sistema = sistema_;
    s.tarefas = tarefas_;   // cópia profunda
    s.eventosVisuais = eventosVisuais_;
    s.segmentos = segmentos_;
    s.simulacaoTerminada = simulacaoTerminada_;

    // Converte CPUs para CPUSnapshot (sem ponteiros).
    s.cpus.clear();
    for (const auto& cpu : cpus_) {
        CPUSnapshot cs;
        cs.id = cpu.id;
        cs.idTarefaAtual = (cpu.tarefaAtual != nullptr)
            ? cpu.tarefaAtual->id
            : -1;
        cs.ligada = cpu.ligada;
        cs.ticksLigada = cpu.ticksLigada;
        cs.ticksDesligada = cpu.ticksDesligada;
        cs.ticksNoQuantum = cpu.ticksNoQuantum;
        s.cpus.push_back(cs);
    }

    return s;
}

void Simulador::salvarSnapshot() {
    historico_.salvar(construirSnapshot());
}

void Simulador::restaurarSnapshot(const Snapshot& s) {
    // Restaura relógio.
    relogio_.definir(s.tick);

    // Restaura parâmetros e tarefas.
    sistema_ = s.sistema;
    tarefas_ = s.tarefas;

    // Restaura eventos e segmentos.
    eventosVisuais_ = s.eventosVisuais;
    segmentos_ = s.segmentos;
    simulacaoTerminada_ = s.simulacaoTerminada;

    // Restaura CPUs (reconstrói ponteiros).
    cpus_.clear();
    for (const auto& cs : s.cpus) {
        CPU cpu;
        cpu.id = cs.id;
        cpu.ligada = cs.ligada;
        cpu.ticksLigada = cs.ticksLigada;
        cpu.ticksDesligada = cs.ticksDesligada;
        cpu.ticksNoQuantum = cs.ticksNoQuantum;

        // Reconstrói ponteiro para TCB.
        if (cs.idTarefaAtual >= 0) {
            cpu.tarefaAtual = buscarTarefa(cs.idTarefaAtual);
        }
        else {
            cpu.tarefaAtual = nullptr;
        }
        cpus_.push_back(cpu);
    }

    // Reconstrói fila de prontos a partir dos estados.
    filaProntos_.limpar();
    for (auto& t : tarefas_) {
        if (t.estado == EstadoTarefa::PRONTA) {
            filaProntos_.inserir(&t);
        }
    }
}


// Etapa 1: Verificar ativações (ingresso e periódica)


void Simulador::verificarAtivacoes(int tick) {
    for (auto& t : tarefas_) {
        // Ignora tarefas já terminadas ou ignoradas.
        if (!TCBUtils::podeAtivar(t)) continue;

        // Ativação inicial: no tick de ingresso.
        if (t.ativacoes == 0 && tick == t.ingresso) {
            TCBUtils::iniciarNovaAtivacao(t, tick);
            filaProntos_.inserir(&t);
            registrarEvento(tick, t.id, EventoVisual::Tipo::CHEGADA);
            continue;
        }

        // Ativação periódica: a cada periodo ticks após o ingresso.
        if (TCBUtils::ehPeriodica(t) && t.ativacoes > 0) {
            int proximaAtivacao = t.ingresso + t.ativacoes * t.periodo;
            if (tick != proximaAtivacao) continue;

            // Ativação anterior ainda não concluiu: perdeu o prazo e é
            // abortada para dar lugar à nova (um TCB só comporta uma
            // ativação por vez).
            if (t.estado == EstadoTarefa::PRONTA ||
                t.estado == EstadoTarefa::EXECUTANDO) {
                if (!t.perdeuDeadline) {
                    t.perdeuDeadline = true;
                    registrarEvento(tick, t.id,
                                    EventoVisual::Tipo::PERDEU_DEADLINE);
                }
                if (t.cpuAtual >= 0) {
                    cpus_[t.cpuAtual].liberar();
                }
                filaProntos_.removerPorId(t.id);
            }

            // Se já atingiu 10 ativações, não ativa mais.
            if (TCBUtils::atingiuLimiteAtivacoes(t)) {
                TCBUtils::terminar(t);
                registrarEvento(tick, t.id, EventoVisual::Tipo::TERMINO);
                continue;
            }
            TCBUtils::iniciarNovaAtivacao(t, tick);
            filaProntos_.inserir(&t);
            registrarEvento(tick, t.id, EventoVisual::Tipo::CHEGADA);
        }
    }
}


// Etapa 2: Verificar deadlines


void Simulador::verificarDeadlines(int tick) {
    for (auto& t : tarefas_) {
        // Só ativações em andamento podem perder o prazo.
        if (t.estado != EstadoTarefa::PRONTA &&
            t.estado != EstadoTarefa::EXECUTANDO) {
            continue;
        }
        // No início do tick == deadline a ativação ainda não terminou: perdeu.
        if (tick >= t.deadlineAtual && !t.perdeuDeadline) {
            t.perdeuDeadline = true;
            registrarEvento(tick, t.id, EventoVisual::Tipo::PERDEU_DEADLINE);
        }
    }
}


// Etapa 3: Escalonar (preemptivo, fila global)
//
// Uma CPU é "ponto de decisão" quando está ociosa ou quando a tarefa que ela
// executa já usou o quantum inteiro (período máximo de execução contínua).
// As tarefas das CPUs que ainda estão dentro do quantum não são tocadas.
//
// Nos pontos de decisão, competem a fila de prontos e as tarefas que estão
// nessas CPUs. As N melhores (N = pontos de decisão) ficam com as CPUs:
//  - quem já estava rodando continua na mesma CPU (sem migração);
//  - quem não foi escolhido é preemptado e volta para a fila;
//  - as demais escolhidas ocupam as CPUs que sobraram.


void Simulador::escalonar(int tick) {
    const int quantum = std::max(1, sistema_.quantum);

    // 1. Vagas e candidatas.
    std::vector<TCB*> pool = filaProntos_.copiar();
    int vagas = 0;
    for (auto& cpu : cpus_) {
        if (cpu.ociosa()) {
            ++vagas;
        } else if (cpu.ticksNoQuantum >= quantum) {
            pool.push_back(cpu.tarefaAtual);
            ++vagas;
        }
    }
    if (vagas == 0 || pool.empty()) return;

    // 2. Escolhe as 'vagas' tarefas de maior prioridade.
    FilaProntos candidatas;
    for (TCB* t : pool) candidatas.inserir(t);

    std::vector<TCB*> escolhidas;
    while (static_cast<int>(escolhidas.size()) < vagas && !candidatas.vazia()) {
        int vagasRestantes = vagas - static_cast<int>(escolhidas.size());
        int empatadas = 1;
        TCB* t = escalonador_->escolher(candidatas, tick, &empatadas);
        if (t == nullptr) break;

        // O sorteio só importa se alguma das empatadas ficar de fora.
        if (empatadas > vagasRestantes) {
            registrarEvento(tick, t->id, EventoVisual::Tipo::SORTEIO);
        }
        candidatas.removerPorId(t->id);
        escolhidas.push_back(t);
    }
    auto foiEscolhida = [&](const TCB* t) {
        return std::find(escolhidas.begin(), escolhidas.end(), t)
               != escolhidas.end();
    };

    // 3. Preempção: quem estava rodando e não foi escolhido volta para a fila.
    //    Quem foi escolhido de novo continua, com um quantum novo.
    for (auto& cpu : cpus_) {
        if (cpu.ociosa() || cpu.ticksNoQuantum < quantum) continue;

        TCB* t = cpu.tarefaAtual;
        if (foiEscolhida(t)) {
            cpu.ticksNoQuantum = 0;
        } else {
            cpu.liberar();
            t->estado = EstadoTarefa::PRONTA;
            filaProntos_.inserir(t);
        }
    }

    // 4. Despacha as escolhidas que ainda não estão em nenhuma CPU.
    for (TCB* t : escolhidas) {
        if (t->cpuAtual >= 0) continue;   // já está rodando

        CPU* livre = nullptr;
        for (auto& cpu : cpus_) {
            if (cpu.ociosa()) { livre = &cpu; break; }
        }
        if (livre == nullptr) break;

        filaProntos_.removerPorId(t->id);
        livre->atribuir(t);
    }
}


// Etapa 4: Registrar segmentos do Gantt
//
// Cada tick [tick, tick+1) é descrito pelo estado em que a tarefa passou
// o tick: EXECUTANDO (com a CPU) ou PRONTA (esperando CPU, cpu = -1).
// Ticks consecutivos com o mesmo (estado, CPU) são fundidos num só segmento.
// Tarefas INATIVAS/TERMINADAS/IGNORADAS não geram segmento.


void Simulador::registrarSegmentos(int tick) {
    for (const auto& t : tarefas_) {
        int cpu = -1;
        if (t.estado == EstadoTarefa::EXECUTANDO) {
            cpu = t.cpuAtual;
        } else if (t.estado != EstadoTarefa::PRONTA) {
            continue;
        }

        SegmentoGantt* ultimo = nullptr;
        for (auto it = segmentos_.rbegin(); it != segmentos_.rend(); ++it) {
            if (it->idTarefa == t.id) { ultimo = &*it; break; }
        }

        if (ultimo != nullptr && ultimo->fim == tick &&
            ultimo->cpu == cpu && ultimo->estado == t.estado) {
            ultimo->fim = tick + 1;
        } else {
            SegmentoGantt seg;
            seg.idTarefa = t.id;
            seg.cpu = cpu;
            seg.inicio = tick;
            seg.fim = tick + 1;
            seg.estado = t.estado;
            segmentos_.push_back(seg);
        }
    }
}


// Etapa 5: Executar CPUs


void Simulador::executarCpus(int /*tick*/) {
    // Também contabiliza o tick das CPUs ociosas (ticksDesligada).
    for (auto& cpu : cpus_) {
        cpu.executarUmTick();
    }
}


// Etapa 6: Pós-execução


void Simulador::posExecucao(int tick) {
    for (auto& cpu : cpus_) {
        TCB* t = cpu.tarefaAtual;
        if (t == nullptr) continue;

        if (!TCBUtils::ativacaoConcluida(*t)) continue;

        // A ativação terminou ao fim deste tick, ou seja, em tick + 1.
        cpu.liberar();
        registrarEvento(tick + 1, t->id, EventoVisual::Tipo::TERMINO);

        if (TCBUtils::atingiuLimiteAtivacoes(*t)) {
            TCBUtils::terminar(*t);
        } else {
            // Aguarda o próximo período; a nova ativação reinsere na fila.
            t->estado = EstadoTarefa::INATIVA;
        }
    }
}

// Etapa 7: Verificar término


bool Simulador::verificarTermino() const {
    for (const auto& t : tarefas_) {
        if (t.estado != EstadoTarefa::TERMINADA &&
            t.estado != EstadoTarefa::IGNORADA) {
            return false;
        }
    }
    return true;
}


// Auxiliares


void Simulador::registrarEvento(int tick, int idTarefa,
    EventoVisual::Tipo tipo) {
    EventoVisual ev;
    ev.tick = tick;
    ev.idTarefa = idTarefa;
    ev.tipo = tipo;
    eventosVisuais_.push_back(ev);
}

TCB* Simulador::buscarTarefa(int id) {
    for (auto& t : tarefas_) {
        if (t.id == id) return &t;
    }
    return nullptr;
}


// Estado para a GUI


EstadoSimulacao Simulador::obterEstado() const {
    EstadoSimulacao estado;
    estado.tickAtual = static_cast<int>(relogio_.agora());
    estado.sistema = sistema_;
    estado.tarefas = tarefas_;
    estado.segmentos = segmentos_;
    estado.eventosVisuais = eventosVisuais_;
    estado.simulacaoIniciada = simulacaoIniciada_;
    estado.simulacaoTerminada = simulacaoTerminada_;

    // Tempo de CPU desligada (requisito 1.2).
    estado.tempoCpuDesligada.clear();
    for (const auto& cpu : cpus_) {
        estado.tempoCpuDesligada.push_back(cpu.ticksDesligada);
    }

    return estado;
}

bool Simulador::terminou() const {
    return simulacaoTerminada_;
}

int Simulador::tickAtual() const {
    return static_cast<int>(relogio_.agora());
}

bool Simulador::modificarTarefa(int id, const std::string& campo,
                                const std::string& valor, std::string& erro) {
    TCB* t = buscarTarefa(id);
    if (!t) {
        erro = "Tarefa id=" + std::to_string(id) + " não encontrada.";
        return false;
    }

    try {
        if (campo == "duracao") {
            int v = std::stoi(valor);
            if (v <= 0) { erro = "duração deve ser > 0."; return false; }
            t->duracao = v;
        } else if (campo == "periodo") {
            int v = std::stoi(valor);
            if (v < 0) { erro = "período deve ser >= 0."; return false; }
            t->periodo = v;
        } else if (campo == "prazo") {
            int v = std::stoi(valor);
            if (v < 0) { erro = "prazo deve ser >= 0."; return false; }
            t->prazo = v;
        } else if (campo == "ingresso") {
            int v = std::stoi(valor);
            if (v < 0) { erro = "ingresso deve ser >= 0."; return false; }
            t->ingresso = v;
        } else if (campo == "cor") {
            // Validar formato hex (6 dígitos)
            if (valor.size() != 6) {
                erro = "cor deve ter 6 dígitos hexadecimais.";
                return false;
            }
            // Converter para RGB
            t->corHex = valor;
            t->r = static_cast<uint8_t>(std::stoi(valor.substr(0, 2), nullptr, 16));
            t->g = static_cast<uint8_t>(std::stoi(valor.substr(2, 2), nullptr, 16));
            t->b = static_cast<uint8_t>(std::stoi(valor.substr(4, 2), nullptr, 16));
        } else {
            erro = "Campo '" + campo + "' não pode ser modificado.";
            return false;
        }
    } catch (...) {
        erro = "Valor inválido para o campo '" + campo + "'.";
        return false;
    }

    // Salva snapshot após a modificação (para permitir retroceder)
    salvarSnapshot();
    return true;
}