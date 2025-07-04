# Documentación del Allocator

## Introducción

Este allocator gestiona la memoria de tu programa de forma inteligente y segura. Su objetivo principal es entregar memoria cuando la necesites y recuperarla cuando la liberes, pero también detectar errores comunes que pueden provocar crashes o vulnerabilidades.

## Componentes Principales

### Arenas

Imagina las arenas como "oficinas" donde se gestiona la memoria. Cada arena tiene sus propios heaps, sus propios bins y sus propias estadísticas. Cuando tu programa usa múltiples hilos, varios hilos pueden trabajar con la misma arena, pero cada hilo sabe a qué arena pertenece gracias a una variable especial llamada `tcache`.

La idea es que si tienes muchos hilos trabajando al mismo tiempo, no todos compitan por la misma "oficina". Esto hace que el programa sea más rápido porque los hilos no se bloquean tanto entre sí. Cada arena tiene su propio mutex para que cuando varios hilos la usen, no se pisen entre ellos.

### Heaps

Los heaps son como "archivadores" dentro de cada oficina donde realmente se guarda la memoria que usas. Hay tres tipos diferentes porque no tiene sentido usar la misma estrategia para guardar una tarjeta de visita que para guardar un expediente completo:

**TINY** es para cuando necesitas poquita memoria, como guardar un número o una palabra pequeña. Estos archivadores pueden contener muchos trocitos pequeños de memoria, todos organizados juntos en sus casillas.

**SMALL** es para cuando necesitas más memoria, como guardar una frase o una estructura de datos mediana. También contiene muchos trocitos, pero cada casilla es más grande.

**LARGE** es especial: cuando pides mucha memoria de una vez, el allocator no la mete en un archivador compartido, sino que va directamente al sistema operativo y pide un espacio exclusivo para ti. Es como si en lugar de guardar algo en un archivador, dedicaras una sala entera solo para ese documento.

### Header (Heap Header)

El heap header es como el "inventario" de cada archivador. Te dice cuántos espacios de memoria hay disponibles en esa página y dónde encontrar la información de cada archivador. Es la forma que tiene el allocator de no perderse entre todos los archivadores que ha creado.

Cada página de memoria puede contener información sobre varios archivadores, y el header lleva la cuenta de cuántos hay y dónde están. Si una página se llena, apunta a la siguiente página donde hay más información.

### Chunks

Los chunks son los "paquetes" de memoria que realmente usas en tu programa. Cada chunk tiene una pequeña etiqueta al principio (el header) que dice cuánto espacio tienes y si está bien o se ha corrompido.

Lo interesante es que el allocator es muy astuto: guarda información útil en el espacio que ya no usas. Cuando liberas un chunk, utiliza parte de tu espacio anterior para crear una lista enlazada con otros chunks libres. Es como reutilizar las cajas vacías para hacer una cadena que te ayude a encontrar más cajas libres después.

También hay un truco con el chunk anterior: si está libre, su tamaño se guarda justo antes de tu chunk actual. Esto permite al allocator moverse hacia atrás en la memoria cuando necesita fusionar chunks.

### Bins

Los bins son como "estanterías organizadas" donde se guardan los chunks que has liberado, listos para ser reutilizados. Cada estantería está pensada para un tipo específico de uso:

**Fast bins** son como una pila de platos: el último que pones es el primero que coges. Están pensados para cuando liberas y vuelves a pedir memoria muy rápido, así que no se molestan en fusionar chunks con sus vecinos. Simplemente los guardan tal como están para entregártelos súper rápido.

**Small bins** son más organizados: cada estantería tiene chunks del mismo tamaño exacto, y funcionan como una cola normal (primero en entrar, primero en salir). Cuando liberas un chunk aquí, sí se fusiona con sus vecinos si también están libres.

**Unsorted bin** es como un "buzón temporal" donde van todos los chunks recién liberados antes de ser clasificados. Es una zona de paso donde el allocator decide después dónde colocar realmente cada chunk.

**Large bins** están reservados para chunks grandes, organizados por rangos de tamaño en lugar de tamaños exactos.

### Alignment (Alineación)

La alineación es importante porque los procesadores trabajan mejor cuando los datos están colocados en direcciones "bonitas". Es como cuando ordenas libros en una estantería: si los pones alineados, es más fácil cogerlos.

El allocator se asegura de que cuando te da memoria, esté alineada correctamente para tu procesador. En sistemas de 64 bits, esto significa que las direcciones terminan en números que son múltiplos de 16. Algunas funciones especiales pueden pedir alineaciones más estrictas, y el allocator las respeta.

### Coalescing (Fusión)

El coalescing es como "juntar terrenos vacíos" para hacer parcelas más grandes. Cuando liberas memoria, el allocator mira si los chunks vecinos también están libres. Si es así, los fusiona en un chunk más grande.

Esto es importante porque evita la fragmentación: sin coalescing, podrías tener muchos espacios pequeños libres pero ninguno lo suficientemente grande para una asignación grande. Es como tener muchos huecos pequeños en un parking pero no poder aparcar un autobús.

Los fast bins son una excepción: no se fusionan inmediatamente porque están diseñados para velocidad. Pero cuando es necesario, el allocator puede hacer una "limpieza general" y fusionar todo lo que se pueda.

### Corruption (Detección de Corrupción)

La detección de corrupción es como tener "sellos de garantía" en tu memoria. El allocator coloca números especiales en lugares estratégicos y los verifica regularmente para detectar si algo ha ido mal.

Cuando tienes un chunk activo, lleva un "número mágico" en su header. Si alguien escribe fuera de los límites de su memoria y machaca este número, el allocator se da cuenta y puede avisarte o parar el programa.

Cuando liberas un chunk, el allocator cambia ese número mágico por un "patrón de veneno". Si más tarde alguien intenta usar esa memoria liberada, o si intentas liberar el mismo chunk dos veces, el allocator detecta que el patrón no es el esperado y sabe que algo está mal.

Este sistema te ayuda a encontrar bugs típicos como escribir fuera de los límites de un array, usar memoria después de liberarla, o liberar la misma memoria dos veces.

## Cómo Funciona Todo Junto

Cuando pides memoria, el allocator primero mira qué tamaño necesitas y decide si usar un archivador TINY, SMALL o ir directamente a LARGE. Luego busca en los bins apropiados de tu arena para ver si tiene algo del tamaño adecuado ya preparado.

Si encuentra algo, te lo da inmediatamente. Si no, puede que tenga que crear un nuevo chunk desde el archivador, o incluso crear un nuevo archivador si todos están llenos.

Cuando liberas memoria, el allocator verifica que todo esté correcto (números mágicos, etc.), decide si puede fusionar con vecinos, y coloca el chunk en el bin apropiado para futura reutilización.

Todo el sistema está diseñado para ser rápido en el caso común, pero también robusto para detectar errores y mantener la memoria bien organizada.

# Documentación de Funciones de Memoria

## Funciones Principales

### free()
```c
void free(void *ptr);
```
Libera un bloque de memoria previamente asignado por malloc, calloc o realloc. Si `ptr` es NULL, no hace nada.

### malloc()
```c
void *malloc(size_t size);
```
Asigna un bloque de memoria de `size` bytes. La memoria no está inicializada. Retorna un puntero al bloque asignado o NULL si falla.

### realloc()
```c
void *realloc(void *ptr, size_t size);
```
Redimensiona un bloque de memoria previamente asignado. Si `ptr` es NULL, actúa como malloc. Si `size` es 0, actúa como free. Preserva el contenido original hasta el mínimo entre el tamaño anterior y el nuevo.

### calloc()
```c
void *calloc(size_t nmemb, size_t size);
```
Asigna memoria para un array de `nmemb` elementos de `size` bytes cada uno. La memoria se inicializa a cero. Retorna un puntero al bloque asignado o NULL si falla.

## Funciones Adicionales

### reallocarray()
```c
void *reallocarray(void *ptr, size_t nmemb, size_t size);
```
Versión segura de realloc que detecta desbordamiento de enteros al multiplicar `nmemb * size`. Equivalente a `realloc(ptr, nmemb * size)` pero con protección contra overflow.

### aligned_alloc()
```c
void *aligned_alloc(size_t alignment, size_t size);
```
Asigna memoria alineada a un múltiplo de `alignment`. El `alignment` debe ser una potencia de 2. Cumple con el estándar C11.

### memalign()
```c
void *memalign(size_t alignment, size_t size);
```
Función GNU que asigna memoria alineada. Similar a aligned_alloc pero con diferente orden de parámetros.

### posix_memalign()
```c
int posix_memalign(void **memptr, size_t alignment, size_t size);
```
Función POSIX para asignación alineada. Almacena el puntero resultante en `*memptr`. Retorna 0 en éxito o un código de error.

### malloc_usable_size()
```c
size_t malloc_usable_size(void *ptr);
```
Retorna el tamaño útil de un bloque de memoria asignado. Puede ser mayor que el tamaño solicitado originalmente debido a la alineación interna.

### valloc()
```c
void *valloc(size_t size);
```
Asigna memoria alineada al tamaño de página del sistema. Función obsoleta, se recomienda usar aligned_alloc.

### pvalloc()
```c
void *pvalloc(size_t size);
```
Como valloc, pero redondea el tamaño al múltiplo de página más cercano. También obsoleta.

## Funciones de Debug

### mallopt()
```c
int mallopt(int param, int value);
```
Configura parámetros del asignador de memoria. Los parámetros disponibles son:

- **M_MXFAST**: Tamaño máximo de chunks rápidos (por defecto 80)
- **M_MIN_USAGE**: Uso mínimo de memoria en porcentaje (por defecto 10)
- **M_CHECK_ACTION**: Acción ante errores de corrupción (por defecto 0)
- **M_PERTURB**: Valor para perturbar memoria libre (por defecto 0)
- **M_ARENA_TEST**: Número de arenas de prueba (por defecto 2 en 32-bit, 8 en 64-bit)
- **M_ARENA_MAX**: Máximo número de arenas (por defecto 0 = sin límite)
- **M_DEBUG**: Nivel de debug (por defecto 0)
- **M_LOGGING**: Activar logging (por defecto 0)

**Nota importante**: Los cambios no están permitidos después de la primera asignación de memoria.

### show_alloc_mem()
```c
void show_alloc_mem(void);
```
Muestra información sobre el estado actual de la memoria asignada. Proporciona un resumen de los bloques en uso.

### show_alloc_mem_ex()
```c
void show_alloc_mem_ex(void);
```
Versión extendida de show_alloc_mem que proporciona información más detallada sobre la memoria asignada.

### show_alloc_hist()
```c
void show_alloc_hist(void);
```
Muestra el historial de asignaciones y liberaciones de memoria realizadas por el programa.

## Variables de Entorno

Las siguientes variables de entorno pueden configurar el comportamiento del asignador:

- **MALLOC_MXFAST_**: Equivalente a M_MXFAST
- **MALLOC_MIN_USAGE_**: Equivalente a M_MIN_USAGE
- **MALLOC_CHECK_**: Equivalente a M_CHECK_ACTION
- **MALLOC_PERTURB_**: Equivalente a M_PERTURB
- **MALLOC_ARENA_TEST**: Equivalente a M_ARENA_TEST
- **MALLOC_ARENA_MAX**: Equivalente a M_ARENA_MAX
- **MALLOC_DEBUG**: Equivalente a M_DEBUG
- **MALLOC_LOGGING**: Equivalente a M_LOGGING
- **MALLOC_LOGFILE**: Especifica el archivo de log (por defecto "auto")

## Notas de Implementación

- Todas las funciones principales (malloc, free, realloc, calloc) mantienen compatibilidad con glibc
- Las funciones de debug proporcionan capacidades de diagnóstico adicionales
- La configuración mediante variables de entorno se lee al inicializar el gestor de memoria
- Los cambios de configuración mediante mallopt requieren que no se haya realizado ninguna asignación previa