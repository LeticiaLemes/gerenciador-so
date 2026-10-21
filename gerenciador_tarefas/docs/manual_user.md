# Manual do Usuário — Gerenciador de Tarefas (Simulador RM/EDF)

## 1. Visão geral

O **Gerenciador de Tarefas** é um simulador de escalonamento de tarefas de tempo real em um sistema operacional multitarefa. Ele:

1. lê um **arquivo de configuração** em texto simples, com os parâmetros do sistema e a lista de tarefas;
2. executa a simulação até o fim, usando o algoritmo **RM** (*Rate Monotonic*) ou **EDF** (*Earliest Deadline First*), com uma ou mais CPUs;
3. imprime um **resumo no terminal**;
4. grava um **relatório HTML** com o diagrama de Gantt, o status das CPUs, a legenda e a tabela de inspeção das tarefas.

O programa tem **dois modos de uso**:

| Modo | Como ativar | Para que serve |
|------|-------------|----------------|
| **Estático** (padrão) | `bin/gerenciador config.txt` | Roda tudo de uma vez e grava o HTML final. Os botões da página **não funcionam**. |
| **Interativo** | `bin/gerenciador config.txt --servidor` | Inicia um servidor local e abre a simulação no tick 0; os botões da página avançam, retrocedem, reiniciam e modificam tarefas. |

---

## 2. Requisitos

| Item | Necessário |
|------|-----------|
| Compilador | `g++` com suporte a C++17 |
| Ferramenta de build | `make` |
| Bibliotecas | Nenhuma de terceiros (usa `-pthread` da biblioteca padrão) |
| Navegador web | Qualquer navegador atual, para visualizar o relatório HTML |

> O programa foi compilado e testado em Linux (Ubuntu, g++). O código possui tratamento para Windows e macOS na abertura automática do navegador, mas esses sistemas não foram testados.

---

## 3. Instalação (compilação)

Abra um terminal **na pasta raiz do projeto** (a pasta `gerenciador_tarefas`, que contém o `Makefile`).

### 3.1. Se você recebeu o projeto compactado

```bash
tar -xzf gerenciador_tarefas_corrigido_tar.gz
cd gerenciador_tarefas
```

### 3.2. Compilar

```bash
make
```

Ao final, o executável é criado em `bin/gerenciador`.

### 3.3. Outros comandos do Makefile

| Comando | O que faz |
|---------|-----------|
| `make` | Compila o programa (`bin/gerenciador`). |
| `make run` | Compila (se necessário) e executa com `config/exemplo.txt`. |
| `make run CFG=meu_arquivo.txt` | Compila (se necessário) e executa com o arquivo de configuração indicado. |
| `make test` | Compila e executa todos os testes automatizados. |
| `make clean` | Remove os arquivos compilados, os binários de teste e os `.html` de `bin/`. |

---

## 4. Como executar

### 4.1. Sintaxe

```bash
# Modo estático
bin/gerenciador <arquivo_de_configuracao> [saida.html] [--abrir]

# Modo interativo
bin/gerenciador <arquivo_de_configuracao> --servidor [--porta N] [--abrir]
```

| Argumento | Obrigatório | Descrição |
|-----------|-------------|-----------|
| `<arquivo_de_configuracao>` | **Sim** | Caminho do arquivo `.txt` com a configuração (ver seção 5). Pode ter qualquer nome e estar em qualquer pasta. |
| `[saida.html]` | Não | Caminho do relatório HTML a ser gerado. Padrão: `bin/gantt.html`. |
| `--abrir` | Não | Modo estático: abre o relatório gerado no navegador. Modo interativo: abre a página do servidor. |
| `--servidor` | Não | Ativa o **modo interativo** (ver seção 6.3). `[saida.html]` é ignorado neste modo. |
| `--porta N` | Não | Porta do servidor, de 1 a 65535. Padrão: `8080`. Só tem efeito com `--servidor`. |

Observações:

- O primeiro argumento que não for `--abrir` é o arquivo de configuração; o segundo é o HTML de saída. A opção `--abrir` pode aparecer em qualquer posição.
- O programa **não cria pastas**. A pasta de destino do HTML precisa existir (a pasta `bin/` já existe no projeto).
- O caminho padrão `bin/gantt.html` é relativo à pasta em que você executa o comando. Execute a partir da raiz do projeto.

### 4.2. Exemplos

Executar com o arquivo de exemplo que acompanha o projeto:

```bash
bin/gerenciador config/exemplo.txt
```

Escolher o nome do relatório:

```bash
bin/gerenciador config/exemplo.txt resultados/simulacao1.html
```

Gerar o relatório e abrir no navegador:

```bash
bin/gerenciador config/exemplo.txt --abrir
```

Modo interativo (abra `http://localhost:8080` no navegador, ou use `--abrir`):

```bash
bin/gerenciador config/exemplo.txt --servidor --abrir
bin/gerenciador config/exemplo.txt --servidor --porta 9000
```

Usando o Makefile (modo estático):

```bash
make run CFG=config/exemplo.txt
```

### 4.3. O que aparece no terminal

Exemplo de saída para `config/exemplo.txt`:

```text
Algoritmo: RM | quantum: 1 | CPUs: 1
Simulação concluída no tick 185.
CPU 0: desligada 115 ticks
Deadlines perdidos: 0
Gantt gravado em: bin/gantt.html
```

| Linha | Significado |
|-------|-------------|
| `Algoritmo / quantum / CPUs` | Parâmetros do sistema usados na simulação. |
| `Simulação concluída no tick N` | A simulação terminou no instante (tick) N. |
| `CPU i: desligada X ticks` | Tempo em que a CPU *i* ficou ociosa/desligada. Há uma linha por CPU. |
| `Deadlines perdidos` | Quantidade de tarefas/ativações que perderam o prazo. |
| `Gantt gravado em` | Caminho do relatório HTML gerado. |

Avisos e erros são escritos na saída de erro (`stderr`) com os prefixos `aviso:` e `erro:`.

### 4.4. Códigos de saída

| Código | Significado |
|--------|-------------|
| `0` | Simulação executada e relatório gravado com sucesso. |
| `1` | Erro no arquivo de configuração, arquivo não encontrado, ou falha ao gravar o HTML. |
| `2` | Nenhum arquivo de configuração foi informado (o programa exibe a sintaxe de uso), ou `--porta` recebeu um valor inválido. |

No modo interativo, o programa só termina quando você pressiona **Ctrl+C** (código `0`). Se a porta estiver ocupada ou o servidor não subir, o código é `1`.

---

## 5. Arquivo de configuração

O arquivo é texto simples (plain text) e pode ter qualquer nome.

### 5.1. Estrutura

- **Primeira linha não vazia:** parâmetros do sistema.
- **Demais linhas não vazias:** uma tarefa por linha.
- Linhas em branco são ignoradas.
- O `;` no final da linha é opcional.
- Texto não diferencia maiúsculas de minúsculas (`RM`, `rm` e `Rm` são equivalentes).
- Os campos são separados por ponto e vírgula (`;`).

### 5.2. Primeira linha — parâmetros do sistema

```text
algoritmo_escalonamento;quantum;qtde_cpus
```

| Campo | Valores aceitos | Padrão se omitido ou vazio |
|-------|-----------------|----------------------------|
| `algoritmo_escalonamento` | `RM` ou `EDF` | `RM` |
| `quantum` | Inteiro maior que 0 (tempo máximo de execução contínua) | `1` |
| `qtde_cpus` | Inteiro maior ou igual a 1 | `1` |

### 5.3. Demais linhas — tarefas

```text
id;cor;ingresso;duracao;periodo;prazo;lista_eventos
```

| Campo | Descrição | Obrigatório | Regra / padrão |
|-------|-----------|-------------|----------------|
| `id` | Identificador da tarefa | Sim | Inteiro ≥ 0 e **único** |
| `cor` | Cor da tarefa no gráfico | Não | 6 dígitos hexadecimais (ex.: `FF0000`). Se vazio: `FFFFFF` |
| `ingresso` | Instante em que a tarefa é criada | Sim | Inteiro ≥ 0 |
| `duracao` | Tempo total de execução | Sim | Inteiro > 0 |
| `periodo` | Intervalo entre ativações | Sim | Inteiro ≥ 0. `0` = tarefa aperiódica (ignorada) |
| `prazo` | Deadline relativo à ativação | Sim | Inteiro ≥ 0 |
| `lista_eventos` | Lista de eventos da tarefa | Não | Lida pelo programa, mas **ainda não processada** (ver seção 8) |

Cada tarefa precisa ter **no mínimo 6 campos** (até `prazo`).

### 5.4. Exemplo comentado

Arquivo `config/exemplo.txt`:

```text
RM;1;1
1;FF0000;2;2;5;5
2;0000FF;0;5;20;20
```

- Linha 1: algoritmo **RM**, quantum **1**, **1** CPU.
- Linha 2: tarefa **1**, cor vermelha (`FF0000`), ingresso no tick 2, duração 2, período 5, prazo 5.
- Linha 3: tarefa **2**, cor azul (`0000FF`), ingresso no tick 0, duração 5, período 20, prazo 20.

Outro exemplo, com EDF, 2 CPUs, quantum 2 e uma tarefa aperiódica:

```text
EDF;2;2
1;00FF00;0;3;6;6
2;0000FF;1;2;4;4
3;FF0000;0;1;0;5
```

Neste caso a tarefa 3 tem período 0: o programa emite um aviso e a ignora, mas a simulação prossegue com as tarefas 1 e 2.

---

## 6. Relatório HTML

Abra o arquivo gerado (por padrão `bin/gantt.html`) em qualquer navegador, ou use a opção `--abrir`. O relatório contém, de cima para baixo:

1. **Cabeçalho:** algoritmo, quantum, número de CPUs e tick atual.
2. **Controles:** botões de navegação, campo "Ir para tick" e formulário "Modificar tarefa" (funcionam apenas no modo interativo; ver seção 6.3).
3. **Status das CPUs:** uma barra por CPU, em azul para o tempo ligada e em vermelho para o tempo desligada, com os totais em ticks.
4. **Diagrama de Gantt:** linha do tempo da execução das tarefas.
5. **Legenda.**
6. **Inspeção de Tarefas:** tabela com o estado de cada tarefa.
7. **Botão "Imprimir / Salvar como PDF":** abre a janela de impressão do navegador, onde é possível escolher "Salvar como PDF".

### 6.1. Legenda do gráfico

| Símbolo | Significado |
|---------|-------------|
| Retângulo azul | Tarefa **executando** |
| Retângulo sem preenchimento (borda cinza) | Tarefa **pronta**, aguardando CPU |
| Retângulo pontilhado | Tarefa **suspensa** |
| Triângulo verde apontando para baixo | **Chegada** (ativação) da tarefa |
| Triângulo vermelho apontando para cima | **Término** da execução |
| "X" vermelho | **Perda de deadline** |
| Círculo amarelo | **Sorteio** usado como critério de desempate |

### 6.2. Tabela de inspeção

Cada linha mostra, para o tick exibido: ID, cor, ingresso, duração, período, prazo, **estado** (Pronta, Executando, Suspensa, Terminada, Ignorada ou Inativa), tempo executado, número de ativações, deadline atual, CPU em uso (`-` se nenhuma), se perdeu o deadline (Sim/Não) e os eventos da tarefa.

### 6.3. Controles interativos (modo `--servidor`)

Inicie o programa com `--servidor`. Ele escreve no terminal o endereço (por padrão `http://localhost:8080`); abra-o no navegador ou use `--abrir`. A simulação **começa no tick 0**, sem ter executado nada, e você a controla pelos botões. Para encerrar, pressione **Ctrl+C** no terminal.

> Os botões só funcionam na página servida por `--servidor`. Se você abrir um `.html` gravado no modo estático, eles exibirão "Erro de comunicação", pois não há servidor por trás.

#### Botões de navegação

| Botão | Efeito |
|-------|--------|
| ◀ Retroceder | Volta 1 tick. |
| Avançar ▶ | Avança 1 tick. |
| ◀◀ Retroceder 10 | Volta 10 ticks (se houver menos, vai até o tick 0). |
| Avançar 10 ▶▶ | Avança 10 ticks (para antes se a simulação terminar). |
| ⏩ Executar completo | Executa até o fim da simulação. |
| ⟲ Reset | Reinicia no tick 0 com a configuração **original do arquivo**, descartando qualquer modificação de tarefa. |
| **Ir para tick** + Ir | Vai para o tick digitado (avançando ou retrocedendo). Se o fim da simulação vier antes, para nele. |

Quando uma ação não pode ser feita (por exemplo, avançar depois do fim, ou retroceder no tick 0), a página mostra um alerta com o motivo e nada muda.

#### Modificar tarefa

Escolha a **tarefa**, o **campo** e digite o **novo valor**; clique em **Aplicar**. A modificação vale a partir do tick atual.

| Campo | Valor aceito | Observação |
|-------|--------------|------------|
| duração | Inteiro > 0 | Se a tarefa já executou mais que o novo valor, ela termina no próximo tick. |
| período | Inteiro > 0 | Recusado se a próxima ativação passaria a cair em um instante já passado. |
| prazo | Inteiro >= 0 | `0` = igual ao período. Se a tarefa está com uma ativação em andamento, o deadline atual é recalculado. |
| ingresso | Inteiro >= 0 | Recusado se a próxima ativação passaria a cair em um instante já passado. |
| cor | 6 dígitos hexadecimais | Ex.: `00FF00`. |
| estado | `suspensa`, `pronta` ou `terminada` | `suspensa`: só para tarefa pronta/executando; libera a CPU. `pronta`: só para tarefa suspensa; devolve à fila. `terminada`: encerra a tarefa; se todas terminarem, a simulação acaba. |

Observações:

- Tarefas ignoradas (aperiódicas) não podem ser modificadas.
- Se algum valor for recusado, a página exibe um alerta com o motivo e a tarefa permanece como estava.
- Retroceder **além** do tick em que a modificação foi feita também desfaz a modificação, pois o estado daquele instante é restaurado.
- O servidor aceita conexões apenas da própria máquina (`127.0.0.1`).

---

## 7. Mensagens de erro e avisos

Qualquer **erro** impede a simulação (o programa termina com código `1` e não gera o HTML). Um **aviso** não interrompe a execução.

Cada mensagem informa o número da linha do arquivo e o motivo.

### 7.1. Erros

| Mensagem (resumo) | Causa | Como corrigir |
|-------------------|-------|---------------|
| `não foi possível abrir o arquivo '...'` | Caminho incorreto ou sem permissão de leitura | Verifique o caminho e as permissões. |
| `arquivo vazio ou sem linha de parâmetros de sistema` | Arquivo sem conteúdo | Inclua a primeira linha de parâmetros. |
| `algoritmo desconhecido ('X'). Use 'RM' ou 'EDF'` | Algoritmo inválido | Use `RM` ou `EDF`. |
| `quantum deve ser > 0` / `quantum inválido` | Quantum zero, negativo ou não numérico | Use um inteiro positivo. |
| `qtde_cpus deve ser >= 1` / `quantidade de CPUs inválida` | Número de CPUs inválido | Use um inteiro ≥ 1. |
| `Esperados ao menos 6 campos` | Linha de tarefa incompleta | Informe `id;cor;ingresso;duracao;periodo;prazo`. |
| `id inválido` / `id deve ser >= 0` | Id não numérico ou negativo | Use um inteiro ≥ 0. |
| `id N repetido` | Dois ids iguais | Use ids únicos. |
| `cor inválida (...)` | Cor fora do formato `RRGGBB` | Use 6 dígitos hexadecimais (0-9, A-F). |
| `ingresso inválido` / `ingresso deve ser >= 0` | Ingresso inválido | Use um inteiro ≥ 0. |
| `duração inválida` / `duração deve ser > 0` | Duração zero, negativa ou não numérica | Use um inteiro positivo. |
| `período inválido` / `período negativo não é permitido` | Período não numérico ou negativo | Use um inteiro ≥ 0. |
| `prazo inválido` / `prazo deve ser >= 0` | Prazo inválido | Use um inteiro ≥ 0. |
| `não foi possível gravar '...'` | Pasta de destino do HTML inexistente ou sem permissão | Crie a pasta ou escolha outro caminho. |

### 7.2. Avisos

| Mensagem | Significado |
|----------|-------------|
| `tarefa id=N é aperiódica (período=0) e será IGNORADA` | A tarefa não será simulada; as demais continuam. |
| `nenhuma tarefa periódica encontrada` | Não há tarefas executáveis; a simulação não terá o que executar. |

---

## 8. Limitações da versão atual

- **Tarefas aperiódicas** (período `0`) são ignoradas e apenas reportadas.
- **Tarefas periódicas** terminam após **10 execuções** (conforme `docs/formato_config.md`).
- O campo **`lista_eventos`** é aceito no arquivo, mas os eventos ainda não são interpretados pela simulação.
- No **modo estático** a simulação é sempre executada por completo; para controlar tick a tick, use `--servidor`.

---

## 9. Solução de problemas

| Sintoma | Possível causa e solução |
|---------|--------------------------|
| `make: g++: command not found` | Instale um compilador C++ (ex.: `sudo apt install build-essential` em Debian/Ubuntu). |
| `Uso: bin/gerenciador <arquivo_de_configuracao> ...` | Faltou informar o arquivo de configuração. |
| `bin/gerenciador: No such file or directory` | O programa ainda não foi compilado (rode `make`) ou o comando não foi executado na raiz do projeto. |
| `não foi possível gravar '...'` | A pasta de destino não existe; crie-a antes de executar. |
| `--abrir` não abre o navegador | Em Linux é necessário o `xdg-open` e um ambiente gráfico. Abra o HTML manualmente. |
| Botões do HTML retornam "Erro de comunicação" | A página não foi aberta pelo servidor. Rode com `--servidor` e acesse o endereço mostrado no terminal (ver seção 6.3). |
| `não foi possível iniciar o servidor na porta N` | A porta já está em uso. Escolha outra com `--porta`. |
| A página demora cerca de 1 s para carregar | Normal quando o navegador abre uma conexão extra ociosa; o servidor a descarta após 1 s. |

---

## 10. Estrutura do projeto

```text
gerenciador_tarefas/
├── Makefile              # compilação, testes e execução
├── bin/                  # executável e relatórios HTML gerados
├── config/exemplo.txt    # arquivo de configuração de exemplo
├── docs/                 # documentação (formato_config.md, este manual)
├── src/                  # código-fonte (config, core, escalonadores, gui, utils)
└── tests/                # testes automatizados (make test)
```