//encrypt.c - Version con OpenSSL
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <openssl/des.h>

static inline void key56_to_block(uint64_t key56, DES_cblock *out){
    uint64_t k = 0;
    for(int i=0; i<8; i++){
        key56 <<= 1;
        k |= (key56 & (0xFEull << (i*8)));
    }
    memcpy(out, &k, 8);
    DES_set_odd_parity(out);
}

void encrypt(uint64_t key, unsigned char *buf, int len){
    DES_cblock cb;
    DES_key_schedule ks;
    key56_to_block(key, &cb);
    DES_set_key_unchecked(&cb, &ks);
    for(int i=0; i<len; i+=8){
        DES_ecb_encrypt((const_DES_cblock*)(buf+i), (DES_cblock*)(buf+i), &ks, DES_ENCRYPT);
    }
}

int main(int argc, char *argv[]){
    if(argc != 4){
        printf("Uso: %s <archivo_entrada.txt> <clave> <archivo_salida.bin>\n", argv[0]);
        printf("  clave: numero entero positivo (0 a 2^56-1)\n");
        return 1;
    }

    char *input_file = argv[1];
    uint64_t key = strtoull(argv[2], NULL, 10);
    char *output_file = argv[3];

    // Validar clave
    if(key >= (1ULL << 56)){
        printf("Error: clave fuera de rango DES\n");
        return 1;
    }

    // Leer archivo de entrada
    FILE *f = fopen(input_file, "r");
    if(!f){
        printf("Error: no se pudo abrir %s\n", input_file);
        return 1;
    }

    fseek(f, 0, SEEK_END);
    long filesize = ftell(f);
    fseek(f, 0, SEEK_SET);

    // Padding a múltiplo de 8 bytes
    int padded_size = ((filesize + 7) / 8) * 8;
    unsigned char *plaintext = calloc(padded_size, 1);

    fread(plaintext, 1, filesize, f);
    fclose(f);

    // Cifrar
    encrypt(key, plaintext, padded_size);

    // Escribir archivo cifrado
    FILE *out = fopen(output_file, "wb");
    if(!out){
        printf("Error: no se pudo crear %s\n", output_file);
        free(plaintext);
        return 1;
    }

    fwrite(plaintext, 1, padded_size, out);
    fclose(out);

    printf("Archivo cifrado exitosamente\n");
    printf("  Entrada: %s (%ld bytes)\n", input_file, filesize);
    printf("  Clave: %llu\n", (unsigned long long)key);
    printf("  Salida: %s (%d bytes)\n", output_file, padded_size);

    free(plaintext);
    return 0;
}
