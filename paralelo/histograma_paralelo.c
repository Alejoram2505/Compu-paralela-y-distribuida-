/*
 * ============================================================================
 *  Ram Industries  |  Consultoria HPC
 *  Examen Parcial 1 - Problema 1: El Histograma Masivo
 *  VERSION PARALELA con OpenMP.
 * ============================================================================
 *
 *  Estrategia de paralelizacion:
 *  ------------------------------
 *  El conteo es un problema de "reduccion": muchos hilos quieren INCREMENTAR
 *  las mismas 100 posiciones del arreglo 'hist'. Si dos hilos hacen hist[idx]++
 *  al mismo tiempo se produce una CONDICION DE CARRERA (race condition) y el
 *  conteo sale mal.
 *
 *  Comparamos tres formas de evitarlo (ver docs/informe.md):
 *    (A) #pragma omp atomic  sobre hist[idx]++        -> correcto pero LENTO
 *                                                         (serializa cada ++).
 *    (B) Histogramas privados por hilo + combinacion   -> rapido, clasico.
 *    (C) reduction(+:hist[:NUM_CUBETAS])               -> OpenMP 4.5+ hace (B)
 *                                                         por nosotros. LIMPIO.
 *
 *  Usamos (C) como solucion principal: cada hilo acumula en su copia privada
 *  del histograma (que cabe en cache, solo 100 enteros) y al final OpenMP
 *  suma todas las copias. Cero contencion en el bucle caliente.
 *
 *  Compilar:   gcc -O2 -fopenmp -o histograma_paralelo histograma_paralelo.c
 *  Ejecutar:   ./histograma_paralelo [N] [num_hilos]
 *              N         = cantidad de mediciones (por defecto 200,000,000)
 *              num_hilos = hilos OpenMP (por defecto: todos los del sistema)
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <omp.h>

#define NUM_CUBETAS 100
#define TEMP_MIN   (-50.0)
#define TEMP_MAX    ( 50.0)

static inline uint64_t xorshift64(uint64_t *estado) {
    uint64_t x = *estado;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    *estado = x;
    return x;
}

/* Mismos datos que la version secuencial (misma semilla base) para que
 * la comparacion de tiempos sea justa. La generacion se reparte entre
 * hilos dandole a cada bloque su propia semilla derivada. */
static void generar_datos(float *datos, long long n) {
    double rango = TEMP_MAX - TEMP_MIN;
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        uint64_t estado = 88172645463325252ULL + (uint64_t)(tid + 1) * 0x9E3779B97F4A7C15ULL;
        #pragma omp for schedule(static)
        for (long long i = 0; i < n; i++) {
            double u = (xorshift64(&estado) >> 11) * (1.0 / 9007199254740992.0);
            datos[i] = (float)(TEMP_MIN + u * rango);
        }
    }
}

int main(int argc, char **argv) {
    long long N = (argc > 1) ? atoll(argv[1]) : 200000000LL;
    if (argc > 2) omp_set_num_threads(atoi(argv[2]));

    double ancho_cubeta = (TEMP_MAX - TEMP_MIN) / NUM_CUBETAS;
    int hilos = 0;
    #pragma omp parallel
    {
        #pragma omp single
        hilos = omp_get_num_threads();
    }

    printf("======================================================\n");
    printf(" Histograma PARALELO (OpenMP)\n");
    printf(" N = %lld mediciones | %d cubetas | hilos = %d\n", N, NUM_CUBETAS, hilos);
    printf(" Memoria del arreglo: %.2f GB\n", (double)N * sizeof(float) / 1e9);
    printf("======================================================\n");

    float *datos = (float *)malloc((size_t)N * sizeof(float));
    if (!datos) {
        fprintf(stderr, "ERROR: no hay memoria para %lld floats.\n", N);
        return 1;
    }

    generar_datos(datos, N);

    long long hist[NUM_CUBETAS] = {0};

    /* ---- Nucleo paralelo: se mide SOLO esta parte ---- */
    double t0 = omp_get_wtime();

    /* reduction sobre el arreglo completo: cada hilo tiene su histograma
     * privado y OpenMP los combina al terminar. schedule(static) porque el
     * trabajo por iteracion es uniforme (no hay desbalance de carga). */
    #pragma omp parallel for schedule(static) reduction(+:hist[:NUM_CUBETAS])
    for (long long i = 0; i < N; i++) {
        int idx = (int)((datos[i] - TEMP_MIN) / ancho_cubeta);
        if (idx < 0) idx = 0;
        else if (idx >= NUM_CUBETAS) idx = NUM_CUBETAS - 1;
        hist[idx]++;
    }

    double t1 = omp_get_wtime();
    double segundos = t1 - t0;
    /* -------------------------------------------------- */

    long long suma = 0;
    for (int b = 0; b < NUM_CUBETAS; b++) suma += hist[b];

    printf("Tiempo de conteo : %.4f s\n", segundos);
    printf("Verificacion     : suma de cubetas = %lld (esperado %lld) -> %s\n",
           suma, N, (suma == N) ? "OK" : "ERROR");
    printf("Muestras         : cubeta[0]=%lld  cubeta[50]=%lld  cubeta[99]=%lld\n",
           hist[0], hist[50], hist[99]);

    /* Linea parseable por el script de benchmark (no borrar). */
    printf("RESULT modo=paralelo threads=%d n=%lld tiempo=%.6f\n", hilos, N, segundos);

    free(datos);
    return 0;
}
