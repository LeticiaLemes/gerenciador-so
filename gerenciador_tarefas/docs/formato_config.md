# Formato do Arquivo de Configuração

O simulador carrega a configuração a partir de um arquivo texto simples
(plain text). O arquivo pode ter qualquer nome e estar em qualquer caminho
do sistema de arquivos (requisito 3.3.4).

## Estrutura

- **Primeira linha não vazia:** parâmetros gerais do sistema.
- **Linhas seguintes não vazias:** uma tarefa por linha.
- Linhas em branco são ignoradas (requisito 3.3.6).
- O caractere `;` no fim da linha é opcional (requisito 3.3.3).
- Strings são tratadas de forma case-insensitive (requisito 3.3.2).
  Ex.: `RM`, `rm`, `Rm` são equivalentes.

## Primeira linha (sistema)
algoritmo_escalonamento;quantum;qtde_cpus

| Campo | Descrição | Padrão |
|-------|-----------|--------|
| `algoritmo_escalonamento` | `RM` (Rate Monotonic) ou `EDF` (Earliest Deadline First) | `RM` |
| `quantum` | Inteiro > 0. Período máximo de execução contínua. | `1` |
| `qtde_cpus` | Inteiro >= 1. Quantidade de processadores. | `1` |

Campos omitidos ou vazios assumem o valor padrão.

## Linhas de tarefa
id;cor;ingresso;duracao;periodo;prazo;lista_eventos

| Campo | Descrição | Obrigatório | Padrão |
|-------|-----------|-------------|--------|
| `id` | Inteiro >= 0, único | Sim | — |
| `cor` | 6 dígitos hexadecimais (ex.: `F0E0D0`) | Não | `FFFFFF` |
| `ingresso` | Inteiro >= 0. Instante de criação. | Sim | — |
| `duracao` | Inteiro > 0. Tempo total de execução. | Sim | — |
| `periodo` | Inteiro >= 0. 0 = aperiódica (ignorada no Projeto A). | Sim | — |
| `prazo` | Inteiro >= 0. Deadline relativo à ativação. | Sim | — |
| `lista_eventos` | Lista de eventos (tratada no Projeto B). | Não | vazio |

## Regras de validação

- **Período negativo:** erro fatal (requisito 3.3).
- **Período zero:** tarefa aperiódica → ignorada e reportada (requisito 4.4).
- **Cor inválida:** erro fatal.
- **Duração <= 0:** erro fatal.
- **Quantum <= 0:** erro fatal.
- **Qtde_cpus < 1:** erro fatal.
- **Algoritmo desconhecido:** erro fatal.

Erros são reportados com o número da linha e o motivo (requisitos 3.3.6 e 5).

## Observações

- No Projeto A, tarefas periódicas terminam após 10 execuções.
- Tarefas aperiódicas são ignoradas no Projeto A e reportadas ao usuário.
