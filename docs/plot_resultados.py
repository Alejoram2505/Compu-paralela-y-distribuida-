#!/usr/bin/env python3
# ============================================================
#  Ram Industries - Graficas de speedup y eficiencia
#  Lee docs/resultados.csv y produce docs/img/speedup.png
#                                     docs/img/eficiencia.png
#  Uso:  python3 docs/plot_resultados.py
#  Requiere: pip install matplotlib
# ============================================================
import csv, os, sys

try:
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
except ImportError:
    sys.exit("Instala matplotlib:  pip install matplotlib")

AQUI = os.path.dirname(os.path.abspath(__file__))
CSV = os.path.join(AQUI, "resultados.csv")
IMG = os.path.join(AQUI, "img")
os.makedirs(IMG, exist_ok=True)

hilos, speedup, efic = [], [], []
with open(CSV) as f:
    for row in csv.DictReader(f):
        hilos.append(int(row["hilos"]))
        speedup.append(float(row["speedup"]))
        efic.append(float(row["eficiencia"]))

# ---- Grafica 1: Speedup ----
plt.figure(figsize=(7, 5))
plt.plot(hilos, speedup, "o-", label="Speedup medido")
plt.plot(hilos, hilos, "--", color="gray", label="Speedup ideal (lineal)")
plt.xlabel("Numero de hilos")
plt.ylabel("Speedup  (T_seq / T_par)")
plt.title("Speedup - Histograma Masivo (OpenMP)\nRam Industries")
plt.legend(); plt.grid(True, alpha=0.3)
plt.tight_layout()
plt.savefig(os.path.join(IMG, "speedup.png"), dpi=130)

# ---- Grafica 2: Eficiencia ----
plt.figure(figsize=(7, 5))
plt.plot(hilos, [e * 100 for e in efic], "s-", color="green")
plt.axhline(100, ls="--", color="gray", label="Eficiencia ideal (100%)")
plt.xlabel("Numero de hilos")
plt.ylabel("Eficiencia (%)")
plt.title("Eficiencia - Histograma Masivo (OpenMP)\nRam Industries")
plt.ylim(0, 110); plt.legend(); plt.grid(True, alpha=0.3)
plt.tight_layout()
plt.savefig(os.path.join(IMG, "eficiencia.png"), dpi=130)

print("Graficas guardadas en docs/img/speedup.png y docs/img/eficiencia.png")
