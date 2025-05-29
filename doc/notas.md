## FRAGMENTACION

La fragmentación se mide típicamente como la diferencia entre el espacio libre total y el bloque libre más grande. Si tienes 1000 bytes libres pero el bloque más grande es solo 100 bytes, tienes alta fragmentación porque no puedes satisfacer solicitudes grandes a pesar de tener espacio. También se puede medir por el número de bloques libres separados o el ratio entre espacio utilizable vs desperdiciado.

| Situación                               | Acción                                  |
| --------------------------------------- | --------------------------------------- |
| `malloc` cabe aunque zona esté picada   | Usa la zona                             |
| `malloc` no cabe pero hay espacio total | Coalescing **solo si fragmentada**      |
| Fragmentada pero malloc cabe            | Sigue usando                            |
| Muy fragmentada, malloc no cabe         | Coalescing → nueva zona si falla        |
| Coalescing reciente/no eficaz           | No repetir enseguida, limita frecuencia |


## M_CHECK_ACTION

- 0	Ignore error conditions; continue execution (with undefined results).
- 1	Print a detailed error message and continue execution.
- 2	Abort the program.
- 3	Print detailed error message, stack trace, and memory mappings, and abort the program.
- 5	Print a simple error message and continue execution.
- 7	Print simple error message, stack trace, and memory mappings, and abort the program.

## DEFINICIONES

### Arena

- Estructura compartida entre hilos que contiene HEAPs y BINs

### Heap
- Region de memoria contigua que se divide en CHUNKs

### Chunk

- Porción de memoria que envuelve un BLOCK y contiene METADATA

### Meta-Data

- Informacion sobre una BLOCK y que se encuentra antes y despues de un BLOCK

### Block

- Region dentro de un CHUNK y perteneciente a un HEAP donde se guardan datos. Es lo que devuelve malloc y ibera free

### Bin

- Listas enlazadas con informacion a CHUNKs libres. Se usa para la reutilizacion de espacios de memoria

### TCache

- Array de listas enlazadas con informacion a CHUNKs libres. Se usa para para la reutilizacion de espacios de memoria, pero cada hilo tiene su propio TCACHE

## RESUMEN DE DISEÑO

### Mapeo global de arenas

- Array fijo t_range[255] por arena
- Cada t_range tiene start[255] y end[255] para zonas
- Mutex global para acceder a estructura de mapeo

### Optimización tcache por hilo

- Thread-local cache con rangos de zonas del hilo actual
- free() busca primero en tcache (sin mutex)
- Si no encuentra → búsqueda global (con mutex)

### Gestión cross-thread

- Free en arena ajena: usa mutex de esa arena
- Si zona queda vacía tras free cross-thread → marca flag "revisar zona"
- Solo el hilo propietario limpia su tcache y libera zonas

### Estrategia de coalescing

- Al liberar: sin coalescing inmediato (free rápido)
- Cuando malloc no encuentra en bins:

### Evaluar fragmentación

- Si fragmentación alta → coalescing completo + retry bins
- Si fragmentación baja → crear nueva zona directamente

### Limpieza de zonas vacías

- Durante coalescing: detectar y liberar zonas completamente vacías
- Actualizar bins tras coalescing
- Limpiar referencias del tcache propio

### OTROS

- mcheck(3), mtrace(3)
