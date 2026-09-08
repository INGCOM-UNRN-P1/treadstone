# Librería `p1_test`: Framework Avanzado de Pruebas Unitarias para C99

Biblioteca *header-only* de pruebas unitarias para lenguaje C (estándar C99 con soporte POSIX), desarrollada para la cátedra de **Programación 1** (Universidad Nacional de Río Negro).

Diseñada para ser simple, didáctica, resiliente a fallos y completamente inmune a directivas de preprocesador como `NDEBUG` o `DEBUG`.

---

## 🚀 Características Principales (v2.0.0)

* **Inmune a Banderas de Compilación:** A diferencia de `<assert.h>`, las aserciones **no se eliminan** al compilar con `-DNDEBUG` ni con `-DDEBUG`.
* **Aislamiento de Fallos y Rescate de Señales:** 
  * `setjmp`/`longjmp`: Un assert fallido aborta únicamente el test en curso sin terminar abruptamente el ejecutable.
  * Captura de `SIGSEGV` y `SIGFPE`: Fallos de segmentación o división por cero en código de alumnos son atrapados y reportados como test fallido sin voltear la suite.
  * Watchdog de Timeout: Interrumpe automáticamente bucles infinitos (`while(1)`) mediante `alarm` y `SIGALRM`.
* **Opciones CLI Avanzadas:**
  * `-k, --filter <texto>`: Ejecuta solo los tests que coincidan con la subcadena.
  * `-f, --fail-fast`: Detiene la suite en el primer fallo.
  * `-q, --quiet`: Modo compacto con progreso en una línea (`.` y `F`).
  * `-t, --timeout <seg>`: Límite de tiempo por test.
  * `--tap`: Salida estándar compatible con Test Anything Protocol v13.
  * `--no-color`: Desactivación de colores (autodetectada si la salida no es terminal interactiva o si está seteada `NO_COLOR`).
* **Catálogo Extendido de Aserciones Didácticas:**
  * Booleanas: `ASSERT_TRUE`, `ASSERT_FALSE`
  * Enteros y Rangos: `ASSERT_INT_EQ`, `ASSERT_INT_NE`, `ASSERT_INT_LT`, `ASSERT_INT_LE`, `ASSERT_INT_GT`, `ASSERT_INT_GE`, `ASSERT_INT_BETWEEN`
  * Enteros sin signo: `ASSERT_UINT_EQ`, `ASSERT_UINT_NE`
  * Números reales: `ASSERT_DOUBLE_EQ`, `ASSERT_DOUBLE_NE` (tolerancia absoluta), `ASSERT_DOUBLE_NEAR_REL` (tolerancia relativa)
  * Cadenas: `ASSERT_STR_EQ`, `ASSERT_STR_NE`, `ASSERT_STR_CASE_EQ` (case-insensitive), `ASSERT_STR_CONTAINS` (seguras con punteros `NULL`)
  * Punteros: `ASSERT_PTR_NULL`, `ASSERT_PTR_NOT_NULL`, `ASSERT_PTR_EQ`, `ASSERT_PTR_NE`
  * Arreglos: `ASSERT_ARRAY_INT_EQ` (reporta índice y discrepancias)
  * Memoria binaria: `ASSERT_MEM_EQ` (con volcado hexadecimal de contexto)
  * Salida estándar: `ASSERT_STDOUT_EQ` (captura consola de funciones con `printf`)
* **Herramientas de Ciclo de Vida y Organización:**
  * `BEFORE_EACH` y `AFTER_EACH` (garantizado tras aborto de asserts)
  * `SUBCASE` para secciones dentro de un test
  * `SKIP_TEST` y `TEST_SKIP` para desarrollo TDD incremental
  * `RUN_TEST_SEEDED` para pruebas reproducibles con `rand()`
* **Auditoría de Memoria:** Target `make memcheck` integrado con Valgrind (`--leak-check=full --error-exitcode=1`).

---

## 📖 Manual de Uso

Podés consultar el manual exhaustivo con ejemplos de código de cada aserción en:
* [Manual de Uso Exhaustivo](docs/manual_uso.md)

---

## 📁 Estructura del Repositorio

* **`include/p1_test.h`**: Cabecera completa del framework.
* **`docs/manual_uso.md`**: Manual de uso completo y referencia de la API.
* **`tests/`**: Pruebas de autoverificación del propio framework:
  * `test_p1_test.c`: Verifica las 20 mejoras QoL, aserciones tipadas, hooks y CLI.
  * `test_p1_failures.c`: Verifica el formato y diagnóstico de fallos de aserción.
  * `test_p1_signals.c`: Verifica la recuperación ante `SIGSEGV` y `SIGFPE`.
* **`ejemplos/proyecto_tp/`**: Proyecto completo de ejemplo basado en `plantilla-TP`, con `tp.sh`, `Makefile` y `ejercicios/ejercicio1` probado con `p1_test`.
* **`Makefile`**: Orquestador principal de compilación, ejecución de pruebas y verificación de memoria.

---

## 🛠️ Comandos de Compilación y Prueba

Compilar y ejecutar todas las pruebas del framework:
```bash
make test
```

Verificar persistencia de aserciones bajo `-DNDEBUG`:
```bash
make test-ndebug
```

Auditar memoria con Valgrind:
```bash
make memcheck
```

Probar el proyecto de ejemplo basado en `plantilla-TP`:
```bash
make test-ejemplo
make run-ejemplo
```

Limpiar archivos generados:
```bash
make clean
```
