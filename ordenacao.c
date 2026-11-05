#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define BITS 11
#define BUCKETS (1u << BITS)   /* 2048 */
#define MASK (BUCKETS - 1u)
#define PASSES 3               /* 3 * 11 = 33 bits >= 32 bits */
#define LIMITE_FAIXA 67108864u /* 64M: acima disso o vetor de contagem seria grande demais */

/* Lê o arquivo inteiro em um buffer de texto. */
static char *ler_arquivo(const char *caminho, size_t *tamanho) {
    FILE *f = fopen(caminho, "rb");
    if (!f) { perror("Erro ao abrir arquivo"); exit(1); }
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    rewind(f);
    char *buf = malloc((size_t)n + 1);
    if (!buf) { fprintf(stderr, "Sem memoria\n"); exit(1); }
    if (fread(buf, 1, (size_t)n, f) != (size_t)n) { fprintf(stderr, "Erro de leitura\n"); exit(1); }
    buf[n] = '\0';
    fclose(f);
    *tamanho = (size_t)n;
    return buf;
}

/* Converte o texto (qualquer separador: linha, espaço, vírgula) em inteiros. */
static int32_t *parse_inteiros(const char *buf, size_t tam, size_t *total) {
    size_t cap = tam / 2 + 1;               /* no máximo 1 número a cada 2 bytes */
    int32_t *v = malloc(cap * sizeof(int32_t));
    if (!v) { fprintf(stderr, "Sem memoria\n"); exit(1); }
    size_t n = 0;
    const char *p = buf, *fim = buf + tam;
    while (p < fim) {
        while (p < fim && *p != '-' && (*p < '0' || *p > '9')) p++;
        if (p >= fim) break;
        int neg = 0;
        if (*p == '-') { neg = 1; p++; }
        if (p >= fim || *p < '0' || *p > '9') continue;
        long long x = 0;
        while (p < fim && *p >= '0' && *p <= '9') { x = x * 10 + (*p - '0'); p++; }
        v[n++] = (int32_t)(neg ? -x : x);
    }
    *total = n;
    return v;
}

/* Radix Sort LSD. Troca o bit de sinal para ordenar corretamente negativos. */
static void radixSort(int32_t *a, size_t n) {
    uint32_t *src = (uint32_t *)a;
    uint32_t *tmp = malloc(n * sizeof(uint32_t));
    if (!tmp) { fprintf(stderr, "Sem memoria\n"); exit(1); }
    uint32_t *dst = tmp;

    for (size_t i = 0; i < n; i++) src[i] ^= 0x80000000u;   /* sinal -> ordem */

    /* Histogramas das 3 passadas calculados de uma vez só. */
    static size_t cont[PASSES][BUCKETS];
    memset(cont, 0, sizeof(cont));
    for (size_t i = 0; i < n; i++) {
        uint32_t x = src[i];
        cont[0][x & MASK]++;
        cont[1][(x >> BITS) & MASK]++;
        cont[2][(x >> (2 * BITS)) & MASK]++;
    }

    for (int p = 0; p < PASSES; p++) {
        size_t soma = 0;
        for (uint32_t b = 0; b < BUCKETS; b++) {            /* prefix sum */
            size_t c = cont[p][b];
            cont[p][b] = soma;
            soma += c;
        }
        int shift = p * BITS;
        for (size_t i = 0; i < n; i++) {
            uint32_t x = src[i];
            dst[cont[p][(x >> shift) & MASK]++] = x;
        }
        uint32_t *t = src; src = dst; dst = t;              /* troca buffers */
    }
    /* 3 passadas (ímpar): resultado está em 'tmp'; copia de volta. */
    if (src != (uint32_t *)a) memcpy(a, src, n * sizeof(uint32_t));

    for (size_t i = 0; i < n; i++) ((uint32_t *)a)[i] ^= 0x80000000u;
    free(tmp);
}


/* Counting Sort: conta quantas vezes cada valor aparece e reescreve o vetor. */
static int countingSort(int32_t *a, size_t n, int32_t min, int32_t max) {
    size_t faixa = (size_t)((int64_t)max - (int64_t)min) + 1;
    uint32_t *cont = calloc(faixa, sizeof(uint32_t));
    if (!cont) return 0;                         /* sem memória: usa o plano B */

    for (size_t i = 0; i < n; i++) cont[(size_t)((int64_t)a[i] - min)]++;

    size_t pos = 0;
    for (size_t k = 0; k < faixa; k++) {
        int32_t valor = (int32_t)((int64_t)min + (int64_t)k);
        uint32_t c = cont[k];
        while (c--) a[pos++] = valor;
    }
    free(cont);
    return 1;
}

/* Escolhe a melhor estratégia conforme a faixa de valores da lista. */
static void ordenarVetor(int32_t *a, size_t n) {
    if (n < 2) return;
    int32_t min = a[0], max = a[0];
    for (size_t i = 1; i < n; i++) {
        if (a[i] < min) min = a[i];
        if (a[i] > max) max = a[i];
    }
    int64_t faixa = (int64_t)max - (int64_t)min + 1;
    if (faixa <= (int64_t)LIMITE_FAIXA && countingSort(a, n, min, max)) return;
    radixSort(a, n);
}

int main(int argc, char **argv) {
    const char *caminho = (argc > 1) ? argv[1] : "listaOdenada.txt";

    size_t tam, total;
    char *texto = ler_arquivo(caminho, &tam);
    int32_t *v = parse_inteiros(texto, tam, &total);
    free(texto);
    printf("Elementos lidos: %zu\n", total);

    /* "Impressão digital" antes de ordenar, para provar que nada se perdeu. */
    uint64_t soma_antes = 0, xor_antes = 0;
    for (size_t i = 0; i < total; i++) { soma_antes += (uint32_t)v[i]; xor_antes ^= (uint32_t)v[i] * 2654435761u; }

    /* ---------- MEDIÇÃO: somente a ordenação ---------- */
    clock_t inicio = clock();
    ordenarVetor(v, total);
    clock_t fim = clock();
    /* -------------------------------------------------- */

    double tempo = ((double)(fim - inicio)) / CLOCKS_PER_SEC;
    printf("Tempo de ordenação: %f segundos (%f ms)\n", tempo, tempo * 1000.0);

    /* Verificação (fora do cronômetro). */
    size_t desordem = 0, repetidos = 0;
    uint64_t soma_depois = 0, xor_depois = 0;
    for (size_t i = 0; i < total; i++) {
        soma_depois += (uint32_t)v[i];
        xor_depois ^= (uint32_t)v[i] * 2654435761u;
        if (i > 0) {
            if (v[i - 1] > v[i]) desordem++;
            else if (v[i - 1] == v[i]) repetidos++;
        }
    }
    printf("Ordem crescente: %s\n", desordem == 0 ? "OK" : "FALHOU");
    printf("Nenhum elemento perdido: %s\n",
           (soma_antes == soma_depois && xor_antes == xor_depois) ? "OK" : "FALHOU");
    printf("Valores repetidos (vizinhos iguais): %zu\n", repetidos);

    free(v);
    return desordem == 0 ? 0 : 1;
}
