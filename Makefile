CC = mpicc
CFLAGS = -Wall -O2
LDFLAGS = -lcrypt

TARGET = bruteforce
SRCDIR = src/sample
SOURCES = $(SRCDIR)/bruteforce.c

# Docker targets
docker-build:
	docker build -t mpi-des-bruteforce .

docker-run: docker-build
	docker run --rm mpi-des-bruteforce

docker-run-single: docker-build
	docker run --rm mpi-des-bruteforce mpirun --allow-run-as-root --oversubscribe -np 1 ./bruteforce

docker-run-multi: docker-build
	docker run --rm mpi-des-bruteforce mpirun --allow-run-as-root --oversubscribe -np 4 ./bruteforce

docker-shell: docker-build
	docker run --rm -it mpi-des-bruteforce /bin/bash

# File encryption/decryption targets
docker-encrypt-sample: docker-build
	docker run --rm -v "$(PWD)/input:/app/input" -v "$(PWD)/output:/app/output" \
		mpi-des-bruteforce mpirun --allow-run-as-root --oversubscribe -np 1 \
		./bruteforce -e input/sample.txt 123 output/sample_encrypted.bin

docker-decrypt-sample: docker-build
	docker run --rm -v "$(PWD)/output:/app/output" \
		mpi-des-bruteforce mpirun --allow-run-as-root --oversubscribe -np 4 \
		./bruteforce -d output/sample_encrypted.bin

docker-decrypt-fast: docker-build
	docker run --rm -v "$(PWD)/output:/app/output" \
		mpi-des-bruteforce mpirun --allow-run-as-root --oversubscribe -np 8 \
		./bruteforce -d output/sample_encrypted.bin

docker-test-validation: docker-build
	docker run --rm -v "$(PWD)/input:/app/input" -v "$(PWD)/output:/app/output" \
		mpi-des-bruteforce mpirun --allow-run-as-root --oversubscribe -np 1 \
		./bruteforce -e input/large_file.txt 789 output/should_fail.bin

# Local targets (may not work on modern Ubuntu)
all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES) $(LDFLAGS)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	mpirun -np 4 ./$(TARGET)

.PHONY: all clean run docker-build docker-run docker-run-single docker-run-multi docker-shell \
        docker-encrypt-sample docker-decrypt-sample docker-decrypt-fast docker-test-validation

# Help target to show available commands
help:
	@echo "Available targets:"
	@echo "  Docker Operations:"
	@echo "    docker-build         - Build the Docker image"
	@echo "    docker-run          - Run brute-force attack (4 processes)"
	@echo "    docker-run-single   - Run with 1 process"
	@echo "    docker-run-multi    - Run with 4 processes"
	@echo "    docker-shell        - Interactive shell in container"
	@echo ""
	@echo "  File Operations:"
	@echo "    docker-encrypt-sample   - Encrypt sample.txt with key 123"
	@echo "    docker-decrypt-sample   - Decrypt sample_encrypted.bin (4 processes)"
	@echo "    docker-decrypt-fast     - Decrypt sample_encrypted.bin (8 processes)"
	@echo "    docker-test-validation  - Test validation with large file"
	@echo ""
	@echo "  Local Operations:"
	@echo "    all                 - Compile locally"
	@echo "    clean              - Clean build files"
	@echo "    run                - Run locally with MPI"
	@echo ""
	@echo "  Usage Examples:"
	@echo "    make docker-encrypt-sample"
	@echo "    make docker-decrypt-fast"
	@echo "    docker run --rm -v \"\$${PWD}/input:/app/input\" -v \"\$${PWD}/output:/app/output\" \\"
	@echo "      mpi-des-bruteforce mpirun --allow-run-as-root -np 1 \\"
	@echo "      ./bruteforce -e input/your_file.txt 456 output/encrypted.bin"