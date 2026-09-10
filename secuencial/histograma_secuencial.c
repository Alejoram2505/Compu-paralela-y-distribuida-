/*
 * ============================================================================
 *  Ram Industries  |  Consultoria HPC
 *  Examen Parcial 1 - Problema 1: El Histograma Masivo.
 *  VERSION SECUENCIAL (algoritmo base, un solo hilo)
 * ============================================================================
 *
 *  Problema:
 *    Tenemos un arreglo unidimensional con miles de millones de mediciones de
 *    temperatura en punto flotante. Debemos clasificar cada medicion en una de
 *    100 cubetas (rangos) y contar cuantas mediciones caen en cada una.
 *
 *  Esta version es la referencia (baseline) contra la cual mediremos el
 *  speedup de la version paralela con OpenMP.
 *
 *  Compilar:   gcc -O2 -o histograma_secuencial histograma_secuencial.c
 *  Ejecutar:   ./histograma_secuencial [N]
 *              N = cantidad de mediciones (por defecto 200,000,000)
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#define NUM_CUBETAS 100        /* 100 rangos, como pide el enunciado */
#define TEMP_MIN   (-50.0)     /* temperatura minima esperada (C)     */
#define TEMP_MAX    ( 50.0)    /* temperatura maxima esperada (C)     */

/* Generador pseudoaleatorio rapido (xorshift64). Semilla fija para que
 * la version secuencial y la paralela trabajen exactamente sobre los
 * mismos datos y el conteo sea comparable. */
static inline uint64_t xorshift64(uint64_t *estado) {
    uint64_t x = *estado;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    *estado = x;
    return x;
}

/* Llena 'datos' con N temperaturas en el rango [TEMP_MIN, TEMP_MAX). */
static void generar_datos(float *datos, long long n) {
    uint64_t estado = 88172645463325252ULL;
    double rango = TEMP_MAX - TEMP_MIN;
    for (long long i = 0; i < n; i++) {
        /* u en [0,1) usando los 53 bits altos */
        double u = (xorshift64(&estado) >> 11) * (1.0 / 9007199254740992.0);
        datos[i] = (float)(TEMP_MIN + u * rango);
    }
}

int main(int argc, char **argv) {
    long long N = (argc > 1) ? atoll(argv[1]) : 200000000LL; /* 200M por defecto */
    double ancho_cubeta = (TEMP_MAX - TEMP_MIN) / NUM_CUBETAS;

    printf("======================================================\n");
    printf(" Histograma SECUENCIAL (baseline)\n");
    printf(" N = %lld mediciones | %d cubetas | rango [%.1f, %.1f)\n",
           N, NUM_CUBETAS, TEMP_MIN, TEMP_MAX);
    printf(" Memoria del arreglo: %.2f GB\n", (double)N * sizeof(float) / 1e9);
    printf("======================================================\n");

    float *datos = (float *)malloc((size_t)N * sizeof(float));
    if (!datos) {
        fprintf(stderr, "ERROR: no hay memoria para %lld floats.\n", N);
        return 1;
    }

    generar_datos(datos, N);

    long long hist[NUM_CUBETAS] = {0};

    /* ---- Nucleo del algoritmo: se mide SOLO esta parte ---- */
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    for (long long i = 0; i < N; i++) {
        int idx = (int)((datos[i] - TEMP_MIN) / ancho_cubeta);
        if (idx < 0) idx = 0;
        else if (idx >= NUM_CUBETAS) idx = NUM_CUBETAS - 1;
        hist[idx]++;
    }

    clock_gettime(CLOCK_MONOTONIC, &t1);
    double segundos = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) / 1e9;
    /* ------------------------------------------------------- */

    /* Verificacion de correctitud: la suma de todas las cubetas
     * debe ser exactamente N (ninguna medicion se pierde ni se cuenta doble). */
    long long suma = 0;
    for (int b = 0; b < NUM_CUBETAS; b++) suma += hist[b];

    printf("Tiempo de conteo : %.4f s\n", segundos);
    printf("Verificacion     : suma de cubetas = %lld (esperado %lld) -> %s\n",
           suma, N, (suma == N) ? "OK" : "ERROR");
    printf("Muestras         : cubeta[0]=%lld  cubeta[50]=%lld  cubeta[99]=%lld\n",
           hist[0], hist[50], hist[99]);

    /* Linea parseable por el script de benchmark (no borrar). */
    printf("RESULT modo=secuencial threads=1 n=%lld tiempo=%.6f\n", N, segundos);

    free(datos);
    return 0;
}
