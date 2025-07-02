sizeof(t_chunk) + size + (alignment - 1)

Busco chunk de ese tamaño.
Si no es el primer chunk, obtengo el chunk anterior.
Determino donde debe empezar el puntero.
Retrocedo el sizeof(t_chunk)
Establezco el tamaño del chunk anterior y el tamaño de prev_size del chunk nuevo.
El tamaño del chunk nuevo también.

Problema. El primer chunk no tiene chunk anterior. Y no puedo anadir el espacio extra a ningun sitio.

Como podria hacer?
Si es el primer chunk tendria que crear uno nuevo vacio.

Pero igualmente, si me piden un tamaño grande, hago un mmap directamente. Pero claro, el header tiene que ir primero y si dejo espacio antes del inicio del header, no coincidiría el chunk con el inicio de mmap...
Si es mmap desplazo y en heap->padding pongo el espacio extra hasta el inicio del heap.
En tiny y small, si chunk es igual a heap->ptr, creo un chunk nuevo antes. Pero tiene que ser de sizeof(t_chunk) * 2
Asi que no se si al tamaño de alignment anadir sizeof(t_chunk) * 2 para tener un margen extra.
Como ahora necesito añadir heap->padding, de quizas uint8_t tendria que ajustar la cantidad de heaps por info  page

-

vcalloc y pvcalloc (alineacion a 4096) y alineaciones que sean > CHUNK_SMALL
Se tiene que reservar size + sizeof(t_chunk) alineado a alignment.
Ea decir, si size es 4096, pues se tiene que alinear a 4096 * 2
Como son LARGE tengo que hacer heap->padding= alignment - sizeof(t_chunk) y ptr estaria a alignment - sizeof(t_chunk)
Cuando se elimine el heap, se hace heap->ptr - heap->padding. Y como size, heap->size + padding.

Para tiny y small, se suma al tamaño total necesitado un minimo para crear otro chunk.
Por ejemplo, en tiny se haria por ejemplo 32 + 16 bytes y en small, 512 + 64. O algo asi
Asi cuando creo un chunk desplazado, lo que hay detras del chunk, se fusiona con el chunk anterior (libre o no) o se crea un chunk nuevo vacio.
El unico cambio es a la hora de eliminar chunks grandes. Lo demas no deberia de afectar al resto... Creo

Bueno, si fusiono con un chunk de fastbin libre, tendria que hacerle un unlink y luego un link para actualizar los bins... De hecho, creo que tendria que actualizar todos los bins independientemente del tamaño del chunk... Vamos un unlink y un link simplemente

Si el chunk actual se puede agrandar al necesario, simplemente devuelvo el mismo puntero.
Si hay que reducir, calculo la alineación que sirve para el tamaño y creo otro chunk (si es de un tamaño aceptable, si no, se lo queda igualmente.
En el caso de que haga falta espacio, compruebo si puede expandirse a la derecha, si el siguiente es libre:
Si siguen siendo pequeño, compruebo si el siguiente también está libre. Si entre todos los libres tengo, entonces realizo las fusiones de chunks y spliteo si es necesario.
Si llego al top chunk necesito splitear el top chunk aunque sea al minimo de 16 bytes y size 0.
Si no sirve nada de eso. Nuevo chunk, copiar y liberar.


# TODO

- [X] Fastbin (MXFAST) - (no coalescing)
- [ ] Smallbin - (coalescing)
- [ ] Unsortedbin
- [ ] Largebin - (coalescing)
- [ ] Coalescing
- [ ] alloc_aligned()
- [ ] heap_find (solo busca en la arena actual)
- [ ] Data-Races

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
- [ ] posix_memalign
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

