//bruteforce.c - Version optimizada con OpenSSL
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <mpi.h>
#include <openssl/des.h>

// Conversión de clave 56-bit a 64-bit con paridad
static inline void key56_to_block(uint64_t key56, DES_cblock *out){
    uint64_t k = 0;
    for(int i=0; i<8; i++){
        key56 <<= 1;
        k |= (key56 & (0xFEull << (i*8)));
    }
    memcpy(out, &k, 8);
    DES_set_odd_parity(out);
}

static inline void des_decrypt(uint64_t key, unsigned char *buf, int len){
    DES_cblock cb;
    DES_key_schedule ks;
    key56_to_block(key, &cb);
    DES_set_key_unchecked(&cb, &ks);
    for(int i=0; i<len; i+=8){
        DES_ecb_encrypt((const_DES_cblock*)(buf+i), (DES_cblock*)(buf+i), &ks, DES_DECRYPT);
    }
}

static inline int tryKey(uint64_t key, const unsigned char *ciph, int len, const char *search){
    unsigned char *tmp = (unsigned char*)malloc(len + 1);
    if(!tmp) return 0;
    memcpy(tmp, ciph, len);
    tmp[len] = 0;
    des_decrypt(key, tmp, len);
    int found = (strstr((const char*)tmp, search) != NULL);
    free(tmp);
    return found;
}

int main(int argc, char *argv[]){
    if(argc != 3){
        printf("Uso: %s <archivo.bin> <palabra_clave>\n", argv[0]);
        return 1;
    }

    char *filename = argv[1];
    char *search = argv[2];

    // Leer archivo
    FILE *f = fopen(filename, "rb");
    if(!f){
        printf("Error: no se pudo abrir %s\n", filename);
        return 1;
    }
    fseek(f, 0, SEEK_END);
    long filesize = ftell(f);
    fseek(f, 0, SEEK_SET);
    unsigned char *cipher = malloc(filesize);
    size_t bytes_read = fread(cipher, 1, filesize, f);
    fclose(f);

    if(bytes_read != filesize){
        printf("Warning: solo se leyeron %zu de %ld bytes\n", bytes_read, filesize);
    }

    if(filesize % 8 != 0){
        printf("Error: archivo no es múltiplo de 8 bytes\n");
        free(cipher);
        return 1;
    }

    // MPI init
    int N, id;
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &N);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    // Rango de búsqueda
    uint64_t upper = (1ULL << 56);
    uint64_t range = upper / N;
    uint64_t mylower = range * id;
    uint64_t myupper = (id == N-1) ? upper : (range * (id+1));

    if(id == 0){
        printf("Buscando con %d procesos MPI\n", N);
        printf("Archivo: %s (%ld bytes)\n", filename, filesize);
        printf("Palabra clave: '%s'\n\n", search);
    }

    // Receive buffer para notificaciones
    const uint64_t NOT_FOUND = UINT64_MAX;
    uint64_t found = NOT_FOUND;
    uint64_t recv_buf = NOT_FOUND;
    MPI_Request req;
    MPI_Status st;
    MPI_Irecv(&recv_buf, 1, MPI_UNSIGNED_LONG_LONG, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &req);

    // Búsqueda
    double t0 = MPI_Wtime();
    uint64_t check_interval = (1ULL << 18); // Verificar MPI cada 262,144 intentos
    uint64_t tries = 0;
    int local_found_flag = 0;

    // Mostrar progreso cada 100k claves (solo proceso 0)
    uint64_t progress_interval = 100000;

    for(uint64_t k = mylower; k < myupper && found == NOT_FOUND; k++){
        if(tryKey(k, cipher, filesize, search)){
            found = k;
            local_found_flag = 1;
            // Notificar a todos los procesos
            for(int node = 0; node < N; node++){
                if(node != id){
                    MPI_Send(&found, 1, MPI_UNSIGNED_LONG_LONG, node, 0, MPI_COMM_WORLD);
                }
            }
            break;
        }

        // Mostrar progreso (solo proceso 0)
        if(id == 0 && k > 0 && k % progress_interval == 0){
            double elapsed = MPI_Wtime() - t0;
            double rate = k / elapsed;
            printf("Proceso 0: probando clave %llu (%.0f claves/seg)\n",
                   (unsigned long long)k, rate);
        }

        // Verificar mensajes de otros procesos (optimizado)
        if((++tries % check_interval) == 0){
            int flag = 0;
            MPI_Test(&req, &flag, &st);
            if(flag && recv_buf != NOT_FOUND){
                found = recv_buf;
                break;
            }
        }
    }

    // Verificar si hay mensaje pendiente
    if(found == NOT_FOUND){
        int flag = 0;
        MPI_Test(&req, &flag, &st);
        if(flag) found = recv_buf;
    }

    // Sincronizar resultado global
    uint64_t my_val = (found != NOT_FOUND) ? found : UINT64_MAX;
    uint64_t global_found = UINT64_MAX;
    MPI_Allreduce(&my_val, &global_found, 1, MPI_UNSIGNED_LONG_LONG, MPI_MIN, MPI_COMM_WORLD);

    double t1 = MPI_Wtime();

    // Rank 0 imprime resultado
    if(id == 0){
        if(global_found == UINT64_MAX){
            printf("No se encontró la clave (tiempo: %.6f s)\n", t1-t0);
        } else {
            unsigned char *result = malloc(filesize + 1);
            memcpy(result, cipher, filesize);
            result[filesize] = 0;
            des_decrypt(global_found, result, filesize);

            printf("=== ENCONTRADA ===\n");
            printf("Clave: %llu\n", (unsigned long long)global_found);
            printf("Tiempo: %.6f segundos\n", t1-t0);
            printf("Mensaje: %s\n", result);
            free(result);
        }
    }

    free(cipher);
    MPI_Finalize();
    return 0;
}
