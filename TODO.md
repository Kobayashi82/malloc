# TODO

- [✗] Cambiar heaps y arenas a un modo single page
- [✗] Contador de frees para eliminar heap (quizas mejor cuando se haga coalescing se limpia zonas vacias. Por investigar)
- [✗] Fastbin[0-1] de 8 y 16 bytes es useless

## Allocation Order

- mmap (if too large)
- fastbin
- smallbin
- coalescing
- unsorted bin
- largebin
- repeat if fastbin not empty
- top chunk

(see Allocation.md)

## Bins

- [✗] Fastbin (MXFAST) - (no coalescing)
- [✗] Smallbin - (coalescing)
- [✗] Unsortedbin
- [✗] Largebin - (coalescing)

## Cosas

- [✗] Coalescing
- [✗] Perturb
- [✗] Free heap

## Main

- [✗] Malloc
- [✗] Calloc
- [✗] Realloc
- [✗] Free
- [✓] Mallopt

## Free

- [✓] Not aligned
- [✓] LARGE (double free is delegated to native free, if not, is middle chunk)
- [✓] Double free
- [✓] Top chunk
- [✓] Middle chunk
- [✓] Not allocated (delegated to native free)

## Extra

- [✗] Signals
- [✓] Multi-Threading with arenas and fork-safe support
- [✓] Debug environment variables and mallopt()
- [✗] Debug logging to file
- [✗] Bin management (fastbin, smallbin, largebin & unsortedbin)
- [✗] Coalescing
- [✗] Print memory
- [✗] Print memory hex
- [✗] Allocations history

## Delegate Free

Se evita comportamientos indefinidos que podrían ocurrir si intentaras liberar memoria que no fue asignada por tu implementación y que podrían haber sido asignados por otros allocadores de memoria antes de que tu biblioteca fuera cargada.

- Es una medida de seguridad esencial para evitar corrupción de memoria.
- Es una práctica estándar en implementaciones personalizadas de `malloc`.
- Solo se usa como fallback cuando se detecta que el puntero no pertenece a tu allocador.
- No forma parte de la lógica principal de asignación/liberación, sino de la gestión de casos límite.

## Delegate Realloc

- Al igual que cuando se delega un puntero a liberar (free), en realloc puede ser necesario liberar un puntero que no ha sido asignado por nuestro malloc y en ese caso, se delega al realloc nativo.

- Como alternativa, podria asignarse un chunk o espacio de memoria para el realloc y delegar la liberación del puntero si no es nuestro. Quizas use este modo por defecto y evito delegar realloc.
