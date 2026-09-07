# Librería `p1_test`: Framework de Pruebas Unitarias para C99

Biblioteca ligera *header-only* de pruebas unitarias para lenguaje C (estándar C99), desarrollada para la cátedra de **Programación 1** (Universidad Nacional de Río Negro).

Diseñada para ser simple, didáctica, resiliente a fallos y completamente inmune a directivas de preprocesador como `NDEBUG` o `DEBUG`.

---

## 🚀 Características Principales

* **Inmune a Banderas de Compilación:** A diferencia de `<assert.h>`, las aserciones **no se eliminan** al compilar con `-DNDEBUG` ni con `-DDEBUG`.
* **Resiliencia ante Fallos (`setjmp`/`longjmp`):** Una aserción fallida no interrumpe abruptamente todo el ejecutable con un core dump (`SIGABRT`); aborta únicamente el caso de prueba actual y permite continuar la suite para obtener un reporte completo.
* **Aserciones Tipadas y Descriptivas:**
  * Booleanas: `ASSERT_TRUE`, `ASSERT_FALSE`
  * Enteros con signo: `ASSERT_INT_EQ`, `ASSERT_INT_NE`, `ASSERT_INT_LT`, `ASSERT_INT_LE`, `ASSERT_INT_GT`, `ASSERT_INT_GE`
  * Enteros sin signo: `ASSERT_UINT_EQ`, `ASSERT_UINT_NE`
  * Coma flotante: `ASSERT_DOUBLE_EQ`, `ASSERT_DOUBLE_NE` (con margen de tolerancia epsilon)
  * Cadenas de caracteres: `ASSERT_STR_EQ`, `ASSERT_STR_NE`, `ASSERT_STR_CONTAINS` (seguras con punteros `NULL`)
  * Punteros: `ASSERT_PTR_NULL`, `ASSERT_PTR_NOT_NULL`, `ASSERT_PTR_EQ`, `ASSERT_PTR_NE`
  * Mensajes personalizados en todas las variantes (`_MSG`) y fallo explícito (`ASSERT_FAIL`).
* **Header-Only:** Sin necesidad de compilar archivos `.a` ni configurar enlazadores complejos; basta con incluir `p1_test.h`.
* **Diagnóstico Claro:** Salida con colores ANSI formateada pedagógicamente.
* **Integración Nativa:** Retorna código de salida estándar (`0` éxito, `1` fallo), ideal para Makefiles y herramientas de integración continua.

---

## 📖 Manual de Uso

Podés consultar el manual completo con ejemplos de código de cada aserción en:
* [Manual de Uso Exhaustivo](docs/manual_uso.md)

---

## 💻 Ejemplo Rápido

```c
#include <stdio.h>
#include "p1_test.h"

int duplicar(int x) {
    return x * 2;
}

TEST(prueba_duplicar) {
    ASSERT_INT_EQ(4, duplicar(2));
    ASSERT_INT_EQ(0, duplicar(0));
    ASSERT_INT_NE(10, duplicar(3));
}

int main(void) {
    TEST_SUITE_BEGIN("Suite de Ejemplo");
    RUN_TEST(prueba_duplicar);
    return TEST_REPORT();
}
```

---

## 📁 Estructura del Repositorio

* **`include/p1_test.h`**: Cabecera principal del framework.
* **`docs/manual_uso.md`**: Manual de uso completo y referencia de la API.
* **`tests/`**: Pruebas unitarias de autoverificación del propio framework:
  * `test_p1_test.c`: Verifica el correcto funcionamiento de todas las aserciones y flags.
  * `test_p1_failures.c`: Verifica la recuperación ante fallos mediante `setjmp`/`longjmp`.
* **`ejemplos/proyecto_tp/`**: Proyecto de ejemplo completo basado en la arquitectura estándar de `plantilla-TP`, con un ejercicio funcional y su suite de pruebas integrada.
* **`Makefile`**: Orquestador para compilar y ejecutar las pruebas del framework y los ejemplos.

---

## 🛠️ Comandos de Compilación y Prueba

Compilar y ejecutar las pruebas del framework:
```bash
make test
```

Compilar y ejecutar las pruebas con bandera `-DNDEBUG`:
```bash
make test-ndebug
```

Compilar y ejecutar el proyecto de ejemplo:
```bash
make run-ejemplo
```

Limpiar archivos generados:
```bash
make clean
```
