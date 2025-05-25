#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

// alineacion
	// #define ALIGNMENT 16
	// #define ALIGN(size) (((size) + ALIGNMENT - 1) & ~(ALIGNMENT - 1))
// arenas
// magic number
// zonas
// bitmap
// freelist
// tcache
// free: double free
// free: invalid pointer
// fusion de bloques
// liberacion de zonas
// lazy coalescing (or not)
// best fit vs first fit

/*
 * Este programa de prueba utiliza la implementación personalizada de malloc
 * Para compilarlo: gcc -o test_malloc main.c -L/ruta/a/tu/lib -lft_malloc
 * Para ejecutarlo: LD_PRELOAD=../build/lib/libft_malloc.so ./test_malloc
 */

// Función de prueba para un hilo
void *thread_test(void *arg)
{
    int thread_num = *(int *)arg;
    char *str;
    int i;
    
    printf("Hilo %d: Iniciando prueba de malloc\n", thread_num);
    
    // Asignar y liberar memoria múltiples veces
    for (i = 0; i < 5; i++) {
        str = (char *)malloc(64);
        if (str) {
            sprintf(str, "Asignación #%d desde hilo %d", i, thread_num);
            printf("Hilo %d: %s (dirección: %p)\n", thread_num, str, str);
            free(str);
        } else {
            printf("Hilo %d: Fallo en malloc\n", thread_num);
        }
    }
    
    // Probar asignaciones más grandes
    void *large_alloc = malloc(1024 * 1024); // 1MB
    if (large_alloc) {
        printf("Hilo %d: Asignación grande exitosa: %p\n", thread_num, large_alloc);
        free(large_alloc);
    }
    
    return NULL;
}

// Prueba asignación de múltiples tamaños
void test_various_sizes(void)
{
    printf("\n=== Prueba de asignaciones de varios tamaños ===\n");
    
    // Pequeñas asignaciones (probablemente TINY)
    char *small = (char *)malloc(16);
    if (small) {
        strcpy(small, "Tiny alloc");
        printf("Asignación pequeña: %s (dirección: %p)\n", small, small);
    }
    
    // Asignaciones medianas (probablemente SMALL)
    char *medium = (char *)malloc(256);
    if (medium) {
        strcpy(medium, "Small alloc");
        printf("Asignación mediana: %s (dirección: %p)\n", medium, medium);
    }
    
    // Asignaciones grandes (probablemente LARGE)
    char *large = (char *)malloc(10240); // 10KB
    if (large) {
        strcpy(large, "Large alloc");
        printf("Asignación grande: %s (dirección: %p)\n", large, large);
    }
    
    // Limpiar memoria
    free(small);
    free(medium);
    free(large);
}

// Probar realloc
void test_realloc(void)
{
    printf("\n=== Prueba de realloc ===\n");
    
    // Asignar memoria inicial
    char *ptr = (char *)malloc(64);
    if (ptr) {
        strcpy(ptr, "Memoria inicial");
        printf("Asignación inicial (%lu bytes): %s (dirección: %p)\n", 
               (unsigned long)64, ptr, ptr);
        
        // Ampliar la memoria
        ptr = (char *)realloc(ptr, 128);
        if (ptr) {
            strcat(ptr, " - ampliada con realloc");
            printf("Después de realloc (%lu bytes): %s (dirección: %p)\n", 
                   (unsigned long)128, ptr, ptr);
        }
        
        // Liberar memoria
        free(ptr);
    }
}

// Función externa para activar debug (declarada en malloc.c)
extern void malloc_debug(int enable);
extern void free_debug(int enable);
extern void realloc_debug(int enable);

int main() {
    printf("=== Programa de prueba para malloc personalizado ===\n\n");
    
#ifdef DEBUG_MODE
    // Activar depuración
    printf("Activando modo debug...\n");
    malloc_debug(1);
    free_debug(1);
    realloc_debug(1);
#endif
    
    // 1. Prueba básica de malloc y free
    printf("=== Prueba básica de malloc ===\n");
    char *str = malloc(100);
    if (str) {
        strcpy(str, "Hola desde malloc personalizado!");
        printf("String asignado: %s (dirección: %p)\n", str, str);
        free(str);
        printf("Memoria liberada\n");
    } else {
        printf("Malloc falló\n");
    }
    
    // 2. Prueba con asignaciones de varios tamaños
    test_various_sizes();
    
    // 3. Prueba de realloc
    test_realloc();
    
    // 4. Prueba con múltiples hilos
    printf("\n=== Prueba con múltiples hilos ===\n");
    pthread_t threads[3];
    int thread_args[3];
    int i;
    
    for (i = 0; i < 3; i++) {
        thread_args[i] = i + 1;
        if (pthread_create(&threads[i], NULL, thread_test, &thread_args[i]) != 0) {
            perror("Error creando hilo");
            return 1;
        }
    }
    
    // Esperar a que todos los hilos terminen
    for (i = 0; i < 3; i++)
        pthread_join(threads[i], NULL);
    
    printf("\n=== Pruebas completadas ===\n");
    
    return 0;
}