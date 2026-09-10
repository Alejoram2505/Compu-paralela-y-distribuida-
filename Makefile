# ============================================================
#  Ram Industries - Consultoria HPC.
#  Parcial 1 - Problema 1: Histograma Masivo
# ============================================================
CC      = gcc
CFLAGS  = -O2 -Wall
OMPFLAG = -fopenmp

.PHONY: all clean run bench

all: secuencial/histograma_secuencial paralelo/histograma_paralelo

secuencial/histograma_secuencial: secuencial/histograma_secuencial.c
	$(CC) $(CFLAGS) -o $@ $<

paralelo/histograma_paralelo: paralelo/histograma_paralelo.c
	$(CC) $(CFLAGS) $(OMPFLAG) -o $@ $<

# Corrida rapida de prueba (100 millones)
run: all
	./secuencial/histograma_secuencial 100000000
	./paralelo/histograma_paralelo 100000000

# Benchmark completo (genera docs/resultados.csv)
bench: all
	bash bench.sh

clean:
	rm -f secuencial/histograma_secuencial paralelo/histograma_paralelo
