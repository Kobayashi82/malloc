# TODO

## Today

- [ ] Alloc_hist
- [X] Perturb
- [X] Corruption
- [X] Errno
- [ ] Free heap
- [ ] Contador de frees para eliminar heap
- [ ] heap_find solo busca en la arena actual
- [X] make(0) cuenta como alloc and free
- [X] Separación (reallocarray -> realloc)

aprintf recibe un argumento de si debe enviar a history allocation.
Y este ya se encarga de llamar a la función de crear el historial con el buffer

## Allocation Order

- mmap (if too large)
- fastbin
- smallbin
- coalescing
- unsorted bin
- largebin
- repeat if fastbin not empty
- top chunk

## Global

- [ ] Data-Races
- [ ] Makefile for Windows

## Chunks

- [ ] Corruption (Magic / Poison)
- [ ] Perturb
- [ ] Coalescing

## Bins

- [X] Fastbin (MXFAST) - (no coalescing)
- [ ] Smallbin - (coalescing)
- [ ] Unsortedbin
- [ ] Largebin - (coalescing)

## Heaps

- [ ] Free heap
- [ ] Contador de frees para eliminar heap
- [ ] heap_find solo busca en la arena actual
- [ ] alloc_aligned()

## Main

- [X] free
- [X] malloc
- [ ] realloc
- [X] calloc

## Extra

- [ ] reallocarray
- [X] aligned_alloc
- [X] memalign
- [ ] posix_memalign
- [X] malloc_usable_size
- [X] valloc
- [X] pvalloc

## Debug

- [X] mallopt
- [X] malloc_stats
- [X] show_alloc_mem
- [X] show_alloc_mem_ex
- [ ] show_alloc_hist

## Documentation

- [ ] Arenas
- [ ] Heaps
- [ ] Header
- [ ] Chunks
- [ ] Bins
- [ ] Alignment 
- [ ] Coalescing
- [ ] Corruption

- [ ] Main
- [ ] Extra
- [ ] Debug

- [ ] Fork-Safe
- [ ] Preload library

