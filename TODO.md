# TODO

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

- [ ] ERRNO
- [ ] Data-Races

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
- [ ] your_aligned_allocation()

## Main

- [X] free
- [X] malloc
- [X] realloc
- [X] calloc

## Extra

- [X] reallocarray
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

