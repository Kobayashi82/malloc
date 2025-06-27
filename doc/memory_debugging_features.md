# Funciones de Debugging de Memoria

## Allocation History

### ¿Qué es?
Un **registro cronológico** de todas las operaciones de memoria que realiza el programa. Funciona como un "log de auditoría" de malloc/free.

### Información que registra

**Por cada operación:**
- Función llamada (malloc, free, realloc, etc.)
- Timestamp de cuándo ocurrió
- Puntero devuelto/liberado
- Bytes solicitados vs bytes realmente asignados
- Stack trace del código que hizo la llamada
- ID del hilo donde ocurrió

**Estado global:**
- Memoria activa en cada momento
- Pico máximo de memoria utilizada
- Número de allocaciones sin liberar

### Para qué sirve

**Detectar problemas:**
- Memory leaks (allocaciones nunca liberadas)
- Double-free (liberar el mismo puntero dos veces)
- Use-after-free (usar memoria ya liberada)

**Análisis de rendimiento:**
- Patrones de uso de memoria
- Análisis de fragmentación
- Identificar qué código consume más memoria

**Debugging:**
- Ver exactamente qué pasó antes de un crash
- Entender flujos complejos de allocaciones

### Consideraciones de implementación

**Problema de consumo de memoria:**
- Registrar todas las operaciones consume muchos bytes
- Para aplicaciones con millones de allocaciones puede ser prohibitivo

**Solución condicional:**
- Solo activar si `DEBUG=1` o variable de entorno específica
- En producción el overhead sería demasiado alto

**Estrategia de almacenamiento con mmap:**
- Empezar con tamaño base (ej: 1MB)
- Cuando se llena → crear nuevo mmap del doble de tamaño
- Copiar datos al nuevo espacio → liberar el anterior
- Ventajas del mmap:
  - Puede crecer a tamaños enormes sin problemas
  - El OS maneja la memoria virtual eficientemente
  - Posibilidad de mapear a archivo para persistencia

**Tipos de implementación:**
- **Completo:** Guarda todo - perfecto para debugging pero consume mucha memoria
- **Rolling:** Solo mantiene los últimos N eventos - balance funcionalidad/consumo
- **Lazy:** Solo registra detalles cuando detecta problemas - mínimo overhead
- **Sampling:** Solo registra fracción de operaciones - para análisis estadístico

---

## Print Memory

### Propósito
Mostrar un resumen del estado actual de la memoria gestionada por el allocator.

### Qué mostrar

**Información general:**
- Número total de allocaciones activas
- Bytes totales en uso
- Pico máximo de memoria alcanzado

**Estadísticas por tamaño:**
- Cuántos bloques de diferentes rangos (ej: <64 bytes, 64-1KB, 1KB-1MB, >1MB)
- Distribución de tamaños más comunes

**Top allocaciones:**
- Los 10 bloques más grandes actualmente activos
- Información de dónde se allocaron (si hay stack trace disponible)

### Qué NO mostrar
- **Todos los punteros individuales** - pueden ser miles y sería inmanejable
- Contenido de la memoria - para eso está print_memory_hex

### Ejemplo de salida conceptual:
```
=== Memory Status ===
Active allocations: 1,247
Total memory in use: 45.2 MB
Peak memory usage: 52.8 MB

Size distribution:
  < 64 bytes: 892 blocks (12.3 KB)
  64-1KB: 284 blocks (156.7 KB) 
  1KB-1MB: 68 blocks (8.9 MB)
  > 1MB: 3 blocks (36.0 MB)

Largest active blocks:
  12.0 MB at 0x7f1234000000 (allocated from main.c:142)
  8.5 MB at 0x7f1298000000 (allocated from parser.c:89)
  ...
```

---

## Print Memory Hex

### Propósito
Mostrar el contenido hexadecimal de una región específica de memoria gestionada por el allocator.

### Sintaxis
```
print_memory_hex <address> [length]
```

### Comportamiento según parámetros

**Si no se especifica length:**
- Buscar el puntero en el registro de allocaciones
- Mostrar el chunk completo desde la dirección dada hasta el final del chunk
- Si el puntero no está al inicio del chunk, mostrar desde el offset hasta el final

**Si se especifica length:**
- Mostrar el mínimo entre el length solicitado y los bytes disponibles en el chunk
- Si se solicita más de lo disponible, mostrar warning: "Requested X bytes, but only Y bytes available in chunk"

### Manejo de punteros que no están al inicio del chunk

**El problema:**
- Si dan un puntero en medio de un chunk, hay que determinar a qué chunk pertenece
- No se puede saber cuánto queda del chunk sin esta información

**Solución propuesta:**
1. **Determinar a qué chunk pertenece** el puntero
   - Recorrer chunks hasta encontrar: `inicio_chunk <= puntero < inicio_chunk + tamaño_chunk`

2. **Validaciones:**
   - Si está en top chunk → error ("Cannot show top chunk contents")
   - Si el chunk está liberado → error ("Chunk is freed, cannot show contents") 
   - Si no pertenece a ningún chunk → error ("Pointer not in any managed chunk")

3. **Una vez localizado:**
   - Calcular offset: `offset = puntero_dado - inicio_chunk`
   - Calcular bytes restantes: `bytes_restantes = tamaño_chunk - offset`
   - Mostrar: `min(tamaño_solicitado, bytes_restantes)` desde el puntero dado

### Ejemplo de salida
```
Chunk: 0x7fff12340000-0x7fff12340100 (256 bytes, allocated)
Showing: 64 bytes from offset +48 (208 bytes remaining in chunk)

0x7fff12340030: 48 65 6c 6c 6f 20 57 6f 72 6c 64 00 00 00 00 00  |Hello World.....|
0x7fff12340040: ff ff ff ff 12 34 56 78 9a bc de f0 00 00 00 00  |.....4Vx........|
0x7fff12340050: 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f 10  |................|
0x7fff12340060: aa bb cc dd ee ff 00 11 22 33 44 55 66 77 88 99  |........"3DUfw..|
```

### Casos edge adicionales

**Puntero no reconocido:**
- "Error: Pointer 0x... not found in allocation records"

**Puntero ya liberado:**
- "Error: Pointer 0x... was freed (use allocation history to see when)"

**Tamaño solicitado mayor que chunk:**
- Mostrar solo lo disponible + warning explicativo

### Casos de uso
- **Debugging:** Ver contenido de un bloque corrupto
- **Verificación:** Comprobar inicialización correcta de memoria
- **Análisis:** Examinar patrones de datos para detectar problemas