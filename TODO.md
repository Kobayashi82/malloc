# TODO

## Bins

- [X] Fastbin (MXFAST) - (no coalescing)
- [ ] Smallbin - (coalescing)
- [ ] Unsortedbin
- [ ] Largebin - (coalescing)
- [ ] Coalescing

### Allocation Order

- mmap (if too large)
- fastbin
- smallbin
- coalescing
- unsorted bin
- largebin
- repeat if fastbin not empty
- top chunk

## Heaps

- [ ] Free heap (contador de frees para eliminar heap)
- [ ] heap_find (solo busca en la arena actual)
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
- [X] show_alloc_hist

## Global

- [ ] Data-Races
- [ ] Makefile for Windows

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

