# MPI DES Bruteforce

A parallel brute-force program to crack DES encrypted messages using MPI (Message Passing Interface).

## Prerequisites

- Docker Desktop (with WSL2 integration if on Windows)
- Git (to clone the repository)

## Quick Start

### Using Docker (Recommended)

1. **Clone the repository**:
   ```bash
   git clone https://github.com/josuemj/mpi-des-bruteforce.git
   cd mpi-des-bruteforce
   ```

2. **Build and run with Docker**:
   ```bash
   # Build the Docker image
   make docker-build
   
   # Run with 4 MPI processes
   make docker-run
   ```

### Manual Docker Commands

```bash
# Build the image
docker build -t mpi-des-bruteforce .

# Run with 4 processes (default)
docker run --rm mpi-des-bruteforce

# Run with 1 process
docker run --rm mpi-des-bruteforce mpirun --allow-run-as-root --oversubscribe -np 1 ./bruteforce

# Run with custom number of processes
docker run --rm mpi-des-bruteforce mpirun --allow-run-as-root --oversubscribe -np 8 ./bruteforce
```

### Available Make Targets

- `make docker-build`: Build the Docker image
- `make docker-run`: Run the program with 4 MPI processes
- `make docker-run-single`: Run with 1 MPI process
- `make docker-run-multi`: Run with 4 MPI processes
- `make docker-shell`: Open an interactive shell in the container

## Expected Output

When the program successfully finds the DES key, it will output:

```
68390 Save the planet
```

Where:
- `68390` is the DES key that was found
- `Save the planet` is the decrypted message