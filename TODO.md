
# TODO

- [ ] Fastbin (MXFAST) - (no coalescing)
- [ ] Smallbin - (coalescing)
- [ ] Unsortedbin
- [ ] Largebin - (coalescing)
- [ ] Coalescing

- [ ] Poner bonito allocate_aligned()
- [ ] heap_find (solo busca en la arena actual)
- [ ] heap_create (reutilizar old heaps)
- [ ] Double free (inactive) - En valloc falla
- [ ] Data-Races

- [ ] Tests perturb
- [ ] Tests malloc_usable_size
- [ ] Tests mallopt, show_alloc_history, show_alloc_mem y show_alloc_mem_ex
- [ ] Eval.sh show_alloc_mem_ex

## Main

- [X] free
- [X] malloc
- [X] realloc
- [X] calloc

## Extra

- [X] reallocarray
- [X] aligned_alloc
- [X] memalign
- [X] posix_memalign
- [X] malloc_usable_size
- [X] valloc
- [X] pvalloc

## Debug

- [X] mallopt
- [X] show_alloc_mem
- [X] show_alloc_mem_ex
- [X] show_alloc_hist

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

### Notes

- mmap (if too large)
- fastbin
- smallbin
- coalescing
- unsorted bin
- largebin
- repeat if fastbin not empty
- top chunk
