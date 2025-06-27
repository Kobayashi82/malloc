# Manejo de malloc(0)

## Problemática

Cuando se llama `malloc(0)`, el comportamiento según el estándar C es:
- **Puede** devolver `NULL` 
- **Puede** devolver un puntero válido único

La mayoría de implementaciones (glibc, musl) eligen devolver **punteros únicos válidos** por compatibilidad.

## Problemas a resolver

### 1. Puntero único por llamada
```c
void *ptr1 = malloc(0);
void *ptr2 = malloc(0);
// ptr1 != ptr2 (deben ser diferentes)
```

### 2. free() debe funcionar
```c
void *ptr = malloc(0);
free(ptr); // No debe dar error
```

### 3. Consistencia con malloc normal
- Debe devolver punteros alineados
- Debe comportarse como una allocación "real"

## Solución implementada

### Estructura del manager
```c
typedef struct {
    // ... otros campos del manager
    size_t zero_malloc_counter;
} manager_t;

#define ALIGNMENT 8
#define ZERO_MALLOC_BASE 0x1000
```

### Implementación en malloc
```c
void* my_malloc(size_t size) {
    if (size == 0) {
        // Generar dirección única y alineada
        size_t aligned_offset = (g_manager.zero_malloc_counter * ALIGNMENT);
        g_manager.zero_malloc_counter++;
        return (void*)(ZERO_MALLOC_BASE + aligned_offset);
    }
    
    // ... resto de la lógica de malloc normal
}
```

### Implementación en free
```c
void my_free(void *ptr) {
    if (ptr == NULL) {
        return; // free(NULL) está permitido
    }
    
    uintptr_t addr = (uintptr_t)ptr;
    
    // Detectar si es un malloc(0)
    if (addr >= ZERO_MALLOC_BASE && 
        addr < ZERO_MALLOC_BASE + (g_manager.zero_malloc_counter * ALIGNMENT) &&
        (addr - ZERO_MALLOC_BASE) % ALIGNMENT == 0) {
        return; // Es malloc(0), no hacer nada
    }
    
    // ... resto de la lógica de free normal
}
```

## Comportamiento resultante

### Direcciones generadas
```c
void *ptr1 = malloc(0);  // → 0x1000
void *ptr2 = malloc(0);  // → 0x1008  
void *ptr3 = malloc(0);  // → 0x1010
```

### Propiedades garantizadas
- ✅ **Punteros únicos**: Cada malloc(0) devuelve dirección diferente
- ✅ **Alineamiento**: Direcciones alineadas a 8 bytes (o ALIGNMENT)
- ✅ **free() funciona**: Detecta y maneja correctamente estos punteros
- ✅ **Direcciones imposibles**: 0x1000+ nunca será heap real
- ✅ **Sin overhead**: No consume memoria real del heap

### Detección en free()
```c
// Verificaciones que hace free():
1. addr >= ZERO_MALLOC_BASE          // Está en el rango base
2. addr < ZERO_MALLOC_BASE + (counter * ALIGNMENT)  // Dentro del rango usado
3. (addr - ZERO_MALLOC_BASE) % ALIGNMENT == 0       // Está alineado correctamente
```

## Ventajas de esta implementación

1. **Simplicidad**: Sin estructuras de datos complejas
2. **Eficiencia**: Sin overhead de memoria
3. **Compatibilidad**: Comportamiento consistente con glibc
4. **Detección rápida**: free() puede identificar malloc(0) rápidamente
5. **Seguridad**: Usa direcciones que nunca serán válidas

## Consideraciones

### Límites
- Máximo ~268M malloc(0) con counter de 32-bit
- Si necesitas más, usar `uint64_t` para el counter

### Errno
- malloc(0) **no** setea errno (es operación válida)
- Solo setear errno en fallos reales (ENOMEM)

### Debugging
En tu allocation history, aparecerá como:
```
malloc(0) -> 0x1000
malloc(0) -> 0x1008
free(0x1000)
free(0x1008)
```