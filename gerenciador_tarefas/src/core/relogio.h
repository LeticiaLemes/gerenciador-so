#pragma once
#include <cstdint>

// Relógio global do sistema simulado.

// Responsabilidade única: manter o tick atual e permitir avançar/retroceder.
// A passagem do tempo é simulada (requisito 1.1): não há espera em tempo real.

// O relógio NÃO verifica eventos, NÃO decide escalonamento e NÃO conhece
// tarefas. Quem faz isso é o Simulador, a cada tick.

class Relogio {
public:
    // Cria um relógio começando no tick 0.
    Relogio();

    // Cria um relógio começando em um tick específico (útil para restaurar
    // histórico ou configurar o início da simulação).
    explicit Relogio(int64_t tickInicial);

    // Retorna o tick atual.
    int64_t agora() const;

    // Avança um tick. Retorna o novo tick.
    int64_t avancar();

    // Retrocede um tick. Não permite ir abaixo de 0.
    // Retorna o novo tick.
    int64_t retroceder();

    // Define o tick diretamente. Usado ao restaurar um snapshot do histórico.
    // Valores negativos são rejeitados (mantém o tick atual).
    void definir(int64_t novoTick);

    // Reinicia para o tick 0.
    void reset();

    // Retorna true se o tick atual é >= 0 (sempre true, mas útil para
    // documentar a intenção em verificações).
    bool valido() const;

private:
    int64_t tickAtual_;   // tick atual da simulação
};