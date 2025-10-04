# MPI DES Bruteforce

A parallel brute-force program for DES encryption/decryption using MPI (Message Passing Interface). This tool can encrypt text files with a known key or decrypt files using brute-force attack with MPI parallelization.
For computacion paralela

## Prerequisites

- Docker Desktop (with WSL2 integration if on Windows)
- Git (to clone the repository)

## Features

- ** File Encryption**: Encrypt text files using DES with a known key
- ** Brute-Force Decryption**: Decrypt files without knowing the key using parallel brute-force
- ** Defensive Programming**: Input validation (max 350 words per file)
- ** MPI Parallelization**: Distribute workload across multiple processes
- ** Performance Monitoring**: Real-time progress reporting and load balancing metrics
- ** Docker Ready**: Containerized for easy deployment and reproducibility

## Quick Start

### 1. Setup

```bash
# Clone the repository
git clone https://github.com/josuemj/mpi-des-bruteforce.git
cd mpi-des-bruteforce

# Build the Docker image
docker build -t mpi-des-bruteforce .
```

### 2. File Encryption Mode

Encrypt a text file using a known DES key:

#### Linux/macOS (bash):
```bash
# Basic encryption (1 process)
docker run --rm -v "${PWD}/input:/app/input" -v "${PWD}/output:/app/output" \
  mpi-des-bruteforce mpirun --allow-run-as-root --oversubscribe -np 1 \
  ./bruteforce -e input/sample.txt 123 output/encrypted_file.bin

# Encrypt with different key
docker run --rm -v "${PWD}/input:/app/input" -v "${PWD}/output:/app/output" \
  mpi-des-bruteforce mpirun --allow-run-as-root --oversubscribe -np 1 \
  ./bruteforce -e input/your_file.txt 456789 output/encrypted.bin
```

#### Windows PowerShell:
```powershell
# Basic encryption (1 process) - SINGLE LINE VERSION
docker run --rm -v "${PWD}/input:/app/input" -v "${PWD}/output:/app/output" mpi-des-bruteforce mpirun --allow-run-as-root --oversubscribe -np 1 ./bruteforce -e input/sample.txt 123 output/encrypted_file.bin

# Encrypt with different key
docker run --rm -v "${PWD}/input:/app/input" -v "${PWD}/output:/app/output" mpi-des-bruteforce mpirun --allow-run-as-root --oversubscribe -np 1 ./bruteforce -e input/your_file.txt 456789 output/encrypted.bin

# Alternative: Using PowerShell continuation (backtick)
docker run --rm -v "${PWD}/input:/app/input" -v "${PWD}/output:/app/output" `
  mpi-des-bruteforce mpirun --allow-run-as-root --oversubscribe -np 1 `
  ./bruteforce -e input/sample.txt 123 output/encrypted_file.bin
```

**Encryption Parameters:**
- `-e`: Encryption mode
- `input/file.txt`: Input text file (max 350 words)
- `123`: DES encryption key (positive integer)
- `output/file.bin`: Output encrypted file

### 3. Brute-Force Decryption Mode

Decrypt a file without knowing the key (finds key using brute-force):

#### Linux/macOS (bash):
```bash
# Single process decryption
docker run --rm -v "${PWD}/output:/app/output" \
  mpi-des-bruteforce mpirun --allow-run-as-root --oversubscribe -np 1 \
  ./bruteforce -d output/encrypted_file.bin

# Multi-process decryption (4 processes - FASTER)
docker run --rm -v "${PWD}/output:/app/output" \
  mpi-des-bruteforce mpirun --allow-run-as-root --oversubscribe -np 4 \
  ./bruteforce -d output/encrypted_file.bin

# High-performance decryption (8 processes)
docker run --rm -v "${PWD}/output:/app/output" \
  mpi-des-bruteforce mpirun --allow-run-as-root --oversubscribe -np 8 \
  ./bruteforce -d output/encrypted_file.bin

# Maximum parallel decryption (16 processes)
docker run --rm -v "${PWD}/output:/app/output" \
  mpi-des-bruteforce mpirun --allow-run-as-root --oversubscribe -np 16 \
  ./bruteforce -d output/encrypted_file.bin
```

#### Windows PowerShell:
```powershell
# Single process decryption
docker run --rm -v "${PWD}/output:/app/output" mpi-des-bruteforce mpirun --allow-run-as-root --oversubscribe -np 1 ./bruteforce -d output/encrypted_file.bin

# Multi-process decryption (4 processes - FASTER)
docker run --rm -v "${PWD}/output:/app/output" mpi-des-bruteforce mpirun --allow-run-as-root --oversubscribe -np 4 ./bruteforce -d output/encrypted_file.bin

# High-performance decryption (8 processes)
docker run --rm -v "${PWD}/output:/app/output" mpi-des-bruteforce mpirun --allow-run-as-root --oversubscribe -np 8 ./bruteforce -d output/encrypted_file.bin

# Maximum parallel decryption (16 processes)
docker run --rm -v "${PWD}/output:/app/output" mpi-des-bruteforce mpirun --allow-run-as-root --oversubscribe -np 16 ./bruteforce -d output/encrypted_file.bin

# Alternative: Using PowerShell continuation (backtick)
docker run --rm -v "${PWD}/output:/app/output" `
  mpi-des-bruteforce mpirun --allow-run-as-root --oversubscribe -np 4 `
  ./bruteforce -d output/encrypted_file.bin
```

**Decryption Parameters:**
- `-d`: Decryption mode (brute-force)
- `output/file.bin`: Encrypted file to decrypt
- `-np X`: Number of MPI processes (1, 2, 4, 8, 16, etc.)

### 4. Original Brute-Force Mode (Hardcoded Cipher)

Run brute-force attack on the hardcoded cipher with different load balancing strategies:

```bash
# Original sequential distribution
docker run --rm mpi-des-bruteforce mpirun --allow-run-as-root --oversubscribe -np 4 ./bruteforce 0

# Improved interleaved distribution (default)
docker run --rm mpi-des-bruteforce mpirun --allow-run-as-root --oversubscribe -np 4 ./bruteforce 1

# Scale up processes for faster search
docker run --rm mpi-des-bruteforce mpirun --allow-run-as-root --oversubscribe -np 8 ./bruteforce 1
```

## Process Scaling Guide

### Choosing Number of Processes

| Processes | Use Case | Expected Performance |
|-----------|----------|---------------------|
| `-np 1` | Testing, small files | Baseline speed |
| `-np 2` | Dual-core systems | ~2x faster |
| `-np 4` | Quad-core systems | ~4x faster |
| `-np 8` | High-performance | ~8x faster |
| `-np 16` | Maximum parallelization | ~16x faster |

### Performance Examples

#### Linux/macOS:
```bash
# Conservative (good for testing)
docker run --rm -v "${PWD}/output:/app/output" \
  mpi-des-bruteforce mpirun --allow-run-as-root --oversubscribe -np 2 \
  ./bruteforce -d output/encrypted.bin

# Balanced (recommended for most cases)
docker run --rm -v "${PWD}/output:/app/output" \
  mpi-des-bruteforce mpirun --allow-run-as-root --oversubscribe -np 4 \
  ./bruteforce -d output/encrypted.bin

# Aggressive (for fast results)
docker run --rm -v "${PWD}/output:/app/output" \
  mpi-des-bruteforce mpirun --allow-run-as-root --oversubscribe -np 8 \
  ./bruteforce -d output/encrypted.bin
```

#### Windows PowerShell:
```powershell
# Conservative (good for testing)
docker run --rm -v "${PWD}/output:/app/output" mpi-des-bruteforce mpirun --allow-run-as-root --oversubscribe -np 2 ./bruteforce -d output/encrypted.bin

# Balanced (recommended for most cases)
docker run --rm -v "${PWD}/output:/app/output" mpi-des-bruteforce mpirun --allow-run-as-root --oversubscribe -np 4 ./bruteforce -d output/encrypted.bin

# Aggressive (for fast results)
docker run --rm -v "${PWD}/output:/app/output" mpi-des-bruteforce mpirun --allow-run-as-root --oversubscribe -np 8 ./bruteforce -d output/encrypted.bin
```

## Available Make Targets

```bash
# Docker operations
make docker-build         # Build the Docker image
make docker-run           # Run default brute-force (4 processes)
make docker-run-single    # Run with 1 process
make docker-run-multi     # Run with 4 processes
make docker-shell         # Interactive shell in container

# Local compilation (may not work on modern systems)
make all                  # Compile locally
make clean               # Clean build files
make run                 # Run locally with MPI
```

## Input Validation & Limits

- **Maximum file size**: 350 words per text file
- **File format**: Plain text (.txt) for encryption
- **Encryption key**: Positive integer (1 to 2^56-1)
- **Search keyword**: Program looks for " the " to verify decryption
- **Timeout**: 30 seconds default for brute-force attempts

## Advanced Usage

### Custom Search Patterns
The program searches for " the " by default. To modify the search pattern, edit `search[]` in `src/sample/bruteforce.c` and rebuild.

### Performance Monitoring
The program provides detailed performance metrics including:
- Iterations per process
- Load balancing analysis
- Time distribution
- Search space coverage

### Batch Processing

#### Linux/macOS:
```bash
# Encrypt multiple files
for file in input/*.txt; do
  name=$(basename "$file" .txt)
  docker run --rm -v "${PWD}/input:/app/input" -v "${PWD}/output:/app/output" \
    mpi-des-bruteforce mpirun --allow-run-as-root -np 1 \
    ./bruteforce -e "input/$name.txt" $RANDOM "output/$name.enc"
done
```

#### Windows PowerShell:
```powershell
# Encrypt multiple files
Get-ChildItem input/*.txt | ForEach-Object {
  $name = $_.BaseName
  $key = Get-Random -Minimum 1 -Maximum 1000000
  docker run --rm -v "${PWD}/input:/app/input" -v "${PWD}/output:/app/output" mpi-des-bruteforce mpirun --allow-run-as-root -np 1 ./bruteforce -e "input/$($_.Name)" $key "output/$name.enc"
}
```

## Quick Copy-Paste Commands for PowerShell

### Test Encryption (sample.txt):
```powershell
docker run --rm -v "${PWD}/input:/app/input" -v "${PWD}/output:/app/output" mpi-des-bruteforce mpirun --allow-run-as-root --oversubscribe -np 1 ./bruteforce -e input/sample.txt 123 output/sample_encrypted.bin
```

### Test Decryption (4 processes):
```powershell
docker run --rm -v "${PWD}/output:/app/output" mpi-des-bruteforce mpirun --allow-run-as-root --oversubscribe -np 4 ./bruteforce -d output/sample_encrypted.bin
```

### Fast Decryption (8 processes):
```powershell
docker run --rm -v "${PWD}/output:/app/output" mpi-des-bruteforce mpirun --allow-run-as-root --oversubscribe -np 8 ./bruteforce -d output/sample_encrypted.bin
```