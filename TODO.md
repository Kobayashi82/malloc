
# TODO

- [X] Fastbin (MXFAST) - (no coalescing)
- [ ] Smallbin - (coalescing)
- [ ] Unsortedbin
- [ ] Largebin - (coalescing)
- [ ] Coalescing

- [ ] arreglar allocate_aligned()
- [ ] heap_find (solo busca en la arena actual)
- [ ] valloc falla con el Double free (inactive)
- [ ] Añadir a tests perturb
- [ ] Logging con debug mejor
- [ ] Double free (inactive)
- [ ] Perturb en realloc
- [ ] Show alloc history if logging
- [ ] Perturb en allocation_aligned?
- [X] Mover /lib a la raiz
- [ ] Documentar .h
- [ ] Terminar de arreglar tests
- [ ] Malloc_usable en tests
- [ ] Test para mallopt, show alloc history, show alloc mem y show alloc mem ex
- [ ] Anadir show alloc mem ex a eval.sh
- [X] Quitar nativos
- [ ] Free allocation para evitar llamar a free
- [X] _pid.log en logfile always
- [X] Make borra los logs en /tmp/malloc_*.log
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

