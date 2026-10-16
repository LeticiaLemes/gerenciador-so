#pragma once
#include "core/estado_simulacao.h"
#include "core/comando.h"
#include <string>


// Geração dos controles HTML:
//  - Botões: Avançar, Retroceder, Executar completo, Passo-a-passo, Reset.
//  - Campo para ir a um tick específico.
//  - Formulário para modificar uma tarefa.
//
// Os controles usam JavaScript para enviar comandos ao servidor HTTP
// via fetch('/comando', { method: 'POST', body: JSON }).

class ControlesHtml {
    public:
        ControlesHTML() = default;

        // Gera o bloco do HTML dos controles
        std::string gerar(const EstadoSimulacao& estado) const;

    private:
        // Gera o bloco de botões principais
        std::string geraBotoes() const;

        // Gera o campo "ir para tick"
        std::string gerarIrTick(const EstadoSimulacao& estado) const;

        // Gera formulário de geração de tarefa
        std::string geraFormMod(const EstadoSimulacao& estado) const;

        // Gera o bloco JavaScript que envia comandos
        std::string geraJava() const;
};