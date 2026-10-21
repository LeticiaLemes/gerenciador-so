#include "core/relogio.h"

// Construtor padrão: começa em 0.

Relogio::Relogio() : tickAtual_(0) {}


// Construtor com tick inicial. Se negativo, começa em 0.

Relogio::Relogio(int64_t tickInicial)
    : tickAtual_(tickInicial < 0 ? 0 : tickInicial) {
}


// Retorna o tick atual.

int64_t Relogio::agora() const {
    return tickAtual_;
}


// Avança um tick. Retorna o novo valor.

int64_t Relogio::avancar() {
    ++tickAtual_;
    return tickAtual_;
}


// Retrocede um tick, sem ir abaixo de 0.

int64_t Relogio::retroceder() {
    if (tickAtual_ > 0) {
        --tickAtual_;
    }
    return tickAtual_;
}


// Define o tick diretamente. Usado ao restaurar snapshot do histórico.
// Ignora valores negativos.

void Relogio::definir(int64_t novoTick) {
    if (novoTick >= 0) {
        tickAtual_ = novoTick;
    }
}

// Reinicia para 0.

void Relogio::reset() {
    tickAtual_ = 0;
}

// Sempre válido (tick >= 0 por construção).

bool Relogio::valido() const {
    return tickAtual_ >= 0;
}

/*teste

static void testarConstrutorPadrao() {
    Relogio r;
    assert(r.agora() == 0);
    std::cout << "[OK] construtor padrão começa em 0\n";
}

static void testarConstrutorComTick() {
    Relogio r(5);
    assert(r.agora() == 5);
    std::cout << "[OK] construtor com tick inicial\n";
}

static void testarConstrutorNegativo() {
    Relogio r(-3);
    assert(r.agora() == 0);  // negativo é corrigido para 0
    std::cout << "[OK] construtor negativo vira 0\n";
}

static void testarAvancar() {
    Relogio r;
    assert(r.avancar() == 1);
    assert(r.avancar() == 2);
    assert(r.agora() == 2);
    std::cout << "[OK] avancar incrementa o tick\n";
}

static void testarRetroceder() {
    Relogio r(2);
    assert(r.retroceder() == 1);
    assert(r.retroceder() == 0);
    assert(r.retroceder() == 0);  // não vai abaixo de 0
    std::cout << "[OK] retroceder não vai abaixo de 0\n";
}

static void testarDefinir() {
    Relogio r(3);
    r.definir(10);
    assert(r.agora() == 10);
    r.definir(-5);          // ignorado
    assert(r.agora() == 10);
    std::cout << "[OK] definir ignora valores negativos\n";
}

static void testarReset() {
    Relogio r(7);
    r.reset();
    assert(r.agora() == 0);
    std::cout << "[OK] reset volta para 0\n";
}

int main() {
    testarConstrutorPadrao();
    testarConstrutorComTick();
    testarConstrutorNegativo();
    testarAvancar();
    testarRetroceder();
    testarDefinir();
    testarReset();
    std::cout << "Todos os testes do relógio passaram.\n";
    return 0;
}*/
