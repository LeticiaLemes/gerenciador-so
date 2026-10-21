#pragma once
#include <string>


// Tipos de comando que o usuário pode enviar pela interface HTML.
// O JavaScript envia via POST /comando; o servidor HTTP converte para
// esta struct e o núcleo aplica.

enum class TipoComando {
    AVANCAR,    // avança 1 tick
    RETROCEDER,     // retrocede 1 tick
    AVANCAR_N,  // avança N ticks
    RETROCEDER_N,   // retrocede N ticks
    EXECUTAR_COMPLETO,  // executa até acabar sem intervenção
    IR_TICK,    // vai para o tick N
    MOD_TAREFA,     // Modificacampo da tarefa
    RESET,  //reinicia simulação
    DESCONHECIDO
};

// Comando completo, com parâmetros
struct Comando {
    TipoComando tipo = TipoComando::DESCONHECIDO;
    int valor = 0;      // usado por AVANCAR_N, RETROCEDER_N, IR_TICK
    int idTarefa = -1;      // usado por MOD_TAREFA
    std::string campo;      // campo a modificar
    std::string valorCampo;      // novo valor
};

// Resultado aplicação comando
struct  ResultadoComando {
    bool sucesso = false;
    std::string msg;     // mensagem de erro ou confirmação
};