# Competição de Ordenação — O Desafio dos Milissegundos

Ordenação de 10.000.000 de inteiros em C, medindo apenas o tempo de CPU da ordenação.

## Equipe
1. [Jaíne Marques Tavares] — Líder
2. [Jônatas Eduardo de Souza Fagundes]
3. [Miguel do Carmo Silva]
4. [Maíra Sousa Berguerand]
5. [Matheus de Souza França]

## Análise dos dados
O arquivo possui 10.000.000 de números inteiros, todos entre **1 e 2.500.000**, com muitos valores repetidos
(faixa pequena em relação à quantidade de elementos).

## Algoritmo escolhido
**Counting Sort (ordenação por contagem), O(N + K)**, onde N = 10.000.000 e K = 2.500.000 (faixa de valores).
Como não faz comparações entre elementos, supera algoritmos O(N log N) (Quick Sort, Merge Sort) nesse cenário.

- Passo 1: percorre o vetor para achar o mínimo e o máximo.
- Passo 2: conta quantas vezes cada valor aparece.
- Passo 3: reescreve o vetor em ordem crescente a partir das contagens.

**Plano B:** se a faixa de valores for maior que 64 milhões (ou faltar memória), o programa usa
**Radix Sort LSD** (3 passadas de 11 bits, O(N)), que funciona para qualquer inteiro de 32 bits, inclusive negativos.

## Como compilar
```bash
gcc -O2 -o ordenacao ordenacao.c
```
(Windows/MinGW: `gcc -O2 -o ordenacao.exe ordenacao.c`)

## Como executar
Coloque o arquivo `listaOdenada.txt` na mesma pasta (ele não vai para o GitHub por ser grande) e rode:
```bash
./ordenacao listaOdenada.txt
```

## O que o programa imprime
- Quantidade de elementos lidos
- Tempo de ordenação, medido com `clock()` **somente** na função de ordenação (leitura do arquivo e conversão ficam fora)
- Verificação de ordem crescente e de que nenhum elemento foi perdido (soma + hash antes/depois)
- Quantidade de valores repetidos (a lista tem repetidos, então a ordem é crescente não-estrita)

## Resultado
Tempo medido em testes: cerca de 35 ms para 10.000.000 de elementos.

![execucao](<img width="844" height="370" alt="print terminal" src="https://github.com/user-attachments/assets/55eeb3f3-6d65-4be0-94d7-061159138614" />
)
