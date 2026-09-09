# Treadstone (`p1_test`): Framework de Pruebas Unitarias para C11

Biblioteca de pruebas unitarias para lenguaje C (estándar C11 con soporte POSIX), desarrollada para la cátedra de **Programación 1** (Universidad Nacional de Río Negro).

Inspirada conceptualmente en el programa de operaciones de precisión (*The Bourne Identity*, 2002), **Treadstone** provee evaluación rigurosa, resiliencia ante excepciones fatales en tiempo de ejecución y compatibilidad nativa como biblioteca estática (`libp1_test.a`) administrada por el gestor de dependencias del proyecto.

---

## 🎯 Características Principales (v2.0.0)

* **Compilación Estándar y Estructurada:** Compila como biblioteca estática `libp1_test.a` en `build/` y raíz, compatible con el gestor [`manage.sh`](file:///home/mrtin/dev/p1/practicas/plantillas/lib_test/manage.sh) e integrable vía [`library.spec`](file:///home/mrtin/dev/p1/practicas/plantillas/lib_test/library.spec) y [`library.json`](file:///home/mrtin/dev/p1/practicas/plantillas/lib_test/library.json).
* **Persistencia ante Banderas de Preprocesador:** Las aserciones permanecen activas tanto bajo `-DNDEBUG` como bajo `-DDEBUG`.
* **Aislamiento de Fallos y Rescate de Señales:**
  * Control de flujo no local (`setjmp`/`longjmp`): Un fallo de aserción interrumpe únicamente el test en curso sin terminar la suite completa.
  * Rescate de señales de hardware (`SIGSEGV`, `SIGFPE`, `SIGILL`, `SIGBUS`): Captura fallos de segmentación y divisiones por cero en el código evaluado, registrándolos como pruebas fallidas.
  * Watchdog de Timeout: Interrumpe ejecuciones colgadas en bucles infinitos mediante temporizador POSIX (`alarm` y `SIGALRM`).
* **Control por Línea de Comandos (CLI):**
  * `-k, --filter <texto>`: Ejecuta únicamente los tests cuyo identificador coincida con la subcadena.
  * `-f, --fail-fast`: Detiene la suite inmediatamente tras el primer fallo.
  * `-q, --quiet`: Salida compacta en una sola línea (`.` para acierto, `F` para fallo, `S` para omitido).
  * `-t, --timeout <seg>`: Configura el tiempo límite en segundos por prueba.
  * `--tap`: Genera salida compatible con Test Anything Protocol v13.
  * `--no-color`: Desactiva secuencias de escape ANSI.
* **Catálogo de Aserciones Didácticas:**
  * Booleanas: `ASSERT_TRUE`, `ASSERT_FALSE`
  * Enteros y Rangos: `ASSERT_INT_EQ`, `ASSERT_INT_NE`, `ASSERT_INT_LT`, `ASSERT_INT_LE`, `ASSERT_INT_GT`, `ASSERT_INT_GE`, `ASSERT_INT_BETWEEN`
  * Enteros sin signo: `ASSERT_UINT_EQ`, `ASSERT_UINT_NE`
  * Coma flotante: `ASSERT_DOUBLE_EQ`, `ASSERT_DOUBLE_NE` (tolerancia absoluta), `ASSERT_DOUBLE_NEAR_REL` (tolerancia relativa)
  * Cadenas de texto: `ASSERT_STR_EQ`, `ASSERT_STR_NE`, `ASSERT_STR_CASE_EQ` (insensible a mayúsculas), `ASSERT_STR_CONTAINS`
  * Punteros: `ASSERT_PTR_NULL`, `ASSERT_PTR_NOT_NULL`, `ASSERT_PTR_EQ`, `ASSERT_PTR_NE`
  * Arreglos: `ASSERT_ARRAY_INT_EQ` (señala el índice y los valores discordantes)
  * Memoria binaria: `ASSERT_MEM_EQ` (con volcado hexadecimal de bytes en discrepancia)
  * Salida estándar: `ASSERT_STDOUT_EQ` (captura y compara streams de salida de `printf`)
* **Ciclo de Vida y Organización:**
  * `BEFORE_EACH` y `AFTER_EACH` para inicialización y limpieza por test.
  * `SUBCASE` para estructurar fases dentro de una misma prueba.
  * `SKIP_TEST` y `TEST_SKIP` para pruebas pendientes o salteadas condicionalmente.
  * `RUN_TEST_SEEDED` para evaluaciones determinísticas con semilla de `rand()`.
* **Auditoría de Memoria:** Verificación con Valgrind integrada (`make memcheck`).

---

## 📁 Estructura del Repositorio

* [`src/p1_test.c`](file:///home/mrtin/dev/p1/practicas/plantillas/lib_test/src/p1_test.c): Implementación de símbolos compilables para `libp1_test.a`.
* [`include/p1_test.h`](file:///home/mrtin/dev/p1/practicas/plantillas/lib_test/include/p1_test.h): Cabecera principal del framework, macros de aserción y orquestador.
* [`include/p1_arrays.h`](file:///home/mrtin/dev/p1/practicas/plantillas/lib_test/include/p1_arrays.h): Comprobaciones sobre arreglos numéricos, ordenamiento y contención.
* [`include/p1_files.h`](file:///home/mrtin/dev/p1/practicas/plantillas/lib_test/include/p1_files.h): Aserciones sobre sistema de archivos (existencia, contenido de texto y volcados binarios).
* [`include/p1_stdio.h`](file:///home/mrtin/dev/p1/practicas/plantillas/lib_test/include/p1_stdio.h): Captura bidireccional y simulación de streams (`stdin`, `stdout`, `stderr`).
* [`include/tda/contracts.h`](file:///home/mrtin/dev/p1/practicas/plantillas/lib_test/include/tda/contracts.h): Macro-contratos de pre/postcondiciones e invariantes.
* [`library.spec`](file:///home/mrtin/dev/p1/practicas/plantillas/lib_test/library.spec) / [`library.json`](file:///home/mrtin/dev/p1/practicas/plantillas/lib_test/library.json): Metadatos y definición de exportaciones para el gestor de dependencias.
* [`manage.sh`](file:///home/mrtin/dev/p1/practicas/plantillas/lib_test/manage.sh): Script de administración de la biblioteca (build, test, rename, info).
* [`tests/`](file:///home/mrtin/dev/p1/practicas/plantillas/lib_test/tests): Batería de autoverificación del framework (rescate de señales, fallos, timeouts, hooks y contratos).
* [`ejemplos/proyecto_tp/`](file:///home/mrtin/dev/p1/practicas/plantillas/lib_test/ejemplos/proyecto_tp): Proyecto completo de integración basado en `plantilla-TP`.
* [`docs/manual_uso.md`](file:///home/mrtin/dev/p1/practicas/plantillas/lib_test/docs/manual_uso.md): Referencia exhaustiva de la API con ejemplos.

---

## 🛠️ Comandos de Compilación y Prueba

Compilar la biblioteca estática y la suite de pruebas:
```bash
make all
```

Ejecutar todas las pruebas unitarias:
```bash
make test
```

Verificar persistencia de aserciones bajo `-DNDEBUG` y `-DDEBUG`:
```bash
make test-ndebug
make test-debug
```

Auditar memoria con Valgrind:
```bash
make memcheck
```

Probar el proyecto de integración de ejemplo:
```bash
make test-ejemplo
make run-ejemplo
```

Inspeccionar metadatos de exportación mediante el gestor:
```bash
./manage.sh info
```

Limpiar binarios y artefactos generados:
```bash
make clean
```
