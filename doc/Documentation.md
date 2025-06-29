# Material for the documentation

## Delegate Free

Se evita comportamientos indefinidos que podrían ocurrir si intentaras liberar memoria que no fue asignada por tu implementación y que podrían haber sido asignados por otros allocadores de memoria antes de que tu biblioteca fuera cargada.

- Es una medida de seguridad esencial para evitar corrupción de memoria.
- Es una práctica estándar en implementaciones personalizadas de `malloc`.
- Solo se usa como fallback cuando se detecta que el puntero no pertenece a tu allocador.
- No forma parte de la lógica principal de asignación/liberación, sino de la gestión de casos límite.

## Delegate Realloc

- Al igual que cuando se delega un puntero a liberar (free), en realloc puede ser necesario liberar un puntero que no ha sido asignado por nuestro malloc y en ese caso, se delega al realloc nativo.

- Como alternativa, podria asignarse un chunk o espacio de memoria para el realloc y delegar la liberación del puntero si no es nuestro. Quizas use este modo por defecto y evito delegar realloc.

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
