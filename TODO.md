
# TODO

- [X] Fastbin (MXFAST) - (no coalescing)
- [ ] Smallbin - (coalescing)
- [ ] Unsortedbin
- [ ] Largebin - (coalescing)
- [ ] Coalescing

- [ ] arreglar allocate_aligned()
- [ ] heap_find (solo busca en la arena actual)
- [ ] valloc falla con el Double free (inactive)

- [ ] Documentar .h

- [ ] Añadir a tests perturb
- [ ] Terminar de arreglar tests
- [ ] Malloc_usable en tests
- [ ] Test para mallopt, show alloc history, show alloc mem y show alloc mem ex
- [ ] Anadir show alloc mem ex a eval.sh
- [ ] Data races

- mmap (if too large)
- fastbin
- smallbin
- coalescing
- unsorted bin
- largebin
- repeat if fastbin not empty
- top chunk

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

