# Manual de Uso: Librería de Pruebas `p1_test`

## 1. Introducción y Fundamentos

`p1_test` es una biblioteca de pruebas unitarias para lenguaje C (estándar C99) concebida para la cátedra de **Programación 1** de la Universidad Nacional de Río Negro (UNRN).

A diferencia del mecanismo estándar provisto por `<assert.h>`, `p1_test` resuelve dos problemas centrales en el desarrollo y la corrección de software:

1. **Inmunidad a banderas de compilación (`DEBUG` / `NDEBUG`):** En C estándar, si se define la macro `NDEBUG` (habitual en builds de optimización o release), las llamadas a `assert()` se eliminan por completo del código compilado en el preprocesador. Con `p1_test`, todas las aserciones se evalúan de forma incondicional, independientemente de qué banderas de depuración u optimización estén activas.
2. **Resiliencia de la Suite (No detiene el proceso):** Un `assert()` estándar de C aborta abruptamente todo el proceso del programa (`abort()` o `SIGABRT`) ante la primera falla. Esto impide conocer el estado del resto de las pruebas. `p1_test` captura el fallo mediante saltos no locales (`setjmp`/`longjmp`), finaliza únicamente el caso de prueba en curso, registra el error con diagnóstico detallado y continúa con la ejecución de las pruebas restantes para brindar un reporte consolidado.

---

## 2. Características Técnicas

* **Formato Cabecera Única (*Header-Only*):** Solo necesitás incluir `p1_test.h`. No requiere compilar archivos objeto adicionales ni lidiar con dependencias complejas.
* **Compatibilidad Estricta:** C99 puro, compatible con `-Wall -Wextra -Werror -pedantic -std=c99`.
* **Aserciones Tipadas y Didácticas:** Diagnósticos claros que indican archivo, línea, expresión evaluada, valor esperado y valor realmente obtenido.
* **Soporte de Colores ANSI:** Resaltado visual en terminal (desactivable automáticamente definiendo `P1_TEST_NO_COLOR`).
* **Integración con Makefiles y CI/CD:** La función `TEST_REPORT()` retorna `0` si todas las pruebas fueron exitosas y `1` si hubo fallos, permitiendo que `make test` corte o prosiga según corresponda.

---

## 3. Guía de Inicio Rápido (Quickstart)

Estructura mínima de un archivo de prueba (`prueba.c`):

```c
#include <stdio.h>
#include "p1_test.h"

// Función simple a probar
int sumar(int a, int b) {
    return a + b;
}

// 1. Declarar el caso de prueba con TEST(nombre)
TEST(suma_positivos) {
    ASSERT_INT_EQ(5, sumar(2, 3));
    ASSERT_INT_GT(sumar(1, 1), 0);
}

TEST(suma_con_cero) {
    ASSERT_INT_EQ(0, sumar(0, 0));
    ASSERT_INT_EQ(7, sumar(7, 0));
}

// 2. Punto de entrada con la suite
int main(void) {
    TEST_SUITE_BEGIN("Suite de Prueba: Operaciones Matemáticas");

    RUN_TEST(suma_positivos);
    RUN_TEST(suma_con_cero);

    return TEST_REPORT();
}
```

### Compilación y Ejecución directa
```bash
gcc -std=c99 -Wall -Wextra -pedantic -Iinclude prueba.c -o test_bin
./test_bin
```

---

## 4. Catálogo Detallado de Aserciones

Todas las aserciones tienen una versión estándar y una versión con sufijo `_MSG`, que permite agregar una explicación adicional o contexto dinámico mediante formato estilo `printf`.

### 4.1. Aserciones Booleanas

Verifican condiciones de verdad o falsedad lógica:

* `ASSERT_TRUE(condicion)` / `ASSERT_TRUE_MSG(condicion, "mensaje", ...)`
  Falla si `condicion` evalúa a cero (falso).
* `ASSERT_FALSE(condicion)` / `ASSERT_FALSE_MSG(condicion, "mensaje", ...)`
  Falla si `condicion` evalúa a distinto de cero (verdadero).

**Ejemplo:**
```c
TEST(validar_banderas) {
    int logueado = 1;
    ASSERT_TRUE(logueado);
    ASSERT_FALSE(logueado == 0);
    ASSERT_TRUE_MSG(logueado > 0, "El estado debe ser positivo, valor: %d", logueado);
}
```

---

### 4.2. Aserciones de Enteros con Signo (`int`, `long`, `short`)

Imprimen el valor esperado y el valor obtenido como enteros:

* `ASSERT_INT_EQ(esperado, obtenido)`: Comprueba `esperado == obtenido`.
* `ASSERT_INT_NE(esperado, obtenido)`: Comprueba `esperado != obtenido`.
* `ASSERT_INT_LT(valor, limite)`: Comprueba `valor < limite`.
* `ASSERT_INT_LE(valor, limite)`: Comprueba `valor <= limite`.
* `ASSERT_INT_GT(valor, limite)`: Comprueba `valor > limite`.
* `ASSERT_INT_GE(valor, limite)`: Comprueba `valor >= limite`.

**Ejemplo:**
```c
TEST(operaciones_enteras) {
    int resultado = 10 * 2;
    ASSERT_INT_EQ(20, resultado);
    ASSERT_INT_NE(0, resultado);
    ASSERT_INT_LT(resultado, 50);
    ASSERT_INT_GE(resultado, 20);
}
```

---

### 4.3. Aserciones de Enteros sin Signo (`unsigned int`, `size_t`)

Imprimen los valores como enteros no negativos (`%llu`):

* `ASSERT_UINT_EQ(esperado, obtenido)`: Comprueba igualdad sin signo.
* `ASSERT_UINT_NE(esperado, obtenido)`: Comprueba desigualdad sin signo.

**Ejemplo:**
```c
TEST(longitud_de_arreglo) {
    size_t elementos = 5;
    ASSERT_UINT_EQ(5, elementos);
    ASSERT_UINT_NE(0, elementos);
}
```

---

### 4.4. Aserciones de Coma Flotante (`double`, `float`)

En aritmética de coma flotante, comparar con `==` produce falsos negativos por imprecisión de redondeo. `p1_test` exige un parámetro de tolerancia (*epsilon*):

* `ASSERT_DOUBLE_EQ(esperado, obtenido, epsilon)`: Comprueba que `|esperado - obtenido| <= epsilon`.
* `ASSERT_DOUBLE_NE(esperado, obtenido, epsilon)`: Comprueba que `|esperado - obtenido| > epsilon`.

**Ejemplo:**
```c
TEST(calculo_geometrico) {
    double division = 1.0 / 3.0;
    // Compara con una tolerancia de 0.0001
    ASSERT_DOUBLE_EQ(0.333333, division, 0.0001);
}
```

---

### 4.5. Aserciones sobre Cadenas de Caracteres (`strings`)

Comparan mediante `strcmp` y gestionan punteros `NULL` sin generar fallos de segmentación:

* `ASSERT_STR_EQ(esperado, obtenido)`: Comprueba que ambas cadenas sean idénticas. Si ambas son `NULL`, se consideran iguales.
* `ASSERT_STR_NE(esperado, obtenido)`: Comprueba que difieran en contenido o estado de nulidad.
* `ASSERT_STR_CONTAINS(cadena, subcadena)`: Comprueba que `subcadena` esté presente dentro de `cadena`.

**Ejemplo:**
```c
TEST(manipulacion_texto) {
    const char *mensaje = "Hola Cátedra P1";
    ASSERT_STR_EQ("Hola Cátedra P1", mensaje);
    ASSERT_STR_NE("Chau", mensaje);
    ASSERT_STR_CONTAINS(mensaje, "Cátedra");
}
```

---

### 4.6. Aserciones sobre Punteros

Verifican direcciones de memoria o condiciones de nulidad:

* `ASSERT_PTR_NULL(ptr)`: Comprueba `ptr == NULL`.
* `ASSERT_PTR_NOT_NULL(ptr)`: Comprueba `ptr != NULL`.
* `ASSERT_PTR_EQ(esperado, obtenido)`: Comprueba que ambos punteros apunten a la misma dirección de memoria.
* `ASSERT_PTR_NE(esperado, obtenido)`: Comprueba direcciones diferentes.

**Ejemplo:**
```c
TEST(gestion_memoria) {
    int variable = 42;
    int *puntero = &variable;
    int *nulo = NULL;

    ASSERT_PTR_NOT_NULL(puntero);
    ASSERT_PTR_NULL(nulo);
    ASSERT_PTR_EQ(&variable, puntero);
}
```

---

### 4.7. Fallo Explícito

* `ASSERT_FAIL("mensaje explicativo", ...)`: Provoca de manera inmediata el fallo del test en curso con el mensaje especificado. Se utiliza comúnmente en ramas que nunca deberían ejecutarse (por ejemplo, el default de un switch exhaustivo o tras una llamada que debió lanzar error previo).

---

## 5. Control de Flujo y Resiliencia

El framework utiliza internamente `setjmp` y `longjmp` de `<setjmp.h>`:

1. `RUN_TEST(nombre)` invoca `setjmp(jump_env)` estableciendo un punto de restauración y llama a la función del test.
2. Si cualquier aserción falla (incluso si está dentro de una función auxiliar anidada llamada por el test):
   * Se formatea y muestra el mensaje de error en `stderr`.
   * Se incrementa el contador de aserciones falladas.
   * Se marca el test como fallado.
   * Se invoca `longjmp(jump_env, 1)`, devolviendo la ejecución instantáneamente a `RUN_TEST`.
3. `RUN_TEST` procesa el resultado del test e incrementa el contador de aprobados o desaprobados.
4. El programa continúa ejecutando el siguiente `RUN_TEST` programado.
5. Al concluir, `TEST_REPORT()` imprime el informe consolidado y retorna `0` o `1`.

---

## 6. Integración con Proyectos `plantilla-TP`

Para integrar `p1_test` en un Trabajo Práctico con la arquitectura de cátedra:

1. Copiar `include/p1_test.h` dentro de `libs/p1_test/include/p1_test.h`.
2. En el archivo `ejercicios/ejercicioX/Makefile`, definir:
   ```makefile
   LIB_NAME ?= p1_test
   ```
3. En `prueba.c` del ejercicio, incluir la cabecera:
   ```c
   #include "p1_test.h"
   ```
4. Ejecutar las pruebas directamente con el gestor del TP:
   ```bash
   ./tp.sh test ejercicio1
   ```
   o ejecutando `make test` desde la carpeta del ejercicio o desde la raíz del proyecto.
