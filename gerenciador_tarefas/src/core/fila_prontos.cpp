#include "core/fila_prontos.h"
#include <algorithm>

// Insere no fim da fila. Ignora ponteiro nulo.

void FilaProntos::inserir(TCB* tarefa) {
    if (tarefa == nullptr) return;
    fila_.push_back(tarefa);
}


// Remove e retorna a primeira. Retorna nullptr se vazia.

TCB* FilaProntos::removerPrimeira() {
    if (fila_.empty()) return nullptr;
    TCB* primeira = fila_.front();
    fila_.erase(fila_.begin());
    return primeira;
}


// Remove uma tarefa específica pelo id.

bool FilaProntos::removerPorId(int id) {
    auto it = std::find_if(fila_.begin(), fila_.end(),
        [id](const TCB* t) { return t->id == id; });
    if (it == fila_.end()) return false;
    fila_.erase(it);
    return true;
}


// Retorna a primeira sem remover.

TCB* FilaProntos::espiarPrimeira() const {
    if (fila_.empty()) return nullptr;
    return fila_.front();
}


// Vazia?

bool FilaProntos::vazia() const {
    return fila_.empty();
}


// Tamanho.

std::size_t FilaProntos::tamanho() const {
    return fila_.size();
}

// Limpa tudo.

void FilaProntos::limpar() {
    fila_.clear();
}

// Cópia dos ponteiros.

std::vector<TCB*> FilaProntos::copiar() const {
    return fila_;
}


// Contém tarefa com este id?

bool FilaProntos::contem(int id) const {
    return std::any_of(fila_.begin(), fila_.end(),
        [id](const TCB* t) { return t->id == id; });
}