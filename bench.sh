#!/usr/bin/env bash
# ============================================================
#  Ram Industries - Benchmark de speedup y eficiencia
#  Parcial 1 - Problema 1: Histograma Masivo
#
#  Uso:   bash bench.sh [N] "lista de hilos"
#  Ej:    bash bench.sh 200000000 "1 2 4 8 16 24 32"
#
#  Genera: docs/resultados.csv  con columnas
#          hilos,tiempo_s,speedup,eficiencia
# ============================================================
set -e

N="${1:-200000000}"
HILOS_LISTA="${2:-1 2 4 8 16 24 32}"
CSV="docs/resultados.csv"

echo ">> Compilando..."
make all >/dev/null

echo ">> Baseline secuencial (N=$N)..."
SEQ_OUT=$(./secuencial/histograma_secuencial "$N")
echo "$SEQ_OUT"
T_SEQ=$(echo "$SEQ_OUT" | grep RESULT | sed -n 's/.*tiempo=\([0-9.]*\).*/\1/p')
echo ">> Tiempo secuencial = ${T_SEQ}s"

echo "hilos,tiempo_s,speedup,eficiencia" > "$CSV"

for H in $HILOS_LISTA; do
    echo ">> Paralelo con $H hilos..."
    OUT=$(./paralelo/histograma_paralelo "$N" "$H")
    T_PAR=$(echo "$OUT" | grep RESULT | sed -n 's/.*tiempo=\([0-9.]*\).*/\1/p')
    # speedup = T_seq / T_par ; eficiencia = speedup / hilos
    SPEEDUP=$(awk "BEGIN{printf \"%.3f\", $T_SEQ/$T_PAR}")
    EFIC=$(awk "BEGIN{printf \"%.3f\", ($T_SEQ/$T_PAR)/$H}")
    echo "   tiempo=${T_PAR}s  speedup=${SPEEDUP}x  eficiencia=${EFIC}"
    echo "$H,$T_PAR,$SPEEDUP,$EFIC" >> "$CSV"
done

echo ""
echo ">> Resultados guardados en $CSV"
echo "---------------------------------------------"
column -s, -t "$CSV"
echo "---------------------------------------------"
echo "Para graficar: python3 docs/plot_resultados.py"
