# Tablas de Configuración del Malloc

## 1. M_PERTURB - Llenado de Memoria

| Función | Valor != 0 | Valor == 0 | Notas |
|---------|------------|------------|-------|
| **malloc()** | ✅ Llenar área usuario con perturb | ❌ Sin llenado | Memoria sin inicializar |
| **calloc()** | ❌ **Siempre** llenar con 0 | ❌ **Siempre** llenar con 0 | Garantía de calloc() |
| **realloc()** | ✅ Área nueva con perturb | ❌ Área nueva sin inicializar | Solo espacio adicional |
| **free()** | ✅ Primero perturb, luego FD/BK | ❌ Solo metadatos FD/BK | Ayuda debug uso post-free |

---

## 2. MALLOC_CHECK - Manejo de Errores

| Valor | Comportamiento en Error | Logging | Casos de Error |
|-------|------------------------|---------|----------------|
| **0** (default) | ❌ **ABORT** | ✅ (si debug=1) | • Double-free<br>• Free inválido<br>• Heap corruption |
| **1** | ✅ **CONTINÚA** | ✅ (si debug=1) | • Ignora operación<br>• Programa sigue |
| **2** | ✅ **CONTINÚA** | ❌ **Silencioso** | • Sin warnings<br>• Sin abort |

**⚠️ EXCEPCIÓN:** Fallos de mmap/sbrk **siempre abortan** (independiente del valor)

---

## 3. Sistema de Logging (DEBUG + LOGGING + LOGFILE)

| DEBUG | LOGGING | LOGFILE | Resultado | Output |
|-------|---------|---------|-----------|--------|
| **0** | * | * | ❌ Sin logging | Ninguno |
| **1** | **0** (default) | * | ✅ Logging básico | **stderr** |
| **1** | **1** | ❌ NULL/vacío | ⚠️ Fallback | **stderr** + warning |
| **1** | **1** | ✅ Ruta válida | ✅ Logging a archivo | **archivo** |
| **1** | **1** | ❌ Ruta inválida | ⚠️ Fallback | **stderr** + error msg |

### Variables de Entorno de Ejemplo:
```bash
# Solo debug a stderr
MALLOC_DEBUG=1

# Debug + archivo de log
MALLOC_DEBUG=1 MALLOC_LOGGING=1 MALLOC_LOGFILE="/tmp/malloc.log"

# Debug silencioso (sin logging)
MALLOC_DEBUG=0
```

### Comportamiento de Checks:
- **Corruption checks**: Siempre activos (independiente de debug)
- **Error logging**: Solo si debug=1
- **Abort behavior**: Controlado por malloc_check