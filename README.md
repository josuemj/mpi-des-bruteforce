# MPI DES Brute Force

Proyecto de cifrado DES y descifrado por fuerza bruta usando MPI (Message Passing Interface) y OpenSSL.

## Requisitos

- GCC o compilador compatible con C
- OpenSSL (libssl-dev)
- MPI (Open MPI o MPICH)

### Instalación de dependencias (Linux/Ubuntu)

```bash
sudo apt-get update
sudo apt-get install gcc libssl-dev libopenmpi-dev openmpi-bin
```

### Instalación de dependencias (MSYS2/Windows)

```bash
pacman -S gcc openssl-devel openmpi
```
## Compilación

### 1. Compilar el programa de cifrado

```bash
gcc -o encrypt encrypt.c -lcrypto -lssl
```

### 2. Compilar el programa de fuerza bruta

```bash
mpicc -o brute_force brute_force.c -lcrypto -lssl
```

## Uso

### Paso 1: Cifrar un mensaje

El programa de cifrado lee un archivo de texto y genera un archivo binario cifrado usando DES.

**Sintaxis:**
```bash
./encrypt <archivo_entrada.txt> <clave> <archivo_salida.bin>
```

**Parámetros:**
- `<archivo_entrada.txt>`: Ruta del archivo de texto a cifrar
- `<clave>`: Número entero entre 0 y 2^56-1 (clave DES de 56 bits)
- `<archivo_salida.bin>`: Ruta del archivo cifrado de salida

**Ejemplo:**
```bash
./encrypt cipher_input/message.txt 123456 brute_force_input/message_secret.bin
```

**Salida esperada:**
```
Archivo cifrado exitosamente
  Entrada: cipher_input/message.txt (178 bytes)
  Clave: 123456
  Salida: brute_force_input/message_secret.bin (184 bytes)
```

### Paso 2: Descifrar por fuerza bruta

El programa de fuerza bruta utiliza MPI para distribuir la búsqueda de la clave entre múltiples procesos.

**Sintaxis:**
```bash
mpirun -np <num_procesos> ./brute_force <archivo.bin> <palabra_clave>
```

**Parámetros:**
- `<num_procesos>`: Número de procesos MPI a utilizar
- `<archivo.bin>`: Ruta del archivo cifrado
- `<palabra_clave>`: Palabra que debe aparecer en el mensaje descifrado

**Ejemplo con 4 procesos:**
```bash
mpirun -np 4 ./brute_force brute_force_input/message_secret.bin "secreto"
```

**Ejemplo con 8 procesos:**
```bash
mpirun -np 8 ./brute_force brute_force_input/message_secret.bin "secreto"
```

**Salida esperada:**
```
Buscando con 4 procesos MPI
Archivo: brute_force_input/message_secret.bin (184 bytes)
Palabra clave: 'secreto'

Proceso 0: probando clave 100000 (45234 claves/seg)
Proceso 0: probando clave 200000 (50123 claves/seg)
=== ENCONTRADA ===
Clave: 123456
Tiempo: 2.456789 segundos
Mensaje: Este es un mensaje super secreto que contiene the palabra clave...
```

## Ejemplo Completo

### 1. Crear un mensaje de prueba

```bash
echo "Este es un mensaje super secreto para probar el sistema" > cipher_input/message.txt
```

### 2. Cifrar con una clave conocida

```bash
./encrypt cipher_input/message.txt 999999 brute_force_input/message_secret.bin
```

### 3. Descifrar usando fuerza bruta con 4 procesos

```bash
mpirun -np 4 ./brute_force brute_force_input/message_secret.bin "secreto"
```

### 4. Descifrar usando más procesos para mejor rendimiento

```bash
mpirun -np 16 ./brute_force brute_force_input/message_secret.bin "secreto"
```

## Notas Técnicas

### Cifrado
- Utiliza DES en modo ECB con claves de 56 bits
- Aplica padding automático a múltiplos de 8 bytes
- Soporta archivos de texto de cualquier tamaño

### Fuerza Bruta
- Divide el espacio de claves (2^56) entre los procesos MPI
- Cada proceso busca en su rango asignado
- Cuando un proceso encuentra la clave, notifica a todos los demás
- Optimizado con verificación periódica de mensajes MPI (cada 262,144 intentos)
- El proceso 0 muestra progreso cada 100,000 claves probadas

### Rendimiento
- Mayor número de procesos = búsqueda más rápida
- Rendimiento típico: 40,000-60,000 claves/segundo por proceso
- Con 16 procesos: ~640,000-960,000 claves/segundo total

Este proyecto es de uso educativo.