#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <unistd.h>
#include <rpc/des_crypt.h>
#include <time.h>
#include <math.h>

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