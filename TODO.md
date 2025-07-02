
# TODO

Si el chunk actual se puede agrandar al necesario, simplemente devuelvo el mismo puntero.
Si hay que reducir, calculo la alineación que sirve para el tamaño y creo otro chunk (si es de un tamaño aceptable, si no, se lo queda igualmente).
En el caso de que haga falta espacio, compruebo si puede expandirse a la derecha, si el siguiente es libre:
Si siguen siendo pequeño, compruebo si el siguiente también está libre. Si entre todos los libres tengo, entonces realizo las fusiones de chunks y spliteo si es necesario.
Si llego al top chunk necesito splitear el top chunk aunque sea al minimo de 16 bytes y size 0.
Si no sirve nada de eso. Nuevo chunk, copiar y liberar.

- [X] Fastbin (MXFAST) - (no coalescing)
- [ ] Smallbin - (coalescing)
- [ ] Unsortedbin
- [ ] Largebin - (coalescing)
- [ ] Coalescing

- [ ] heap_find (solo busca en la arena actual)

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
- [ ] realloc
- [X] calloc

## Extra

- [ ] reallocarray
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

