#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <unistd.h>
#include <rpc/des_crypt.h>
#include <time.h>
#include <math.h>

// Función para contar palabras en un texto
int count_words(const char* text) {
    if (!text) return 0;
    
    int word_count = 0;
    int in_word = 0;
    
    for (int i = 0; text[i] != '\0'; i++) {
        if (text[i] != ' ' && text[i] != '\t' && text[i] != '\n' && text[i] != '\r') {
            if (!in_word) {
                word_count++;
                in_word = 1;
            }
        } else {
            in_word = 0;
        }
    }
    
    return word_count;
}

// Función para validar archivo antes del procesamiento
int validate_file_for_decryption(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Error: No se puede abrir el archivo %s\n", filename);
        return 0;
    }
    
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fclose(file);
    
    // Validación de tamaño máximo (aproximadamente 350 palabras * 6 caracteres promedio)
    const long MAX_FILE_SIZE = 350 * 6;
    if (file_size > MAX_FILE_SIZE) {
        printf("Error: El archivo es demasiado grande. Máximo permitido: ~350 palabras\n");
        printf("Tamaño actual: %ld bytes (máximo: %ld bytes)\n", file_size, MAX_FILE_SIZE);
        return 0;
    }
    
    // Validación de que el tamaño sea múltiplo de 8 (bloques DES)
    if (file_size % 8 != 0) {
        printf("Advertencia: El archivo no parece estar correctamente cifrado (tamaño no es múltiplo de 8)\n");
    }
    
    return 1;
}

// Función para leer archivo completo
char* read_file(const char* filename, long* file_size) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Error: No se puede abrir el archivo %s\n", filename);
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    *file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // Validación de tamaño
    if (*file_size <= 0) {
        printf("Error: El archivo está vacío o no se puede leer\n");
        fclose(file);
        return NULL;
    }
    
    if (*file_size > 10000) { // 10KB máximo
        printf("Error: El archivo es demasiado grande (máximo 10KB)\n");
        fclose(file);
        return NULL;
    }
    
    char* buffer = malloc(*file_size + 1);
    if (!buffer) {
        printf("Error: No se puede asignar memoria\n");
        fclose(file);
        return NULL;
    }
    
    size_t bytes_read = fread(buffer, 1, *file_size, file);
    if (bytes_read != *file_size) {
        printf("Error: No se pudo leer el archivo completo\n");
        free(buffer);
        fclose(file);
        return NULL;
    }
    
    buffer[*file_size] = '\0';
    fclose(file);
    
    return buffer;
}

// Función para escribir archivo
int write_file(const char* filename, const char* data, long size) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        printf("Error: No se puede crear el archivo %s\n", filename);
        return 0;
    }
    
    fwrite(data, 1, size, file);
    fclose(file);
    return 1;
}

// Función para cifrar/descifrar archivo completo
void crypt_file(long key, char* data, long size, int encrypt_mode) {
    long k = 0;
    for (int i = 0; i < 8; ++i) {
        key <<= 1;
        k += (key & (0xFE << i * 8));
    }
    des_setparity((char *)&k);
    
    // Procesar en bloques de 8 bytes (DES block size)
    for (long i = 0; i < size; i += 8) {
        long block_size = (i + 8 <= size) ? 8 : size - i;
        if (block_size == 8) {
            ecb_crypt((char *)&k, &data[i], 8, encrypt_mode);
        } else {
            // Padding para último bloque
            char padded_block[8] = {0};
            memcpy(padded_block, &data[i], block_size);
            ecb_crypt((char *)&k, padded_block, 8, encrypt_mode);
            memcpy(&data[i], padded_block, block_size);
        }
    }
}

void decrypt(long key, char *ciph, int len)
{
    
    long k = 0;
    for (int i = 0; i < 8; ++i)
    {
        key <<= 1;
        k += (key & (0xFE << i * 8));
    }
    des_setparity((char *)&k); 
    ecb_crypt((char *)&k, (char *)ciph, 16, DES_DECRYPT);
}
void encrypt(long key, char *ciph, int len)
{
    
    long k = 0;
    for (int i = 0; i < 8; ++i)
    {
        key <<= 1;
        k += (key & (0xFE << i * 8));
    }
    des_setparity((char *)&k); 
    ecb_crypt((char *)&k, (char *)ciph, 16, DES_ENCRYPT);
}
char search[] = " the ";
int tryKey(long key, char *ciph, int len)
{
    char temp[len + 1];
    memcpy(temp, ciph, len);
    temp[len] = 0;
    decrypt(key, temp, len);
    return strstr((char *)temp, search) != NULL;
}
unsigned char cipher[] = {108, 245, 65, 63, 125, 200, 150, 66, 17, 170, 207, 170,
                          34, 31, 70, 215, 0};
int main(int argc, char *argv[])
{ 
    int N, id;
    long upper = (1L << 56); 
    MPI_Status st;
    MPI_Request req;
    int ciphlen = strlen((char *)cipher);
    MPI_Comm comm = MPI_COMM_WORLD;
    
    double start_time, end_time;
    long iterations = 0;
    int use_improved_distribution = 1;
    
    double time_limit = 30.0;
    
    MPI_Init(NULL, NULL);
    MPI_Comm_size(comm, &N);
    MPI_Comm_rank(comm, &id);
    
    // MODO CIFRADO: Requiere llave conocida
    if (argc >= 4 && strcmp(argv[1], "-e") == 0) {
        if (id == 0) { // Solo el proceso 0 maneja archivos
            const char* input_file = argv[2];
            long key = atol(argv[3]);
            const char* output_file = (argc > 4) ? argv[4] : NULL;
            
            // Validaciones de entrada
            if (key <= 0) {
                printf("Error: La llave debe ser un número positivo\n");
                MPI_Finalize();
                return 1;
            }
            
            printf("=== MODO CIFRADO ===\n");
            printf("Archivo entrada: %s\n", input_file);
            printf("Llave: %ld\n", key);
            
            long file_size;
            char* file_data = read_file(input_file, &file_size);
            
            if (!file_data) {
                MPI_Finalize();
                return 1;
            }
            
            // Validar contenido para cifrado
            int word_count = count_words(file_data);
            printf("Palabras en el archivo: %d\n", word_count);
            
            if (word_count > 350) {
                printf("Error: El archivo tiene demasiadas palabras (%d). Máximo permitido: 350\n", word_count);
                free(file_data);
                MPI_Finalize();
                return 1;
            }
            
            printf("Tamaño del archivo: %ld bytes\n", file_size);
            
            // Ajustar tamaño para ser múltiplo de 8 (DES block size)
            long padded_size = ((file_size + 7) / 8) * 8;
            if (padded_size != file_size) {
                file_data = realloc(file_data, padded_size);
                if (!file_data) {
                    printf("Error: No se puede reasignar memoria\n");
                    MPI_Finalize();
                    return 1;
                }
                memset(file_data + file_size, 0, padded_size - file_size);
                printf("Archivo expandido a %ld bytes (múltiplo de 8)\n", padded_size);
            }
            
            start_time = MPI_Wtime();
            crypt_file(key, file_data, padded_size, DES_ENCRYPT);
            end_time = MPI_Wtime();
            
            if (output_file) {
                if (write_file(output_file, file_data, padded_size)) {
                    printf("Archivo cifrado guardado en: %s\n", output_file);
                } else {
                    printf("Error al guardar archivo cifrado\n");
                    free(file_data);
                    MPI_Finalize();
                    return 1;
                }
            } else {
                printf("Error: Debe especificar un archivo de salida para el cifrado\n");
                free(file_data);
                MPI_Finalize();
                return 1;
            }
            
            printf("Tiempo de cifrado: %.6f segundos\n", end_time - start_time);
            free(file_data);
        }
        
        MPI_Finalize();
        return 0;
    }
    
    // MODO DESCIFRADO: Búsqueda por fuerza bruta (NO requiere llave)
    if (argc >= 3 && strcmp(argv[1], "-d") == 0) {
        const char* input_file = argv[2];
        
        if (id == 0) {
            printf("=== MODO DESCIFRADO POR FUERZA BRUTA ===\n");
            printf("Archivo a descifrar: %s\n", input_file);
            printf("Buscando llave usando la palabra clave: '%s'\n", search);
            printf("Procesos MPI: %d\n", N);
            
            // Validar archivo antes del descifrado
            if (!validate_file_for_decryption(input_file)) {
                MPI_Finalize();
                return 1;
            }
        }
        
        // Leer archivo cifrado
        long file_size;
        char* cipher_data = NULL;
        
        if (id == 0) {
            cipher_data = read_file(input_file, &file_size);
            if (!cipher_data) {
                MPI_Finalize();
                return 1;
            }
            printf("Tamaño del archivo cifrado: %ld bytes\n", file_size);
        }
        
        // Broadcast del archivo cifrado a todos los procesos
        MPI_Bcast(&file_size, 1, MPI_LONG, 0, comm);
        
        if (id != 0) {
            cipher_data = malloc(file_size);
            if (!cipher_data) {
                printf("Proceso %d: Error al asignar memoria\n", id);
                MPI_Finalize();
                return 1;
            }
        }
        
        MPI_Bcast(cipher_data, file_size, MPI_CHAR, 0, comm);
        
        // Realizar búsqueda por fuerza bruta distribuida
        long found = 0;
        MPI_Irecv(&found, 1, MPI_LONG, MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &req);
        
        start_time = MPI_Wtime();
        iterations = 0;
        
        // Distribución intercalada para mejor balance
        for (long key = id + 1; key < upper && (found == 0); key += N) {
            iterations++;
            
            // Crear copia temporal para probar
            char* temp_data = malloc(file_size);
            if (temp_data) {
                memcpy(temp_data, cipher_data, file_size);
                crypt_file(key, temp_data, file_size, DES_DECRYPT);
                
                // Buscar la palabra clave
                if (strstr(temp_data, search) != NULL) {
                    found = key;
                    printf("\n¡LLAVE ENCONTRADA por proceso %d!\n", id);
                    
                    // Notificar a todos los procesos
                    for (int node = 0; node < N; node++) {
                        MPI_Send(&found, 1, MPI_LONG, node, 0, MPI_COMM_WORLD);
                    }
                    
                    free(temp_data);
                    break;
                }
                
                free(temp_data);
            }
            
            // Reporte de progreso cada 100,000 iteraciones
            if (iterations % 100000 == 0) {
                double current_time = MPI_Wtime();
                if (current_time - start_time > time_limit) {
                    printf("Proceso %d: Tiempo límite alcanzado (%d seg)\n", id, (int)time_limit);
                    break;
                }
                if (id == 0) {
                    printf("Progreso: %ld llaves probadas por proceso 0...\n", iterations);
                }
            }
        }
        
        end_time = MPI_Wtime();
        
        // Recopilar resultados
        if (id == 0) {
            MPI_Wait(&req, &st);
            
            if (found > 0) {
                // Descifrar con la llave encontrada
                crypt_file(found, cipher_data, file_size, DES_DECRYPT);
                
                printf("\n=== DESCIFRADO EXITOSO ===\n");
                printf("Llave encontrada: %ld\n", found);
                printf("Tiempo total: %.6f segundos\n", end_time - start_time);
                printf("\nContenido descifrado:\n");
                printf("========================\n");
                printf("%s\n", cipher_data);
                printf("========================\n");
                
                // Validar que el descifrado es correcto
                int decrypted_words = count_words(cipher_data);
                printf("\nPalabras en texto descifrado: %d\n", decrypted_words);
                
            } else {
                printf("\nNo se encontró la llave en el tiempo límite de %d segundos\n", (int)time_limit);
            }
        }
        
        free(cipher_data);
        MPI_Finalize();
        return 0;
    }
    
    // Modo brute force (comportamiento original)
    if (argc > 1) {
        use_improved_distribution = atoi(argv[1]);
    }
    
    long found = 0;
    MPI_Irecv(&found, 1, MPI_LONG, MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &req);
    
    start_time = MPI_Wtime();
    
    if (use_improved_distribution == 0) {
        // DISTRIBUCIÓN ORIGINAL: División equitativa secuencial
        if (id == 0) printf("Usando distribución ORIGINAL (secuencial)\n");
        
        int range_per_node = upper / N;
        long mylower = range_per_node * id;
        long myupper = range_per_node * (id + 1) - 1;
        if (id == N - 1) {
            myupper = upper;
        }
        
        for (long i = mylower; i < myupper && (found == 0); ++i) {
            iterations++;
            
            if (iterations % 10000 == 0) {
                double current_time = MPI_Wtime();
                if (current_time - start_time > time_limit) {
                    printf("Proceso %d: Tiempo límite alcanzado (%d seg)\n", id, (int)time_limit);
                    break;
                }
                printf("Proceso %d: %ld iteraciones, %.2f%% completado\n", 
                       id, iterations, 100.0 * (i - mylower) / (myupper - mylower));
            }
            
            if (tryKey(i, (char *)cipher, ciphlen)) {
                found = i;
                for (int node = 0; node < N; node++) {
                    MPI_Send(&found, 1, MPI_LONG, node, 0, MPI_COMM_WORLD);
                }
                break;
            }
        }
    } else {
        // DISTRIBUCIÓN MEJORADA: Intercalada para mejor balance
        if (id == 0) printf("Usando distribución MEJORADA (intercalada)\n");
        
        for (long i = id; i < upper && (found == 0); i += N) {
            iterations++;
            
            if (iterations % 10000 == 0) {
                double current_time = MPI_Wtime();
                if (current_time - start_time > time_limit) {
                    printf("Proceso %d: Tiempo límite alcanzado (%d seg)\n", id, (int)time_limit);
                    break;
                }
            }
            
            if (tryKey(i, (char *)cipher, ciphlen)) {
                found = i;
                for (int node = 0; node < N; node++) {
                    MPI_Send(&found, 1, MPI_LONG, node, 0, MPI_COMM_WORLD);
                }
                break;
            }
        }
    }
    
    end_time = MPI_Wtime();
    
    // Recopilar estadísticas de rendimiento
    long all_iterations[N];
    double all_times[N];
    double execution_time = end_time - start_time;
    
    MPI_Gather(&iterations, 1, MPI_LONG, all_iterations, 1, MPI_LONG, 0, comm);
    MPI_Gather(&execution_time, 1, MPI_DOUBLE, all_times, 1, MPI_DOUBLE, 0, comm);
    
    if (id == 0) {
        MPI_Wait(&req, &st);
        
        if (found > 0) {
            decrypt(found, (char *)cipher, ciphlen);
            printf("\n=== RESULTADO ===\n");
            printf("%li %s\n", found, cipher);
            
            printf("\n=== ANÁLISIS DE RENDIMIENTO ===\n");
            printf("Método: %s\n", use_improved_distribution ? "Intercalado (Mejorado)" : "Secuencial (Original)");
            printf("Procesos: %d\n", N);
            printf("Key encontrada: %li\n", found);
            
            printf("\nIteraciones por proceso:\n");
            printf("Proceso\tIteraciones\tTiempo (s)\n");
            
            long total_iterations = 0;
            double max_time = 0.0;
            double min_iterations = all_iterations[0];
            double max_iterations = all_iterations[0];
            
            for (int i = 0; i < N; i++) {
                printf("%d\t%ld\t\t%.6f\n", i, all_iterations[i], all_times[i]);
                total_iterations += all_iterations[i];
                if (all_times[i] > max_time) max_time = all_times[i];
                if (all_iterations[i] < min_iterations) min_iterations = all_iterations[i];
                if (all_iterations[i] > max_iterations) max_iterations = all_iterations[i];
            }
            
            double avg_iterations = (double)total_iterations / N;
            double variance = 0.0;
            
            for (int i = 0; i < N; i++) {
                double diff = all_iterations[i] - avg_iterations;
                variance += diff * diff;
            }
            variance /= N;
            double std_dev = sqrt(variance);
            
            printf("\n=== MÉTRICAS DE BALANCE ===\n");
            printf("Total iteraciones: %ld\n", total_iterations);
            printf("Promedio por proceso: %.2f\n", avg_iterations);
            printf("Mín iteraciones: %.0f\n", min_iterations);
            printf("Máx iteraciones: %.0f\n", max_iterations);
            printf("Desviación estándar: %.2f\n", std_dev);
            printf("Coeficiente de variación: %.4f\n", std_dev / avg_iterations);
            printf("Tiempo total: %.6f segundos\n", max_time);
            
            printf("\nBalance de carga: %s\n", 
                   (std_dev / avg_iterations < 0.1) ? "EXCELENTE" : 
                   (std_dev / avg_iterations < 0.3) ? "BUENO" : "NECESITA MEJORA");
            
            
        } else {
            printf("Búsqueda terminada sin encontrar la key en %d segundos\n", (int)time_limit);
            
            long search_total = 0;
            for (int i = 0; i < N; i++) {
                search_total += all_iterations[i];
            }
            
            printf("Total de iteraciones realizadas: %ld\n", search_total);
            printf("Porcentaje del espacio explorado: %.8f%%\n", 
                   100.0 * search_total / upper);
        }
    }
    
    MPI_Finalize();
}