# Algoritmo de Gestión de Memoria Dinámica

## Flujo Principal de Asignación (malloc)

### 1. Verificación Inicial
- Si el tamaño es muy grande → **mmap directo**
- Si no, continuar con el algoritmo de bins

### 2. Búsqueda en Bins (Orden)
1. **Fastbin** - Búsqueda de coincidencia exacta
2. **Smallbin** - Búsqueda de coincidencia exacta
3. **Consolidación de Fastbin** (si no hay coincidencia)
4. **Procesamiento de Unsorted Bin**
5. **Largebin** - Búsqueda progresiva por tamaños
6. **Gestión de Zonas** (Fallback)

## Algoritmo de Liberación (free)

### Estrategia por Tipo de Chunk
- **Fastbin**: Insertar directamente sin coalescing
- **Smallbin/Largebin**: Hacer coalescing con chunks adyacentes antes de insertar

## Consolidación de Fastbin

```
Si no hay coincidencia en fastbin y smallbin:
├── Recorrer todos los chunks de fastbin
├── Mover chunks de fastbin → unsorted bin
├── Durante el movimiento: aplicar coalescing
└── Resultado: unsorted bin con chunks potencialmente fusionados
```

**Nota**: Los chunks fusionados pueden ser de cualquier tamaño (fastbin, smallbin o largebin)

## Procesamiento de Unsorted Bin

```
Mientras haya chunks en unsorted bin:
├── Tomar chunk del unsorted bin
├── ¿Es coincidencia exacta con el request?
│   ├── SÍ → Devolver al usuario
│   └── NO → Redistribuir a bin correspondiente
│       ├── Tamaño fastbin → fastbin
│       ├── Tamaño smallbin → smallbin
│       └── Tamaño large → largebin
└── Continuar hasta vaciar unsorted bin
```

**Importante**: Tras consolidación de fastbin, el unsorted bin puede contener chunks de todos los tamaños.

## Búsqueda en Largebin

```
Para cada largebin (orden ascendente de tamaño):
├── ¿Hay chunk suficientemente grande?
│   ├── SÍ → Realizar split si es necesario
│   │   ├── Parte 1 → Usuario
│   │   └── Parte 2 → Unsorted bin
│   └── NO → Continuar con siguiente largebin
```

## Gestión de Zonas (Fallback)

### Paso 1: Selección de Zona
```
Determinar tipo de zona (TINY/SMALL):
├── Buscar zona con > 90% de uso
├── Si no existe → Buscar zona con < 80% fragmentación
└── Si no existe → Crear nueva zona
```

### Paso 2: Asignación desde Top Chunk
```
En la zona seleccionada:
├── ¿Top chunk tiene espacio suficiente?
│   ├── SÍ → Asignar desde top chunk
│   └── NO → Buscar otra zona o crear nueva
```

## Problema Identificado: Violación de Límites de Zona

### Descripción del Problema
```
Situación problemática:
├── Zona TINY (límite: 512 bytes)
├── Chunks se van fusionando → Chunk de 1024 bytes
├── Request de 800 bytes → Podría usar chunk de 1024 bytes
└── PROBLEMA: Chunk de 800 bytes en zona TINY (debería ir en SMALL)
```

### El Problema del Coalescing Cross-Zone

Cuando se hace coalescing de chunks en fastbin:
- Chunk A (64 bytes) + Chunk B (128 bytes) + Chunk C (256 bytes) = Chunk fusionado (448 bytes)
- Si el chunk fusionado queda en zona TINY pero luego crece a >512 bytes por más fusiones
- Un request de 600 bytes podría asignarse desde este chunk en zona TINY
- **Violación**: Chunk de 600 bytes viviendo en zona TINY

### Posibles Soluciones

#### Opción 1: Verificación con heap_find()
```c
// Pseudocódigo
if (chunk_size > zone_limit) {
    heap_t *heap = heap_find(chunk_ptr);
    if (heap->type != appropriate_zone_type) {
        // Rechazar este chunk y buscar en otra parte
        continue_search();
    }
}
```

**Pros**: Solución limpia y correcta  
**Contras**: Mayor procesamiento por verificación

#### Opción 2: Metadata en Headers
```c
typedef struct chunk_header {
    size_t size;
    zone_type_t origin_zone;  // Nuevo campo
    // ... otros campos
} chunk_header_t;
```

**Pros**: Verificación rápida O(1)  
**Contras**: Mayor overhead de memoria

#### Opción 3: Restricción de Coalescing
```
Política de fusión:
├── Solo fusionar si chunk resultante <= límite de zona original
├── Si excede límite → No fusionar
└── Mantener chunks pequeños en sus zonas correspondientes
```

**Pros**: Mantiene integridad de zonas  
**Contras**: Menor eficiencia de coalescing, más fragmentación

#### Opción 4: Migración de Chunks
```
Cuando chunk fusionado excede límite:
├── Crear nuevo chunk en zona apropiada
├── Copiar datos si es necesario
├── Liberar chunk original
└── Actualizar referencias
```

**Pros**: Máxima flexibilidad  
**Contras**: Muy complejo, costoso computacionalmente

#### Opción 5: Zonas Flexibles
```
Permitir que zonas contengan chunks de otros tamaños:
├── Zona TINY puede contener chunks > 512 si provienen de coalescing
├── Al asignar, verificar si el tamaño solicitado "pertenece" a la zona
└── Mantener estadísticas para optimizar futuras zonas
```

**Pros**: Flexible, aprovecha mejor el coalescing  
**Contras**: Menos predecible, zonas menos "puras"

## Recomendación

La **Opción 1 (heap_find())** parece la más equilibrada:
- Mantiene la integridad conceptual del sistema de zonas
- El costo computacional adicional es mínimo comparado con operaciones de mmap
- Solución limpia y mantenible
- Evita violaciones de límites de zona sin sacrificar el coalescing

## Flujo Completo Resumido

```
malloc(size):
├── size > MMAP_THRESHOLD → mmap()
├── Buscar en fastbin[size]
├── Buscar en smallbin[size]
├── Si no hay coincidencia:
│   ├── Consolidar fastbin → unsorted bin (con coalescing)
│   ├── Procesar unsorted bin:
│   │   ├── Coincidencia exacta → devolver
│   │   └── Redistribuir a bins apropiados
│   ├── Buscar en largebin (con split si necesario)
│   └── Fallback: gestión de zonas + top chunk
└── Verificar integridad de zona antes de devolver
```
