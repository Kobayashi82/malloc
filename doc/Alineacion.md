# Explicación: Redondeo a PAGE_SIZE con Operaciones de Bits

## El Código Original
```c
size_t total_size = (size + sizeof(t_chunk) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
```

## El Problema
Necesitamos 1024 bytes, pero el sistema operativo solo asigna memoria en páginas completas de 4096 bytes. Queremos calcular cuántas páginas necesitamos.

## Los Números en Binario

```
PAGE_SIZE = 4096 = 0001 0000 0000 0000 (binario)
                   ↑
                   Bit 12 activado (2^12 = 4096)

PAGE_SIZE - 1 = 4095 = 0000 1111 1111 1111 (binario)
                       ↑   ↑-----------↑
                       |   Los últimos 12 bits
                       |   están todos en 1
                       Bit 12 está en 0
```

## Paso a Paso con el Cálculo

### Datos iniciales:
```
size = 1024 bytes
sizeof(t_chunk) = 64 bytes (ejemplo típico)
```

### Paso 1: Sumar todo
```
1024 + 64 + 4095 = 5183 bytes
```

### Paso 2: Ver 5183 en binario
```
5183 = 0001 0100 0011 1111 (binario)
       ↑    ↑-----------↑
       |    Estos bits representan
       |    el "resto" dentro de la página
       Páginas completas
```

### Paso 3: Crear la máscara
```
~(PAGE_SIZE - 1) = ~4095 = 1111 0000 0000 0000 (binario)
                           ↑    ↑-----------↑
                           |    Los últimos 12 bits
                           |    están en 0
                           El resto en 1
```

### Paso 4: Aplicar la máscara (operación AND)
```
   5183 = 0001 0100 0011 1111
& ~4095 = 1111 0000 0000 0000
   ────   ───────────────────
   4096 = 0001 0000 0000 0000
```

## ¿Qué Pasó Visualmente?

### Memoria antes del redondeo:
```
[----Página 1 (4096)----][--1087 bytes usados--]
```

### Memoria después del redondeo:
```
[----Página 1 (4096)----][----Página 2 (4096)----]
                         ↑
                         Solo usamos 1087 bytes,
                         pero reservamos la página completa
```

## ¿Por Qué Sumamos PAGE_SIZE - 1?

### Sin sumar 4095:
- 1088 bytes necesarios
- `1088 & ~4095 = 0` ← **¡MAL!** Nos daría 0 páginas

### Sumando 4095:
- `1088 + 4095 = 5183`
- `5183 & ~4095 = 4096` ← **¡BIEN!** Una página completa

> La suma de 4095 "empuja" cualquier número que no sea múltiplo exacto de 4096 hacia el siguiente múltiplo.

## Tabla de Ejemplos

| Bytes  | (+) 4095 | (&) Resultado | Páginas |
|--------|----------|---------------|---------|
| 1      | 4096     | 4096          | 1       |
| 1024   | 5119     | 4096          | 1       |
| 4096   | 8191     | 8192          | 2       |
| 4097   | 8192     | 8192          | 2       |
| 8192   | 12287    | 12288         | 3       |

## Resultado Final

- **Bytes solicitados:** 1088
- **Bytes asignados:** 4096 (una página completa)
- **"Desperdicio":** 3008 bytes (pero es inevitable, el SO funciona así)

## ¿Por Qué Funciona Este Truco?

El truco funciona porque **4096 es una potencia de 2**, y todos sus múltiplos tienen los últimos 12 bits en cero. La máscara simplemente "borra" esos bits, garantizando un múltiplo perfecto.

### Múltiplos de 4096 en binario:
```
4096  = 0001 0000 0000 0000
8192  = 0010 0000 0000 0000
12288 = 0011 0000 0000 0000
16384 = 0100 0000 0000 0000
```

Todos terminan en 12 ceros, por eso la máscara `1111 0000 0000 0000` funciona perfectamente.