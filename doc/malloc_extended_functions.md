# Funciones Extendidas de la Familia malloc

## Funciones de Alineación

### aligned_alloc()
**Prototipo**: `void *aligned_alloc(size_t alignment, size_t size);`

**Propósito**: Asigna memoria con alineación específica según el estándar C11.

**Restricciones**:
- `alignment` debe ser una potencia de 2
- `size` debe ser múltiplo de `alignment`
- Si no se cumplen estas condiciones, el comportamiento es indefinido

**Casos de uso**:
- Optimizaciones SIMD (SSE, AVX) que requieren alineación de 16, 32 o 64 bytes
- Estructuras para DMA que necesitan alineación específica del hardware
- Optimizaciones de cache (alineación a líneas de cache de 64 bytes)
- Algoritmos que requieren alineación para vectorización automática

**Ejemplo de interceptación**:
```c
void* aligned_alloc(size_t alignment, size_t size) {
    // Validar parámetros
    if (!is_power_of_two(alignment) || size % alignment != 0) {
        return NULL;
    }
    
    // Tu lógica de asignación alineada
    void* ptr = your_aligned_allocation(alignment, size);
    if (ptr) {
        register_allocation(ptr, size, alignment);
    }
    return ptr;
}
```

**Consideraciones especiales**:
- En sistemas con múltiples niveles de cache, considerar alineación jerárquica
- Algunos compiladores pueden generar código optimizado automáticamente si detectan alineación
- La alineación excesiva puede desperdiciar memoria significativamente

---

### posix_memalign()
**Prototipo**: `int posix_memalign(void **memptr, size_t alignment, size_t size);`

**Propósito**: Asigna memoria alineada usando interfaz POSIX con manejo de errores explícito.

**Diferencias clave con aligned_alloc()**:
- Devuelve código de error (0 = éxito, errno en caso de fallo)
- El puntero resultado se almacena en `*memptr`
- `alignment` debe ser múltiplo de `sizeof(void*)` además de potencia de 2
- `size` no necesita ser múltiplo de `alignment`

**Valores de retorno**:
- `0`: Éxito
- `EINVAL`: alignment inválido
- `ENOMEM`: memoria insuficiente

**Ventajas**:
- Mejor manejo de errores que aligned_alloc()
- Más flexible en cuanto a restricciones de size
- Ampliamente soportado en sistemas POSIX

**Ejemplo de interceptación**:
```c
int posix_memalign(void **memptr, size_t alignment, size_t size) {
    *memptr = NULL;
    
    // Validaciones POSIX específicas
    if (alignment % sizeof(void*) != 0 || !is_power_of_two(alignment)) {
        return EINVAL;
    }
    
    void* ptr = your_aligned_allocation(alignment, size);
    if (!ptr) {
        return ENOMEM;
    }
    
    *memptr = ptr;
    register_allocation(ptr, size, alignment);
    return 0;
}
```

---

### memalign()
**Prototipo**: `void *memalign(size_t alignment, size_t size);`

**Propósito**: Extensión GNU para asignación de memoria alineada (no estándar).

**Características**:
- Solo requiere que `alignment` sea potencia de 2
- No tiene restricciones sobre `size`
- Específico de glibc y algunas implementaciones BSD
- Comportamiento similar a `aligned_alloc()` pero menos restrictivo

**Portabilidad**:
- No disponible en todos los sistemas
- Puede estar definido como macro a `aligned_alloc()` en algunos sistemas
- Código que lo usa puede no compilar en sistemas no-GNU

**Cuándo interceptar**:
- Solo si tu aplicación target usa glibc o sistemas compatibles
- Para compatibilidad con código legacy que no puede migrar fácilmente
- En aplicaciones que requieren máxima flexibilidad en parámetros de alineación

**Ejemplo de interceptación**:
```c
void* memalign(size_t alignment, size_t size) {
    if (!is_power_of_two(alignment)) {
        return NULL;
    }
    
    // Más flexible que aligned_alloc
    void* ptr = your_aligned_allocation(alignment, size);
    if (ptr) {
        register_allocation(ptr, size, alignment);
    }
    return ptr;
}
```

---

## Funciones de Extensión

### reallocarray()
**Prototipo**: `void *reallocarray(void *ptr, size_t nmemb, size_t size);`

**Propósito**: Redimensiona arrays con protección contra overflow en la multiplicación.

**Problema que resuelve**:
```c
// Peligroso - puede hacer overflow silencioso
ptr = realloc(ptr, num_elements * sizeof(element));

// Seguro - detecta overflow
ptr = reallocarray(ptr, num_elements, sizeof(element));
```

**Detección de overflow**:
- Verifica que `nmemb * size` no cause overflow antes de la asignación
- Falla con `errno = ENOMEM` si detecta overflow potencial
- Previene allocaciones masivas accidentales que podrían crashear el sistema

**Casos de uso críticos**:
- Redimensionamiento de arrays dinámicos basados en input del usuario
- Parsing de archivos donde el tamaño se calcula dinámicamente
- Cualquier operación donde los tamaños pueden venir de fuentes externas

**Implementación típica de verificación**:
```c
void* reallocarray(void *ptr, size_t nmemb, size_t size) {
    // Verificar overflow
    if (nmemb != 0 && size > SIZE_MAX / nmemb) {
        errno = ENOMEM;
        return NULL;
    }
    
    // Proceder con realloc normal
    return realloc(ptr, nmemb * size);
}
```

**Consideraciones para interceptación**:
- Crítica en aplicaciones que procesan datos de tamaño variable
- Especialmente importante en parsers, decoders, y aplicaciones de red
- OpenBSD la introdujo, adoptada por glibc 2.26+

---

## Funciones de Información Críticas

### malloc_usable_size()
**Prototipo**: `size_t malloc_usable_size(void *ptr);`

**Propósito**: Devuelve el tamaño real utilizable de un bloque asignado.

**Por qué es importante**:
- Los allocators suelen asignar más memoria de la solicitada por alineación/granularidad
- Permite optimizar el uso de buffers sin reasignar
- Común en implementaciones de strings dinámicos y buffers de red

**Casos de uso**:
```c
char* buffer = malloc(100);
size_t actual_size = malloc_usable_size(buffer);
// actual_size podría ser 112, 128, etc.
// Podemos usar hasta actual_size bytes sin reasignar
```

**Implementaciones comunes**:
- Strings dinámicos que crecen incrementalmente
- Buffers de I/O que se adaptan al tamaño real disponible
- Optimizaciones en JSON parsers y XML processors

**Riesgos y consideraciones**:
- **No es portable** - extensión GNU
- No debe usarse para validar si un puntero es válido
- El valor puede cambiar entre llamadas si hay compactación de heap
- Algunos allocators pueden devolver 0 para punteros no reconocidos

**Interceptación crítica**:
```c
size_t malloc_usable_size(void *ptr) {
    if (!ptr) return 0;
    
    // Verificar si es nuestro puntero
    if (is_our_pointer(ptr)) {
        return get_our_block_size(ptr);
    }
    
    // Delegar al sistema para punteros externos
    return original_malloc_usable_size(ptr);
}
```

**Aplicaciones que la usan frecuentemente**:
- Redis (optimización de strings)
- jemalloc profiling tools
- Custom memory pools
- High-performance networking libraries

---

## Consideraciones de Interceptación

### Detección de Ownership
Todas estas funciones requieren determinar si un puntero "nos pertenece":

```c
typedef struct {
    void* start;
    void* end;
    size_t size;
    size_t alignment;
} allocation_record;

bool is_our_pointer(void* ptr) {
    // Búsqueda en tabla hash, árbol, o lista de allocaciones
    return find_allocation_record(ptr) != NULL;
}
```

### Delegation Pattern
```c
#define DELEGATE_IF_EXTERNAL(func_name, ptr, ...) \
    do { \
        if (ptr && !is_our_pointer(ptr)) { \
            return original_##func_name(ptr, ##__VA_ARGS__); \
        } \
    } while(0)
```

### Performance Considerations
- `malloc_usable_size()` puede ser llamada frecuentemente - optimizar lookup
- Las funciones de alineación pueden requerir algoritmos más complejos
- `reallocarray()` necesita aritmética cuidadosa para evitar overflow

### Compatibility Testing
- Testear con diferentes allocators (glibc, musl, jemalloc)
- Verificar comportamiento con punteros NULL
- Validar manejo de parámetros inválidos
- Confirmar que la delegación funciona correctamente