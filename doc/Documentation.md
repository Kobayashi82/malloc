# Malloc

## Introducción

Este allocator gestiona la memoria de tu programa de forma inteligente y segura. Su objetivo principal es entregar memoria cuando la necesites y recuperarla cuando la liberes, pero también detectar errores comunes que pueden provocar crashes o vulnerabilidades.

## Componentes Principales

### Arenas

Las arenas son áreas independientes. Imaginalas como "oficinas" donde se gestiona la memoria. Cada arena tiene sus propios heaps, sus propios bins y sus propias estadísticas. Cuando tu programa usa múltiples hilos, a cada hilo se le asigna una arena. Varios hilos pueden trabajar con la misma arena y cada hilo sabe a qué arena pertenece gracias a una variable especial llamada `tcache`, que es única para cada hilo y guarda un puntero a la arena que tiene asignada.

La idea es que si tienes muchos hilos trabajando al mismo tiempo, no todos compitan por la misma "oficina". Esto hace que el programa sea más rápido porque los hilos no se bloquean tanto entre sí. Cada arena tiene su propio mutex para que cuando varios hilos la usen, no se pisen entre ellos.

### Heaps

Los heaps son como "archivadores" dentro de cada oficina donde realmente se guarda la memoria que usas. Hay tres tipos diferentes:

**TINY**

Es para cuando necesitas poca memoria. Estos archivadores pueden contener muchos trocitos pequeños de memoria.

**SMALL**

Es para cuando necesitas más memoria. También contiene muchos trocitos, pero de tamaños más grande.

**LARGE**

Es especial: cuando pides mucha memoria de una vez, el allocator no la mete en un archivador compartido, sino que va directamente al sistema operativo y pide un espacio exclusivo para ti. Es como si se creara un archivador experesamente esa solicitud, sin compartirla con nadie más.

**HEAP HEADER**

El heap header es un tipo de heap especia. Cada heap header ocupa un espacio de memoria de una página completa.
Es como el "inventario" de cada archivador. Guarda información de cada heap creado. Es la forma que tiene el allocator de no perderse entre todos los archivadores que ha creado.

### Chunks

Los chunks son los "paquetes" de memoria que realmente usas en tu programa. Cada chunk tiene una pequeña etiqueta al principio (header) que dice cuánto espacio tienes disponible y si está bien o se ha corrompido.

Lo interesante es que el allocator es muy astuto: guarda información útil en el espacio que ya no usas. Cuando liberas un chunk, utiliza parte de tu espacio anterior para crear una lista enlazada con otros chunks libres. Es como reutilizar los documentos que ya no necesitas para anotar donde está el siguiente documento que no se necesita. Creando así una cadena de chunks libres y disponibles para volver a ser asignados.

También hay un truco con el chunk anterior: si está libre, su tamaño se guarda justo antes de tu chunk actual. Esto permite al allocator moverse hacia atrás en la memoria cuando necesita fusionar chunks.

### Bins

Los bins son como grupos de "listas" organizadas donde se anotan los chunks que has liberado, listos para ser reutilizados.

**TOP CHUNK**

Aunque no es un bin, en caso de no encontrar un chunk válido para reutilizar, se procede a coger el espacio necesario del top chunk. Es cúal es un chunk especial al final del heap que contiene todo el espacio restante del heap que no se ha gragmentado aún.

### Alineación

La alineación es importante porque los procesadores trabajan mejor. 

El allocator se asegura de que cuando te da memoria, esté alineada correctamente para tu procesador. En sistemas de 64 bits, esto significa que las direcciones terminan en números que son múltiplos de 16. Algunas funciones especiales pueden pedir alineaciones más estrictas, y el allocator las respeta.

### Coalescing (Fusión)

El coalescing es como unir varios documentos de un archivador creando un documento más grande. Cuando liberas memoria, el allocator mira si los chunks vecinos también están libres. Si es así, los fusiona en un chunk más grande.

Esto es importante porque evita la fragmentación: sin coalescing, podrías tener muchos espacios pequeños libres pero ninguno lo suficientemente grande para una asignación grande. Es como tener muchos huecos pequeños en un parking pero no poder aparcar un autobús.

### Detección de Corrupción

La detección de corrupción es como tener "sellos de garantía" en tu memoria. El allocator coloca números especiales en el header de los chunks y los verifica regularmente para detectar si algo ha ido mal.

Cuando tienes un chunk activo, lleva un "número mágico" en su header. Si alguien escribe fuera de los límites de su memoria y machaca este número, el allocator se da cuenta y puede avisarte o parar el programa.

Cuando liberas un chunk, el allocator cambia ese número mágico por un "patrón de veneno". Si más tarde alguien intenta usar esa memoria liberada, o si intentas liberar el mismo chunk dos veces, el allocator detecta que el patrón no es el esperado y sabe que algo está mal.

## Cómo Funciona Todo Junto

Cuando pides memoria, el allocator primero mira qué tamaño necesitas y decide si usar un archivador TINY, SMALL o ir directamente a LARGE. Luego busca en los bins apropiados de tu arena para ver si tiene algo del tamaño adecuado ya preparado.

Si encuentra algo, te lo da inmediatamente. Si no, puede que tenga que crear un nuevo chunk desde el top chunk, o incluso crear un nuevo archivador si todos están llenos.

Cuando liberas memoria, el allocator verifica que todo esté correcto (números mágicos, etc.), decide si puede fusionar con vecinos, y coloca el chunk en el bin apropiado para futura reutilización.

Todo el sistema está diseñado para ser rápido en el caso común, pero también robusto para detectar errores y mantener la memoria bien organizada.

## Funciones Principales

### FREE

Libera un bloque de memoria previamente asignado. Si `ptr` es NULL, no hace nada.

```c
  void free(void *ptr);

  ptr   – pointer returned by malloc/calloc/realloc.

  • On success: the memory block pointed to by ptr is deallocated.
  • On failure: undefined behavior.

Notes:
  • ptr can be NULL. In that case, free() does nothing.
  • After freeing, ptr becomes invalid. Do not access the memory after calling free().
  • It is the user's responsibility to ensure that ptr points to a valid allocated block.
```

### MALLOC

Asigna un bloque de memoria de `size` bytes. La memoria no está inicializada. Retorna un puntero al bloque asignado o NULL si falla.

```c
  void *malloc(size_t size);

  size  – the number of bytes to allocate.

  • On success: returns a pointer aligned for the requested size.
  • On failure: returns NULL and sets errno to:
      – ENOMEM: not enough memory.

Notes:
  • If size == 0:
      – returns a unique pointer that can be freed.;
```

### REALLOC

Redimensiona un bloque de memoria previamente asignado. Si `ptr` es NULL, actúa como malloc. Si `size` es 0, actúa como free. Preserva el contenido original hasta el mínimo entre el tamaño anterior y el nuevo.

```c
  void *realloc(void *ptr, size_t size);

  ptr   – pointer returned by malloc/calloc/realloc.
  size  – the new size of the memory block, in bytes.

  • On success: returns a pointer aligned for the requested size (may be the same as ptr or a new location).
  • On failure: returns NULL, original block unchanged and sets errno to:
      – ENOMEM: not enough memory.

Notes:
  • If size == 0:
      – ptr != NULL → returns NULL and free ptr.
      – ptr == NULL → behaves like malloc(size).
```


### CALLOC

Asigna memoria para un array de `nmemb` elementos de `size` bytes cada uno. La memoria se inicializa a cero. Retorna un puntero al bloque asignado o NULL si falla.

```c
  void *calloc(size_t nmemb, size_t size);

  nmemb – number of elements.
  size  – the size of each element, in bytes.

  • On success: returns a pointer aligned for the requested size with all bytes initialized to zero.
  • On failure: returns NULL and sets errno to:
      – ENOMEM: not enough memory.

 Notes:
  • If nmemb == 0 || size == 0:
      – returns a unique pointer that can be freed.
  • Memory is zero-initialized, meaning all bits are set to 0.
```

## Funciones Adicionales

### REALLOC ARRAY

Versión segura de realloc que detecta desbordamiento de enteros al multiplicar `nmemb * size`. Equivalente a `realloc(ptr, nmemb * size)` pero con protección contra overflow.

```c
  void *reallocarray(void *ptr, size_t nmemb, size_t size);

  ptr   – pointer returned by malloc/calloc/realloc.
  nmemb – number of elements.
  size  – the size of each element, in bytes.

  • On success: returns a pointer aligned for nmemb * size bytes (may be the same as ptr or a new location).
  • On failure: returns NULL, original block unchanged and sets errno to:
      – ENOMEM: not enough memory.

Notes:
  • If nmemb == 0 || size == 0:
      – ptr != NULL → returns NULL, leaves ptr valid (not an error).
      – ptr == NULL → returns a unique pointer you can free (malloc(0)).
```

### ALIGNED ALLOC

Asigna memoria alineada a un múltiplo de `alignment`. El `alignment` debe ser una potencia de 2.

```c
  void *aligned_alloc(size_t alignment, size_t size);

  alignment – the required alignment in bytes (must be a power of two and a multiple of sizeof(void *)).
  size      – the size of the memory block to allocate, in bytes (must be a multiple of alignment).

  • On success: returns a pointer aligned to the specified alignment.
  • On failure: returns NULL and sets errno to:
      – EINVAL: alignment not valid.
      – ENOMEM: not enough memory.

Notes:
  • When size is not naturally a multiple of alignment, round it up before calling aligned_alloc().
```

### MEMALIGN

Función GNU que asigna memoria alineada. Similar a aligned_alloc pero con diferente orden de parámetros.

```c
  void *memalign(size_t alignment, size_t size);

  alignment – alignment in bytes (must be a power of two and a multiple of sizeof(void *)).
  size      – the size of the block to allocate, in bytes.

  • On success: returns a pointer aligned to alignment bytes.
  • On failure: returns NULL and sets errno to:
      – EINVAL: alignment not valid.
      – ENOMEM: not enough memory.

Notes:
  • memalign() is non‑standard; prefer posix_memalign() or aligned_alloc() for portable code.
```

### POSIX MEMALIGN

Función POSIX para asignación alineada. Almacena el puntero resultante en `*memptr`. Retorna 0 en éxito o un código de error.

```c
  int posix_memalign(void **memptr, size_t alignment, size_t size);

  memptr    – a pointer to the memory block.
  alignment – the required alignment in bytes (must be a power of two and a multiple of sizeof(void *)).
  size      – the size of the memory block to allocate, in bytes.

  • On success: returns 0 and *memptr receives the allocated memory block.
  • On failure: leaves *memptr unchanged and returns an error code:
      – EINVAL: alignment not valid.
      – ENOMEM: not enough memory.

Notes:
  • When failure occurs, *memptr remains unspecified, and you should not free() it.
  • If alignment or size is invalid, errno is not set; check the return code for errors.
```

### MALLOC USABLE SIZE

Retorna el tamaño útil de un bloque de memoria asignado. Puede ser mayor que el tamaño solicitado originalmente debido a la alineación interna.

```c
  size_t malloc_usable_size(void *ptr);

  ptr – pointer returned by malloc/calloc/realloc.

How it works:
  • Allocators often reserve more memory than requested, due to alignment or internal metadata.
  • When you request n bytes, the actual allocation may be larger.
  • malloc_usable_size(ptr) tells you how many usable bytes are available in that block.

  • On success: returns the number of usable bytes available in the allocated block, which may be larger than the size originally requested.
  • On failure: if the pointer is NULL or invalid, the behavior is undefined.

Notes:
  • The returned size is always >= the requested size (unless ptr is NULL).
  • Do NOT rely on the extra space — it's allocator-specific and may not be portable.
  • Passing an invalid or non-malloced pointer results in undefined behavior.
```

### VALLOC

Asigna memoria alineada al tamaño de página del sistema. Función obsoleta, se recomienda usar aligned_alloc.

```c
  void *valloc(size_t size);

  size – the size of the memory block to allocate, in bytes.

  • On success: returns a pointer aligned to the system page size.
  • On failure: returns NULL and sets errno to:
      – ENOMEM: not enough memory.

Notes:
  • valloc() is non‑standard and obsolete; prefer posix_memalign() or aligned_alloc() for portable code.
```

## PVALLOC

Como valloc, pero redondea el tamaño al múltiplo de página más cercano. También obsoleta.

```c
  void *pvalloc(size_t size);

  size – the size of the memory block to allocate, in bytes. Rounds up to the next multiple of page size

  • On success: returns a pointer aligned to the system page size.
  • On failure: returns NULL and sets errno to:
      – ENOMEM: not enough memory.

Notes:
  • pvalloc() is non‑standard and obsolete; prefer posix_memalign() or aligned_alloc() for portable code.
```

## Funciones de Debug

### SHOW ALLOCATION MEMORY

Muestra información sobre el estado actual de la memoria asignada y proporciona un resumen de los bloques en uso.

```c
  void show_alloc_mem(void);

  • On success: prints a detailed report of all current allocations.

Notes:
  • Output is written to file descriptor 2 (stderr).
  • Heaps are sorted before printing, and grouped by arena.
```

### SHOW ALLOCATION MEMORY EXTENDED

Versión extendida de show_alloc_mem que proporciona información más detallada sobre la memoria asignada.

```c
  void show_alloc_mem_ex(void *ptr, size_t offset, size_t length);

  ptr    – pointer to allocated memory to examine.
  offset – starting byte offset within the chunk (0 = start of user data).
  length – number of bytes to display (0 = show from offset to end of chunk).

  • On success: displays detailed memory information and hexadecimal dump.
  • On failure: prints error message to stderr and returns.

Notes:
  • Shows chunk header (metadata) and user data separately.
  • Automatically truncates length if it exceeds remaining bytes.
  • Hexdump format adapts to system alignment (8 or 16 bytes per row).
```

### SHOW ALLOCATION HISTORY

Muestra el historial de asignaciones y liberaciones de memoria realizadas por el programa.

```c
  void show_alloc_history(void);

  • On call: writes the recorded log buffer to file descriptor 2 (stderr).
  • The log contains a chronological trace of:
      – Allocations, frees and errors.
```

### MALLOPT

Configura parámetros del asignador de memoria.

```c
  int mallopt(int param, int value);

  param – option selector (M_* constant).
  value – value assigned to the option.

  • On success: returns 1.
  • On failure: returns 0 and sets errno to:
      – EINVAL: unsupported param or invalid value.

Supported params:
  • M_ARENA_MAX (-8)       (1-64/128):  Maximum number of arenas allowed.
  • M_ARENA_TEST (-7)         (1-160):  Number of arenas at which a hard limit on arenas is computed.
  • M_PERTURB (-6)          (0-32/64):  Sets memory to the PERTURB value on allocation, and to value ^ 255 on free.
  • M_CHECK_ACTION (-5)         (0-2):  Behaviour on abort errors (0: abort, 1: warning, 2: silence).
  • M_MIN_USAGE (3)           (0-100):  Heaps under this usage % are skipped (unless all are under).
  • M_DEBUG (7)                 (0-1):  Enables debug mode (1: errors, 2: system).
  • M_LOGGING (8)               (0-1):  Enables logging mode (1: to file, 2: to stderr).

Notes:
  • Changes are not allowed after the first memory allocation.
  • If both M_DEBUG and M_LOGGING are enabled:
      – uses $MALLOC_LOGFILE if defined, or fallback to "/tmp/malloc_[PID].log"
```

## Variables de Entorno

Las siguientes variables de entorno pueden configurar el comportamiento del allocator:

| Variable de entorno      | Equivalente interno       | Descripción                             |
|--------------------------|---------------------------|-----------------------------------------|
| **MALLOC_ARENA_MAX**     | `M_ARENA_MAX`             | Límite máximo de arenas                 |
| **MALLOC_ARENA_TEST**    | `M_ARENA_TEST`            | Umbral de prueba para eliminar arenas   |
| **MALLOC_PERTURB_**      | `M_PERTURB`               | Rellena el heap con un patrón           |
| **MALLOC_CHECK_**        | `M_CHECK_ACTION`          | Acción ante errores de memoria          |
| **MALLOC_MIN_USAGE_**    | `M_MIN_USAGE`             | Umbral mínimo de uso para optimización  |
| **MALLOC_DEBUG**         | `M_DEBUG`                 | Activa el modo debug                    |
| **MALLOC_LOGGING**       | `M_LOGGING`               | Habilita logging                        |
| **MALLOC_LOGFILE**       | *(ruta de archivo)*       | Archivo de log (por defecto `"auto"`)   |


## Notas de Implementación

- Todas las funciones principales y adicionales mantienen compatibilidad con glibc
- Las funciones de debug proporcionan capacidades de diagnóstico adicionales
- La configuración mediante variables de entorno se lee al inicializar el gestor de memoria
- Los cambios de configuración mediante mallopt requieren que no se haya realizado ninguna asignación previa
