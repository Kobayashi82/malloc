# TODO

- borrar *.o y make no funciona bien

## Variables de entorno y mallopt solo asignación de valores.

- Arena como se establece para thread local (en cualquier función de allocation)
- Free sin first malloc no hace nada
- Si first_alloc no permitir cambios en g_manager.options

## Estructuras:
- fastbin (MXFAST)
- smallbin
- unsortedbin
- largebin

## Cosas
- Alignment
- Coalescing
- Fragmentation
- Free zone
- Perturb
- Validacion de header

## Free
- Double free prevention
- Not allocated
- Invalid pointer

## Otros
- Abort
- Signals

- Print memory
- Print memory hex
- Debug/Trace/Log

## Requirements (just for the check)

- [ ] Implement `malloc`
