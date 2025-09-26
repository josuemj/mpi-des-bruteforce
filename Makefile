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

# Local targets (may not work on modern Ubuntu)
all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES) $(LDFLAGS)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	mpirun -np 4 ./$(TARGET)

.PHONY: all clean run docker-build docker-run docker-shell