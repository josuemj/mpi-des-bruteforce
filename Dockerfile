# Usar una imagen base más antigua que tenga las librerías DES
FROM ubuntu:18.04

# Instalar dependencias necesarias
RUN apt-get update && apt-get install -y \
    build-essential \
    openmpi-bin \
    openmpi-common \
    libopenmpi-dev \
    libc6-dev \
    openssh-client \
    && rm -rf /var/lib/apt/lists/*

# Establecer directorio de trabajo
WORKDIR /app

# Copiar archivos del proyecto
COPY . .

# Compilar el proyecto
RUN mpicc -Wall -O2 -o bruteforce src/sample/bruteforce.c -lcrypt -lm

# Comando por defecto con configuraciones MPI apropiadas para Docker
CMD ["mpirun", "--allow-run-as-root", "--oversubscribe", "-np", "4", "./bruteforce"]