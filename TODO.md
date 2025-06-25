# TODO

Contador de frees para eliminar heap
Fastbin[0-1] de 8 y 16 bytes es useless

## Allocation
- mmap (if too large)
- fastbin
- smallbin
- coalescing
- unsorted bin
- largebin
- top chunk

(see Allocation.md)

## Free
- fastbin (no coalescing)
- smallbin (coalescing)
- largebin (coalescing)

(see Allocation.md)

## Estructuras:
- fastbin (MXFAST)
- smallbin
- unsortedbin
- largebin

## Cosas
- Alignment
- Coalescing
- Fragmentation
- Free heap
- Perturb
- Header validation

## Free

- [✗] Not aligned
- [✗] LARGE (double free is delegated to native free, if not, is middle chunk)
- [✗] Double free
- [✗] Top chunk
- [✗] Middle chunk
- [✗] Not allocated (delegated to native free)

## Bonus

- [✗] Signals
- [✓] Multi-Threading with arenas and fork-safe support
- [✓] Debug environment variables and mallopt()
- [✗] Debug logging to file
- [✗] Bin management (fastbin, smallbin, largebin & unsortedbin)
- [✗] Coalescing
- [✗] Print memory
- [✗] Print memory hex
- [✗] Allocations history

## Delegate Realloc

Se evita comportamientos indefinidos que podrían ocurrir si intentaras liberar memoria que no fue asignada por tu implementación y que podrían haber sido asignados por otros allocadores de memoria antes de que tu biblioteca fuera cargada.

- Es una medida de seguridad esencial para evitar corrupción de memoria.
- Es una práctica estándar en implementaciones personalizadas de `malloc`.
- Solo se usa como fallback cuando se detecta que el puntero no pertenece a tu allocador.
- No forma parte de la lógica principal de asignación/liberación, sino de la gestión de casos límite.

## Delegate Free

Se evita comportamientos indefinidos que podrían ocurrir si intentaras liberar memoria que no fue asignada por tu implementación y que podrían haber sido asignados por otros allocadores de memoria antes de que tu biblioteca fuera cargada.

- Es una medida de seguridad esencial para evitar corrupción de memoria.
- Es una práctica estándar en implementaciones personalizadas de `malloc`.
- Solo se usa como fallback cuando se detecta que el puntero no pertenece a tu allocador.
- No forma parte de la lógica principal de asignación/liberación, sino de la gestión de casos límite.
