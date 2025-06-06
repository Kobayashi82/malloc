# Malloc

Este readme es un template, y aunque la idea es incorporar todo lo aquí indicado, a día de hoy no representa el estado actual del proyecto.

## 🎯 Descripción

Malloc es un proyecto de la escuela 42 que implementa un sistema completo de gestión de memoria dinámico. Esta implementación va significativamente más allá de los requisitos básicos, incorporando técnicas avanzadas de asignación de memoria utilizadas en asignadores de producción como glibc malloc.

## ✨ Características

### 🔧 Funcionalidades Base
- **Funciones Estándar**: `malloc()`, `free()`, `realloc()` compatibles con libc
- **Visualización**: `show_alloc_mem()` para inspección del estado de memoria
- **Thread Safety**: Soporte completo para aplicaciones multi-hilo y forks
- **Gestión de Zonas**: Sistema de zonas TINY, SMALL y LARGE optimizado

### 🚀 Características Avanzadas

#### **Sistema de Arenas**
- **Múltiples Arenas**: Cada hilo puede usar arenas separadas para reducir contención
- **Balanceado de Carga**: Distribución inteligente entre arenas disponibles
- **Escalabilidad**: Mejor rendimiento en aplicaciones multi-hilo intensivas

#### **Bins Especializados**
- **FastBin**: Cacheo rápido para asignaciones pequeñas y frecuentes
- **SmallBin**: Gestión eficiente de bloques pequeños (< 512 bytes)
- **LargeBin**: Ordenamiento por tamaño para bloques grandes
- **UnsortedBin**: Buffer temporal para optimizar reutilización

#### **Optimizaciones de Memoria**
- **Coalescing**: Fusión automática de bloques adyacentes libres
- **Alineación**: Alineación óptima de memoria para rendimiento

#### **Protección y Seguridad**
- **Validación de Punteros**: Verificación de integridad en operaciones free/realloc

## 🏗️ Arquitectura

### Estructura de Arenas
```
Arena 1
├── id					// 
├── *fastbin[10]		// (16-160 bytes) Arrays de listas simples (LIFO)
├── *unsortedbin[10]	// ???
├── *smallbin[31]		// (176-512 bytes para TINY, 513-4096 para SMALL) Doblemente enlazadas
├── *largebin[10]		// ???
├── *tiny				// Linked list of TINY heaps
├── *small				// Linked list of SMALL heaps
├── *large				// Linked list of LARGE heaps (single chunk per heap)
├── *next				// Pointer to the next arena
└── mutex				// Mutex for thread safety in the current arena
```

### Gestión de Tamaños
- **TINY**: 1-128 bytes → Gestión con bitmap
- **SMALL**: 129-1024 bytes → SmallBins + FastBins  
- **LARGE**: >1024 bytes → LargeBins + mmap directo

## 🔧 Instalación

```bash
git clone git@github.com:Kobayashi82/Malloc.git
cd malloc
make

# La librería se genera en ./build/lib como:
# libft_malloc_$(HOSTTYPE).so

# y se crea el enlace simbolico:
# libft_malloc.so -> libft_malloc_$(HOSTTYPE).so
```

## 🖥️ Uso

### Uso Básico
```bash
# Precargar la librería
export LD_LIBRARY_PATH="./build/lib:$LD_LIBRARY_PATH"
export LD_PRELOAD="libft_malloc.so"

# Ejecutar
./program

# O en una sola línea
LD_PRELOAD="./build/lib/libft_malloc.so" ./pogram
```

### Integración en Código C
```c
#include <stdlib.h>

int main() {
    // Usar malloc normalmente
    void *ptr = malloc(1024);
    
    // Visualizar estado de memoria
    show_alloc_mem();
    
    // Liberar memoria
    free(ptr);
    
    return 0;
}
```

### Compilación con la Librería
```bash
# Compilar y enlazar
gcc program.c -L./build/lib -lft_malloc -o program

# Ejecutar
./program
```

## 🔧 Variables de Entorno

### Variables de Debug (Compatibles con malloc del sistema)
```bash
# Habilitar debug general
export MALLOC_DEBUG=1

# Detectar doble free
export MALLOC_CHECK_=2

# Llenar memoria con patrón
export MALLOC_PERTURB_=0x42

# Registrar todas las operaciones
export MALLOC_LOGFILE=malloc_log.txt
...
```

### Variables Específicas de Malloc
```bash
# Configurar número de arenas
export MALLOC_ARENA_MAX=8

```

## 📚 Funciones Adicionales

### show_alloc_mem_ex()
```c
// Función extendida con información detallada
void show_alloc_mem_ex();
```

**Salida ejemplo:**
```
=== Malloc Memory Report ===
ARENA 0 (Thread: 12345)
  FastBins [16]: 3 chunks
  FastBins [24]: 1 chunk  
  SmallBins [32-40]: 5 chunks
  LargeBins [1024+]: 2 chunks

TINY Zones: 0x7F8A12000000
  0x7F8A12000020 - 0x7F8A12000040: 32 bytes [ALLOC]
  0x7F8A12000040 - 0x7F8A12000050: 16 bytes [FREE]

SMALL Zones: 0x7F8A13000000  
  0x7F8A13000020 - 0x7F8A13000220: 512 bytes [ALLOC]
  0x7F8A13000220 - 0x7F8A13000420: 512 bytes [FREE]

LARGE Allocations:
  0x7F8A14000000 - 0x7F8A14002000: 8192 bytes [mmap]

Total Allocated: 52,698 bytes
Total System Memory: 65,536 bytes  
Fragmentation: 19.6%
```

### mallopt() - Control de Comportamiento
```c
#include "malloc.h"

// Configurar número máximo de arenas
mallopt(M_ARENA_MAX, 4);
...
```

## ⚡ Optimizaciones

### **Rendimiento**
- **FastBins**: Acceso O(1) para tamaños comunes
- **Bitmap**: Gestión ultra-rápida para bloques pequeños
- **Coalescing**: Reducción de fragmentación automática

### **Escalabilidad Multi-hilo**
- **Arena por Hilo**: Minimiza contención entre hilos
- **Lock-free TCache**: Operaciones sin bloqueos cuando es posible
- **Balanceado Dinámico**: Distribución inteligente de carga

### **Uso de Memoria**
- **Defragmentación**: Consolidación automática de espacio libre
- **Lazy Allocation**: Asignación de arenas bajo demanda
- **mprotect**: Liberación de páginas no utilizadas

## 🧪 Testing

### Suite de Pruebas Completa
```bash
# Ejecutar todas las pruebas
make test

# Pruebas específicas
make test-basic       # Funcionalidad básica
make test-threading   # Pruebas multi-hilo
make test-performance # Benchmarks de rendimiento
make test-stress      # Pruebas de estrés
make test-memory      # Detección de leaks
make test-corruption  # Detección de corrupción

# Pruebas de compatibilidad
make test-compatibility  # Programas reales
```

### Benchmarks
```bash
# Comparar con malloc del sistema
./benchmark_malloc

# Resultados ejemplo:
# malloc:    1,234,567 ops/sec
# system malloc: 987,654 ops/sec
# Mejora: +25% en operaciones mixtas
```

### Detección de Problemas
```bash
# Ejecutar con Valgrind
valgrind --tool=memcheck ./programa

# Usar AddressSanitizer
gcc -fsanitize=address programa.c -L. -lft_malloc
```

## 🔬 Detalles Técnicos

### **Gestión de Memoria**
- ✅ **Sin Memory Leaks**: Liberación completa de recursos
- ✅ **Detección de Corrupción**: ...
- ✅ **Alineación Óptima**: 8/16 bytes según arquitectura

### **Thread Safety**
- ✅ **Locks Granulares**: Un lock por arena para minimizar contención
- ✅ **Señales Seguras**: Manejo correcto de interrupciones

### **Compatibilidad**
- ✅ **Drop-in Replacement**: Reemplaza malloc del sistema sin modificaciones
- ✅ **Variables de Entorno**: Compatible con herramientas estándar
- ✅ **ABI Estándar**: Interfaz idéntica a libc malloc

## 📊 Estadísticas de Rendimiento

| Operación | malloc | malloc (glibc) | Mejora |
|-----------|-----------|---------------|---------|
| malloc pequeño | 15ns | 23ns | **+35%** |
| malloc medio | 45ns | 67ns | **+33%** |  
| free | 8ns | 12ns | **+33%** |
| Multi-hilo | 892 ops/μs | 634 ops/μs | **+41%** |

esto está por ver... lol

## License

This project is licensed under the WTFPL – [Do What the Fuck You Want to Public License](http://www.wtfpl.net/about/).
