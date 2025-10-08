#!/usr/bin/env python3
import re
import csv
import os
import sys
from datetime import datetime
import socket

if len(sys.argv) < 3:
    print("Uso: collect.py --log <archivo_log> --out <archivo_csv>")
    sys.exit(1)

log_path = sys.argv[sys.argv.index("--log") + 1]
out_path = sys.argv[sys.argv.index("--out") + 1]

# leer log completo
with open(log_path, "r", encoding="utf-8", errors="ignore") as f:
    text = f.read()

# extraer métricas
time_match = re.search(r"Tiempo:\s*([\d\.]+)", text)
key_match = re.search(r"Clave:\s*(\d+)", text)
msg_match = re.search(r"Mensaje:\s*(.+)", text)
proc_match = re.search(r"Buscando con\s+(\d+)\s+procesos", text)

row = {
    "timestamp": datetime.now().isoformat(timespec="seconds"),
    "host": socket.gethostname(),
    "file": os.path.basename(log_path),
    "processes": int(proc_match.group(1)) if proc_match else "",
    "found_key": key_match.group(1) if key_match else "",
    "time_seconds": float(time_match.group(1)) if time_match else "",
    "message_sample": msg_match.group(1)[:50] if msg_match else "",
}

# escribir o agregar CSV
header = list(row.keys())
file_exists = os.path.exists(out_path)
with open(out_path, "a", newline="") as csvfile:
    writer = csv.DictWriter(csvfile, fieldnames=header)
    if not file_exists:
        writer.writeheader()
    writer.writerow(row)
