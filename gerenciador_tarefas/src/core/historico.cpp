#include "core/historico.h"

Historico::Historico(std::size_t limite) : limite_(limite) {
    if (limite_ == 0) limite_ = 1;  // evita divisão por zero
}

void Historico::salvar(const Snapshot& s) {
    pilha_.push_back(s);
    // Se ultrapassou o limite, descarta o mais antigo.
    while (pilha_.size() > limite_) {
        pilha_.pop_front();
    }
}

const Snapshot* Historico::atual() const {
    if (pilha_.empty()) return nullptr;
    return &pilha_.back();
}

Snapshot Historico::desempilhar() {
    if (pilha_.empty()) return Snapshot{};
    Snapshot s = pilha_.back();
    pilha_.pop_back();
    return s;
}

bool Historico::vazio() const {
    return pilha_.empty();
}

std::size_t Historico::tamanho() const {
    return pilha_.size();
}

void Historico::limpar() {
    pilha_.clear();
}

int Historico::tickMaisAntigo() const {
    if (pilha_.empty()) return -1;
    return pilha_.front().tick;
}