# Malloc

Este readme es un `template`, y aunque la idea es incorporar todo lo aquí indicado, a día de hoy `no representa el estado` actual del proyecto.

## 🎯 Descripción

Malloc es un proyecto de la escuela 42 que implementa un sistema completo de gestión de memoria dinámico. Esta implementación va significativamente más allá de los requisitos básicos, incorporando técnicas avanzadas de asignación de memoria utilizadas en asignadores de producción como glibc malloc.

## ✨ Características

### 🔧 Funcionalidades Base
- **Funciones Estándar**: `malloc()`, `free()`, `realloc()` compatibles con libc
- **Visualización**: `show_alloc_mem()` para inspección del estado de memoria
- **Thread Safety**: Soporte completo para aplicaciones multi-hilo y forks
- **Gestión de Zonas**: Sistema de zonas TINY, SMALL y LARGE

### 🚀 Características Avanzadas

#### **Sistema de Arenas**
- **Múltiples Arenas**: Cada hilo puede usar arenas separadas para reducir contención
- **Balanceado de Carga**: Distribución inteligente entre arenas disponibles

#### **Bins Especializados**
- **FastBin**: Cacheo rápido para asignaciones pequeñas y frecuentes
- **SmallBin**: Gestión eficiente de bloques pequeños (< 512 bytes)
- **LargeBin**: Ordenamiento por tamaño para bloques grandes
- **UnsortedBin**: Buffer temporal para optimizar reutilización

#### **Optimizaciones de Memoria**
- **Coalescing**: Fusión automática de bloques adyacentes libres
- **Alineación**: Alineación óptima de memoria

#### **Protección y Seguridad**
- **Validación de Punteros**: Verificación de integridad de memoria

## 🏗️ Arquitectura

## 🔧 Instalación

```bash
git clone git@github.com:Kobayashi82/Malloc.git
cd malloc
make

# La librería se genera en ./lib como:
# libft_malloc_$(HOSTTYPE).so

# y se crea el enlace simbolico:
# libft_malloc.so -> libft_malloc_$(HOSTTYPE).so
```

## 🖥️ Uso

### Uso Básico
```bash
# Precargar la librería
export LD_LIBRARY_PATH="./lib:$LD_LIBRARY_PATH"
export LD_PRELOAD="libft_malloc.so"

# Ejecutar
./program

# O en una sola línea
LD_PRELOAD="./lib/libft_malloc.so" ./pogram
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
gcc program.c -L./lib -lft_malloc -o program

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

### show_alloc_mem()

**Salida ejemplo:**
```
————————————
 • Arena #1
———————————————————————————————————————
 • Allocations: 7       • Frees: 1
 • TINY: 1              • SMALL: 1
 • LARGE: 0             • TOTAL: 2
———————————————————————————————————————

 SMALL : 0x70000
— — — — — — — — — — — — — — — — —
 0x70010 - 0x707e0 : 2000 bytes
                    — — — — — — —
                     2000 bytes

 TINY : 0xf0000
— — — — — — — — — — — — — — — — —
 0xf0010 - 0xf0020 : 16 bytes
 0xf0030 - 0xf0040 : 16 bytes
 0xf0050 - 0xf0060 : 16 bytes
 0xf0070 - 0xf0080 : 16 bytes
 0xf0090 - 0xf00a0 : 16 bytes
                    — — — — — — —
                     80 bytes

———————————————————————————————————————
 2080 bytes in arena #1


———————————————————————————————————————————————————————————————
 • 7 allocations, 1 free and 2080 bytes across 1 arena
```


### show_alloc_mem_ex()

**Salida ejemplo:**
```
——————————————————————————————————————
 • Pointer: 0x703ab8cbf010 (Arena #1)
————————————————————————————————————————————————————————————————————————————————————
 • Size: 112 bytes      • Offset: 0 bytes      • Length: 112 bytes
————————————————————————————————————————————————————————————————————————————————————
 0x703ab8cbf000  71 00 00 00 00 00 00 00  89 67 45 23 01 ef cd ab  q........gE#....
————————————————————————————————————————————————————————————————————————————————————
 0x703ab8cbf010  48 65 6c 6c 6f 20 57 6f  72 6c 64 21 00 00 00 00  Hello World!....
 0x703ab8cbf020  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  ................
 0x703ab8cbf030  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  ................
 0x703ab8cbf040  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  ................
 0x703ab8cbf050  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  ................
 0x703ab8cbf060  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  ................
 0x703ab8cbf070  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  ................
————————————————————————————————————————————————————————————————————————————————————
```

### mallopt()
```c
// Configurar número máximo de arenas
mallopt(M_ARENA_MAX, 4);
...
```

## ⚡ Optimizaciones

### **Rendimiento**
- **FastBins**: Acceso O(1) para tamaños comunes
- **Coalescing**: Reducción de fragmentación automática

### **Escalabilidad Multi-hilo**
- **Arena por Hilo**: Minimiza contención entre hilos
- **Balanceado Dinámico**: Distribución inteligente de carga

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

## 🔬 Detalles Técnicos

### **Gestión de Memoria**
- ✅ **Detección de Corrupción**: Magic number and poison bytes
- ✅ **Alineación Óptima**: 8/16 bytes según arquitectura

### **Thread Safety**
- ✅ **Locks Granulares**: Un lock por arena para minimizar contención
- ✅ **Fork-Safe**: Fork seguro en entornos multi-hilo

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

## License

This project is licensed under the WTFPL – [Do What the Fuck You Want to Public License](http://www.wtfpl.net/about/).

## Main

- [X] free
- [X] malloc
- [X] realloc
- [X] calloc

## Extra

- [X] reallocarray
- [X] aligned_alloc
- [X] memalign
- [X] posix_memalign
- [X] malloc_usable_size
- [X] valloc
- [X] pvalloc

## Debug

- [X] mallopt
- [X] show_alloc_mem
- [X] show_alloc_mem_ex
- [X] show_alloc_hist

- [ ] Fork-Safe
