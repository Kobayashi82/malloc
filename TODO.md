# TODO

Contador de frees para eliminar heap
Fastbin[0] de 8 bytes es useless

WTF !!
0x7875f44f4008     [FREE] Memory freed of size 16 bytes
0x7875f44f4008   [SYSTEM] Fastbin match for size 16 bytes
0x7875f44f4008   [MALLOC] Allocated 1 bytes
0x7875f44d0000   [SYSTEM] Chunk added to FastBin
0x7875f44d0008     [FREE] Memory freed of size 16 bytes
0x7875f44d0008   [SYSTEM] Fastbin match for size 16 bytes
0x7875f44d0008   [MALLOC] Allocated 1 bytes
0x7875f0ddc008   [SYSTEM] Fastbin match for size 48 bytes
0x7875f0ddc008   [MALLOC] Allocated 40 bytes
0x7875efc8a000   [SYSTEM] Heap of size (69632) allocated
0x7875efc8a008   [MALLOC] Allocated 40 bytes
0x7875efc78000   [SYSTEM] Heap of size (69632) allocated
0x7875efc78008   [MALLOC] Allocated 25 bytes
0x7875efc66000   [SYSTEM] Heap of size (69632) allocated
0x7875efc66008   [MALLOC] Allocated 19 bytes
0x7875efc54000   [SYSTEM] Heap of size (69632) allocated
0x7875efc54008   [MALLOC] Allocated 382 bytes
0x7875efc42000   [SYSTEM] Heap of size (69632) allocated
0x7875efc42008   [MALLOC] Allocated 25 bytes

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
- Double free
- Not allocated
- Invalid pointer

## Otros
- Abort
- Signals
- Print memory
- Print memory hex
- Debug/Trace/Log

## Bonus

- [✓] Multi-Threading with arenas and fork-safe support
- [✓] Debug environment variables and mallopt()
- [✗] Bin management (fastbin, smallbin, largebin & unsortedbin)
- [✗] Allocations history or hexa dump. show_alloc_mem_ex()
- [✗] "Defragment". Coalescing probably

1. **Interoperabilidad con código existente**: Tu implementación necesita manejar punteros que podrían haber sido asignados por otros allocadores de memoria antes de que tu biblioteca fuera cargada.

2. **Prevención de undefined behavior**: Al delegar los punteros desconocidos al allocador original del sistema, evitas comportamientos indefinidos que podrían ocurrir si intentaras liberar memoria que no fue asignada por tu implementación.

3. **Robustez**: Esta estrategia hace que tu implementación sea más robusta ante situaciones donde no tienes control total sobre todas las asignaciones de memoria.

1. Es una medida de seguridad esencial para evitar corrupción de memoria
2. Es una práctica estándar en implementaciones personalizadas de `malloc`
3. Solo se usa como fallback cuando se detecta que el puntero no pertenece a tu allocador
4. No forma parte de la lógica principal de asignación/liberación, sino de la gestión de casos límite
