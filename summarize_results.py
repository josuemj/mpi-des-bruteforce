#!/usr/bin/env python3
# summarize_results.py
# Lee experiments/results_raw.csv y genera experiments/results_summary.csv + dos PNGs.
import pandas as pd
import sys
import os
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

IN = "experiments/results_raw.csv"
OUT = "experiments/results_summary.csv"
PLOT_DIR = "experiments/plots"
os.makedirs(PLOT_DIR, exist_ok=True)

if not os.path.exists(IN):
    print(f"Error: no existe {IN}")
    sys.exit(1)

df = pd.read_csv(IN)

# Normalizar columnas si vienen como strings vacíos
df['processes'] = pd.to_numeric(df['processes'], errors='coerce')
df['time_seconds'] = pd.to_numeric(df['time_seconds'], errors='coerce')

# Agrupar por procesos
g = df.groupby('processes').agg(
    runs=('time_seconds','count'),
    mean_time=('time_seconds','mean'),
    std_time=('time_seconds','std'),
    mean_key=('found_key','first')
).reset_index().sort_values('processes')

# calcular speedup relativo a P=1
if 1 in list(g['processes'].values):
    t1 = float(g.loc[g['processes']==1, 'mean_time'].values[0])
else:
    t1 = None

g['speedup'] = g['mean_time'].apply(lambda t: (t1 / t) if (t1 and t and t>0) else None)
g['efficiency'] = g.apply(lambda row: (row['speedup'] / row['processes']) if (row['speedup'] and row['processes']>0) else None, axis=1)

# guardar CSV resumen
g.to_csv(OUT, index=False)

# imprimir resumen por consola
print("Resumen por procesos:")
print(g.to_string(index=False, float_format='%.6f'))

# Graficar tiempo medio vs P (error bars = std)
plt.figure(figsize=(6,4))
plt.errorbar(g['processes'], g['mean_time'], yerr=g['std_time'], fmt='o-', capsize=5)
plt.xlabel("Procesos (P)")
plt.ylabel("Tiempo medio (s)")
plt.title("Tiempo medio vs P")
plt.grid(True)
plt.savefig(os.path.join(PLOT_DIR, "time_vs_P.png"), dpi=150)
plt.close()

# Graficar speedup (y comparar con ideal)
plt.figure(figsize=(6,4))
plt.plot(g['processes'], g['speedup'], 'o-', label='speedup observado')
if t1:
    plt.plot(g['processes'], g['processes'], '--', label='ideal (speedup = P)')
plt.xlabel("Procesos (P)")
plt.ylabel("Speedup")
plt.title("Speedup observado vs P")
plt.legend()
plt.grid(True)
plt.savefig(os.path.join(PLOT_DIR, "speedup_vs_P.png"), dpi=150)
plt.close()

print(f"\nGuardado: {OUT}")
print(f"Plots: {PLOT_DIR}/time_vs_P.png  {PLOT_DIR}/speedup_vs_P.png")
