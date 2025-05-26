# ft_malloc

este readme es un template, y aunque la idea es incorporar todo lo aquí indicado, a día de hoy no representa el estado actual del proyecto.

## 🎯 Descripción

ft_malloc es un proyecto de la escuela 42 que implementa un sistema completo de gestión de memoria dinámico. Esta implementación va significativamente más allá de los requisitos básicos, incorporando técnicas avanzadas de asignación de memoria utilizadas en asignadores de producción como glibc malloc.

## ✨ Características

### 🔧 Funcionalidades Base
- **Funciones Estándar**: `malloc()`, `free()`, `realloc()` compatibles con libc
- **Visualización**: `show_alloc_mem()` para inspección del estado de memoria
- **Thread Safety**: Soporte completo para aplicaciones multi-hilo
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
- **TCache**: Caché thread-local para máximo rendimiento

#### **Optimizaciones de Memoria**
- **Bitmap**: Gestión ultra-eficiente para asignaciones ≤ 16 bytes
- **Coalescing**: Fusión automática de bloques adyacentes libres
- **Alineación**: Alineación óptima de memoria para rendimiento

#### **Protección y Seguridad**
- **Magic Numbers**: Detección de corrupción de memoria y overflow
- **mprotect**: Protección de zonas y asignaciones grandes contra escritura accidental
- **Detección de Overflow**: Identificación temprana de corrupción de memoria
- **Validación de Punteros**: Verificación de integridad en operaciones free/realloc

## 🏗️ Arquitectura

### Estructura de Arenas
```
Arena 1                Arena 2                Arena N
├── FastBins          ├── FastBins          ├── FastBins
├── SmallBins         ├── SmallBins         ├── SmallBins  
├── LargeBins         ├── LargeBins         ├── LargeBins
├── UnsortedBin       ├── UnsortedBin       ├── UnsortedBin
├── TCache            ├── TCache            ├── TCache
└── Zones             └── Zones             └── Zones
    ├── TINY              ├── TINY              ├── TINY
    ├── SMALL             ├── SMALL             ├── SMALL
    └── LARGE             └── LARGE             └── LARGE
```

### Gestión de Tamaños
- **TINY**: 1-128 bytes → Gestión con bitmap
- **SMALL**: 129-1024 bytes → SmallBins + FastBins  
- **LARGE**: >1024 bytes → LargeBins + mmap directo

## 🔧 Instalación

```bash
# Clonar el repositorio
git clone git@github.com:Kobayashi82/Malloc.git
cd ft_malloc

# Compilar la librería
make

# La librería se genera como:
# libft_malloc_$(HOSTTYPE).so
# libft_malloc.so -> libft_malloc_$(HOSTTYPE).so
```

## 🖥️ Uso

### Uso Básico
```bash
# Precargar la librería
export LD_PRELOAD=./libft_malloc.so

# Ejecutar programa existente
./mi_programa

# O en una sola línea
LD_PRELOAD=./libft_malloc.so ./mi_programa
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
gcc programa.c -L. -lft_malloc -o programa

# Ejecutar
./programa
```

## 🔧 Variables de Entorno

### Variables de Debug (Compatibles con malloc del sistema)
```bash
# Habilitar debug general
export MALLOC_DEBUG=1

# Mostrar estadísticas al finalizar
export MALLOC_STATS=1

# Detectar doble free
export MALLOC_CHECK_=2

# Llenar memoria con patrón
export MALLOC_PERTURB_=0x42

# Registrar todas las operaciones
export MALLOC_TRACE=malloc_log.txt
```

### Variables Específicas de ft_malloc
```bash
# Configurar número de arenas
export FT_MALLOC_ARENAS=8

# Habilitar protección mprotect
export FT_MALLOC_MPROTECT=1

# Mostrar información detallada
export FT_MALLOC_VERBOSE=1

# Configurar tamaño de TCache
export FT_MALLOC_TCACHE_SIZE=64
```

## 📚 Funciones Adicionales

### show_alloc_mem_ex()
```c
// Función extendida con información detallada
void show_alloc_mem_ex();
```

**Salida ejemplo:**
```
=== ft_malloc Memory Report ===
ARENA 0 (Thread: 12345)
  FastBins [16]: 3 chunks
  FastBins [24]: 1 chunk  
  SmallBins [32-40]: 5 chunks
  LargeBins [1024+]: 2 chunks
  TCache hits: 1,234 (89.2%)

TINY Zones: 0x7F8A12000000
  Bitmap: 0xFFE3 (12 allocated, 4 free)
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
#include "ft_malloc.h"

// Configurar número máximo de arenas
mallopt(M_ARENA_MAX, 4);

// Configurar umbral para mmap
mallopt(M_MMAP_THRESHOLD, 128*1024);

// Habilitar/deshabilitar TCache
mallopt(M_TCACHE_ENABLE, 1);
```

## ⚡ Optimizaciones

### **Rendimiento**
- **FastBins**: Acceso O(1) para tamaños comunes
- **TCache**: Caché thread-local sin locks
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
# ft_malloc:    1,234,567 ops/sec
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
- ✅ **Detección de Corrupción**: Magic numbers y checksums
- ✅ **Alineación Óptima**: 8/16 bytes según arquitectura
- ✅ **Protección de Páginas**: mprotect para zonas críticas

### **Thread Safety**
- ✅ **Locks Granulares**: Un lock por arena para minimizar contención
- ✅ **TCache Thread-Local**: Acceso sin locks para operaciones comunes
- ✅ **Señales Seguras**: Manejo correcto de interrupciones

### **Compatibilidad**
- ✅ **Drop-in Replacement**: Reemplaza malloc del sistema sin modificaciones
- ✅ **Variables de Entorno**: Compatible con herramientas estándar
- ✅ **ABI Estándar**: Interfaz idéntica a libc malloc

## 📊 Estadísticas de Rendimiento

| Operación | ft_malloc | system malloc | Mejora |
|-----------|-----------|---------------|---------|
| malloc pequeño | 15ns | 23ns | **+35%** |
| malloc medio | 45ns | 67ns | **+33%** |  
| free | 8ns | 12ns | **+33%** |
| Multi-hilo | 892 ops/μs | 634 ops/μs | **+41%** |

esto está por ver... lol
