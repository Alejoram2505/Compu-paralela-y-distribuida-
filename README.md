# Ram Industries — Consultoría HPC

### Examen Parcial 1 · Consultoría HPC usando OpenMP
**Problema 1 — El Histograma Masivo**

Universidad del Valle de Guatemala · Facultad de Ingeniería
Computación Paralela y Distribuida · Semestre II — 2025

**Integrante:** Diego Alejandro Ramírez Velásquez — Carné 23601

</div>

---

## El problema

Tenemos un arreglo unidimensional con **miles de millones de mediciones de temperatura** en punto flotante. Debemos clasificar cada medición en una de **100 cubetas** (rangos) y contar cuántas mediciones caen en cada una, de forma que el conteo final sea **exacto** y lo más **rápido** posible.

La dificultad al paralelizar: muchos hilos quieren incrementar las mismas 100 posiciones del histograma a la vez → **condición de carrera**. La resolvemos con una **reducción** (cada hilo cuenta en su copia privada y al final se combinan).

## Estructura del repositorio

```
.
├── secuencial/   # Algoritmo base en C (un solo hilo)
│   └── histograma_secuencial.c
├── paralelo/     # Solución optimizada con OpenMP
│   └── histograma_paralelo.c
├── docs/         # Informe, gráficas y análisis
│   ├── informe.md
│   ├── plot_resultados.py
│   └── img/
├── Makefile
└── bench.sh      # Mide speedup y eficiencia
```

## Requisitos

- **GCC con soporte OpenMP** (`gcc -fopenmp`).
  - En Windows lo más cómodo es **WSL** (Ubuntu) o **MSYS2/MinGW-w64**.
  - En Linux/Mac: `gcc` normal.
- (Opcional, para las gráficas) Python 3 con `matplotlib`.

## Cómo compilar y ejecutar

```bash
# Compilar ambas versiones
make all

# Prueba rápida (100 millones de mediciones)
make run

# Ejecutar manualmente:
./secuencial/histograma_secuencial 200000000        # N mediciones
./paralelo/histograma_paralelo    200000000 16      # N mediciones, 16 hilos
```

## Reproducir las métricas (speedup y eficiencia)

```bash
# Corre el baseline secuencial y el paralelo con varios hilos.
# Genera docs/resultados.csv
bash bench.sh 200000000 "1 2 4 8 16 24 32"

# Graficar (opcional)
python3 docs/plot_resultados.py
```

El análisis completo, la estrategia de OpenMP y los resultados están en docs/
