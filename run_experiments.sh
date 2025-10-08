#!/usr/bin/env bash
set -euo pipefail

# matrix
FILES=("brute_force_input/msg_small.bin" "brute_force_input/msg_big.bin")
PROCS=(1 2 4 8 16)
REPEATS=5
OUTDIR=experiments/$(date +%Y%m%d_%H%M%S)
mkdir -p "$OUTDIR" logs

# compile
gcc -o encrypt encrypt.c -lcrypto -lssl
mpicc -o brute_force brute_force.c -lcrypto -lssl

for f in "${FILES[@]}"; do
  for p in "${PROCS[@]}"; do
    for r in $(seq 1 $REPEATS); do
      LOG="logs/$(basename $f)_P${p}_run${r}.log"
      echo "RUN file=$f P=$p run=$r -> $LOG"
      mpirun -np $p ./brute_force "$f" "secreto" > "$LOG" 2>&1
      python3 collect.py --log "$LOG" --out experiments/results_raw.csv
      sleep 1
    done
  done
done
