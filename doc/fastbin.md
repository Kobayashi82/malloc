# Entendiendo los Fastbins en malloc (glibc/ptmalloc2)

Explicación técnica sobre la estructura y funcionamiento de los fastbins, con enfoque en metadatos y punteros.

---

## Tabla de Contenidos
1. [Estructura de un Chunk](#estructura-de-un-chunk)
2. [Encabezado (Chunk Metadata)](#encabezado-chunk-metadata)
   - [Tamaño y Flags](#tamaño-y-flags)
   - [Puntero `fd` (Forward Pointer)](#puntero-fd-forward-pointer)
3. [Funcionamiento de los Fastbins](#funcionamiento-de-los-fastbins)
4. [Ejemplo Visual](#ejemplo-visual)
5. [Ataques y Mitigaciones](#ataques-y-mitigaciones)

---

## Estructura de un Chunk
Cada bloque de memoria gestionado por `malloc` (libre o en uso) tiene un **encabezado** (metadata) antes de los datos del usuario. Ejemplo en memoria:

```
+-----------------------------+
| Tamaño (con flags) | ... | <-- Encabezado (metadata)
+-----------------------------+
| Datos del usuario (si está en uso) |
| o Puntero fd (si está libre) |
+-----------------------------+
```

---

## Encabezado (Chunk Metadata)

### Tamaño y Flags
- **Tamaño del chunk**: Entero almacenado en los primeros 4/8 bytes (32/64 bits). 
- **Flags**: Usan los **3 bits menos significativos** del tamaño:
  | Flag           | Bit | Descripción                                  |
  |----------------|-----|----------------------------------------------|
  | `PREV_INUSE`   | 0   | 1 = El chunk anterior está en uso.           |
  | `IS_MMAPPED`   | 1   | 1 = Chunk asignado directamente con `mmap`.  |
  | `NON_MAIN_ARENA` | 2 | 1 = Pertenece a un arena secundario (hilos). |

- **Ejemplo**:  
  Si un chunk tiene tamaño real 32 bytes (0x20) y el chunk anterior está libre (`PREV_INUSE=0`):
  ```
  Tamaño almacenado = 0x20 | 0x0 = 0x20 (hex)
  ```

### Puntero fd (Forward Pointer)
Solo en chunks libres: Cuando un chunk está en un fastbin, el espacio de usuario se reutiliza para almacenar el puntero fd.

Función: Apunta al siguiente chunk en la lista del fastbin (LIFO).

```c
struct malloc_chunk {
  size_t      size;       // Tamaño + flags
  struct malloc_chunk* fd; 
};
```

Importante:

- En chunks en uso, el espacio después del tamaño es para datos del usuario.
- En fastbins, fd solo enlaza chunks del mismo tamaño.

## Funcionamiento de los Fastbins

### Durante malloc()
Si el tamaño solicitado ≤ global_max_fast (ej: 160 bytes en 64 bits):

- Busca en el fastbin correspondiente al tamaño.
- Si hay chunks libres, toma el último liberado (LIFO).
- Si el fastbin está vacío:
  - Solicita memoria al sistema y divide un nuevo chunk si es necesario.

### Durante free()
- Verifica que el tamaño esté en rango de fastbins.
- Agrega el chunk al inicio del fastbin correspondiente.
- No fusiona chunks adyacentes (evita overhead).

## Ejemplo Visual
Memoria de un Fastbin (32 bytes)

```
HEAD (Fastbin[2]) -> ChunkC (libre) -> ChunkB (libre) -> ChunkA (libre)
```

Encabezados:
- ChunkC.size = 0x20 (32 bytes, PREV_INUSE=1).
- ChunkC.fd = ChunkB (dirección de ChunkB).
- ChunkB.fd = ChunkA.

Asignación de un chunk de 32 bytes:
- malloc(24) → Tamaño ajustado: 32 bytes (24 + encabezado).
- Se extrae ChunkC del fastbin.
- Fastbin queda: HEAD -> ChunkB -> ChunkA.

## Ataques y Mitigaciones

### Técnicas Comunes
**Double Free**:
- Liberar un chunk dos veces para crear un ciclo en la lista.
- Permite escribir en memoria arbitraria modificando fd.

```c
free(ptr);
free(ptr); // Detectado por glibc >= 2.32 (fastbin double-free check)
```

**Heap Overflow**:
- Sobrescribir fd de un chunk libre para apuntar a una dirección controlada.

### Mitigaciones en Glibc
**Double-Free Check**:
```c
if (__builtin_expect (old == p, 0)) // Verifica si el chunk liberado es igual al anterior
  malloc_printerr ("double free or corruption (fasttop)");
```

**Validación de tamaños**:
- El tamaño del chunk debe coincidir con el fastbin seleccionado.

## Conclusión
Los fastbins priorizan velocidad sacrificando fragmentación. Entender su estructura es clave para:

- Optimizar código con muchas asignaciones pequeñas.
- Debuggear errores de heap (corrupción, leaks).
- Desarrollar/Prevenir exploits de memoria.


### Notas Adicionales:
- Para ver en acción: Compila código con `gcc -g` y usa `gdb` con `pwndbg` para inspeccionar chunks (`heap chunks`, `heap bins fast`).
- Ajustes: `mallopt(M_MXFAST, 0)` desactiva fastbins.