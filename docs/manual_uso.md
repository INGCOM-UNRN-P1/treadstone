# Manual de Uso: Librería de Pruebas `p1_test` (v2.0.0)

## 1. Introducción y Fundamentos

`p1_test` es una biblioteca avanzada de pruebas unitarias para lenguaje C (estándar C11) desarrollada para la cátedra de **Programación 1** de la Universidad Nacional de Río Negro (UNRN).

Resuelve de forma nativa los problemas críticos de testing en entornos académicos:
1. **Inmunidad a Banderas de Compilación (`DEBUG` / `NDEBUG`):** A diferencia de `<assert.h>`, las aserciones no se desactivan con `-DNDEBUG` ni con ningún flag del compilador.
2. **Resiliencia de la Suite (No detiene el proceso):** Mediante saltos no locales (`setjmp`/`longjmp`), un assert fallido aborta únicamente la prueba en curso, permitiendo que las pruebas restantes se ejecuten y entreguen un reporte consolidado.
3. **Rescate de Señales Fatales (Segfaults y FPE):** Captura en tiempo de ejecución excepciones de hardware y memoria (`SIGSEGV`, `SIGFPE`, `SIGILL`, `SIGBUS`) evitando que un error de puntero nulo en el código del estudiante aborte abruptamente la suite completa.
4. **Watchdog contra Bucles Infinitos:** Límite configurable de tiempo por test (`SIGALRM`), interrumpiendo automáticamente funciones colgadas en `while(1)`.

---

## 2. Guía Rápida de Inicio (Quickstart)

```c
#include <stdio.h>
#include "p1_test.h"

int multiplicar(int a, int b) {
    return a * b;
}

TEST(prueba_multiplicacion) {
    SUBCASE("Factores positivos");
    ASSERT_INT_EQ(20, multiplicar(4, 5));

    SUBCASE("Multiplicación por cero");
    ASSERT_INT_EQ(0, multiplicar(10, 0));
}

int main(int argc, char **argv) {
    TEST_SUITE_BEGIN_ARGS("Suite de Demostración", argc, argv);
    RUN_TEST(prueba_multiplicacion);
    return TEST_REPORT();
}
```

### Compilación y Ejecución
```bash
gcc -std=c11 -Wall -Wextra -pedantic -Iinclude prueba.c -o test_bin
./test_bin
```

---

## 3. Línea de Comandos y Control de Ejecución (CLI)

Al inicializar la suite con `TEST_SUITE_BEGIN_ARGS("Nombre", argc, argv);`, el binario admite automáticamente las siguientes opciones de ejecución:

| Opción | Argumento | Descripción |
| :--- | :--- | :--- |
| `-k`, `--filter` | `<subcadena>` | Ejecuta únicamente las pruebas cuyo nombre contenga el texto especificado. |
| `-f`, `--fail-fast` | *(ninguno)* | Detiene la ejecución de la suite tras el primer test que falle. |
| `-q`, `--quiet` | *(ninguno)* | Modo compacto: emite `.` por acierto, `F` por fallo y `S` por salteado. |
| `-t`, `--timeout` | `<segundos>` | Tiempo límite máximo por test antes de abortar por timeout (por defecto: 5 s; 0 = desactivado). |
| `--tap` | *(ninguno)* | Emite resultados en formato estándar Test Anything Protocol v13. |
| `--md-report` | `<ruta>` | Genera un reporte detallado consolidado en formato Markdown (GFM). |
| `--no-color` | *(ninguno)* | Desactiva colores ANSI. |
| `-h`, `--help` | *(ninguno)* | Imprime el menú de ayuda y finaliza la ejecución. |

### Ejemplos de uso por terminal
```bash
# Ejecutar solo los tests relacionados a 'cadenas'
./test_bin -k cadenas

# Detener la suite en el primer error encontrado
./test_bin -f

# Ejecutar con timeout estricto de 2 segundos por prueba
./test_bin -t 2

# Generar reporte TAP para herramientas de corrección automática
./test_bin --tap

# Exportar informe consolidado en Markdown para GitHub Classroom
./test_bin --md-report informe_pruebas.md
```

---

## 4. Catálogo Completo de Aserciones

Todas las aserciones admiten dos familias de variantes ortogonales:
* **Sufijo `_MSG`:** permite anexar un mensaje contextual personalizado con formato estilo `printf` (`ASSERT_*_MSG(..., "formato %d", valor)`).
* **Sufijo `_HINT`:** permite suministrar una pista pedagógica (`ASSERT_*_HINT(..., "pista orientativa")`). La pista solo se emite en la terminal si la aserción falla, orientando al estudiante sin revelar la solución directa.

### 4.1. Booleanas
* `ASSERT_TRUE(cond)` / `ASSERT_TRUE_MSG(cond, fmt, ...)`
* `ASSERT_FALSE(cond)` / `ASSERT_FALSE_MSG(cond, fmt, ...)`

### 4.2. Enteros y Rangos Acotados
* `ASSERT_INT_EQ(esperado, obtenido)`: Igualdad con signo (`%lld`).
* `ASSERT_INT_NE(esperado, obtenido)`: Desigualdad con signo.
* `ASSERT_INT_LT(valor, limite)`: Menor estricto (`<`).
* `ASSERT_INT_LE(valor, limite)`: Menor o igual (`<=`).
* `ASSERT_INT_GT(valor, limite)`: Mayor estricto (`>`).
* `ASSERT_INT_GE(valor, limite)`: Mayor o igual (`>=`).
* `ASSERT_INT_BETWEEN(valor, min, max)`: Verifica `min <= valor && valor <= max`.
* `ASSERT_UINT_EQ(esperado, obtenido)`: Igualdad sin signo (`%llu`, `size_t`).
* `ASSERT_UINT_NE(esperado, obtenido)`: Desigualdad sin signo.

```c
ASSERT_INT_BETWEEN(nota, 1, 10);
ASSERT_UINT_EQ(5, strlen("mundo"));
```

### 4.3. Números Reales (Coma Flotante)
* `ASSERT_DOUBLE_EQ(esperado, obtenido, epsilon)`: Tolerancia absoluta (`|a - b| <= eps`).
* `ASSERT_DOUBLE_NE(esperado, obtenido, epsilon)`: Diferencia fuera de tolerancia absoluta.
* `ASSERT_DOUBLE_NEAR_REL(esperado, obtenido, rel_tol)`: Tolerancia relativa porcentual.

```c
// Tolerancia absoluta
ASSERT_DOUBLE_EQ(3.1415, pi_aprox, 0.001);

// Tolerancia relativa (útil para magnitudes muy grandes o ínfimas)
ASSERT_DOUBLE_NEAR_REL(1000000.0, 1000050.0, 0.0001); // 0.01% de margen
```

### 4.4. Cadenas de Caracteres
* `ASSERT_STR_EQ(esperado, obtenido)`: Igualdad exacta (maneja punteros `NULL` sin crashear).
* `ASSERT_STR_NE(esperado, obtenido)`: Desigualdad.
* `ASSERT_STR_CASE_EQ(esperado, obtenido)`: Igualdad insensible a mayúsculas/minúsculas.
* `ASSERT_STR_CONTAINS(cadena, subcadena)`: Búsqueda de subcadena.

```c
ASSERT_STR_CASE_EQ("INICIAR", comando_ingresado);
ASSERT_STR_CONTAINS(saludo, "Mundo");
```

### 4.5. Punteros y Direcciones
* `ASSERT_PTR_NULL(ptr)`
* `ASSERT_PTR_NOT_NULL(ptr)`
* `ASSERT_PTR_EQ(esperado, obtenido)`
* `ASSERT_PTR_NE(esperado, obtenido)`

### 4.6. Arreglos y Memoria Binaria
* `ASSERT_ARRAY_INT_EQ(arr_esperado, arr_obtenido, longitud)`:
  Recorre el arreglo; ante cualquier diferencia reporta el índice exacto, valor esperado y obtenido.
* `ASSERT_MEM_EQ(ptr_esperado, ptr_obtenido, tamaño_bytes)`:
  Compara estructuras o buffers byte a byte. Al fallar, imprime el offset hexadecimal y un volcado de contexto.

```c
int exp[] = {1, 2, 3, 4};
int act[] = {1, 2, 3, 4};
ASSERT_ARRAY_INT_EQ(exp, act, 4);

struct Config cfg1 = { ... }, cfg2 = { ... };
ASSERT_MEM_EQ(&cfg1, &cfg2, sizeof(struct Config));
```

### 4.7. Captura de Salida Estándar (`stdout`)
Evalúa funciones que imprimen en consola redirigiendo internamente los descriptores de salida:

```c
void imprimir_bienvenida(void) {
    printf("¡Bienvenido al sistema!\n");
}

TEST(prueba_salida_consola) {
    ASSERT_STDOUT_EQ(imprimir_bienvenida(), "¡Bienvenido al sistema!\n");
}
```

### 4.8. Fallo Explícito
* `ASSERT_FAIL("mensaje", ...)`: Provoca inmediatamente el fallo de la prueba.

### 4.9. Control de Memoria Dinámica en Runtime (Tracking de Heap)
`p1_test` instrumenta internamente las funciones de asignación dinámica de memoria para auditar fugas y balance de llamadas en tiempo de ejecución:
* `ASSERT_ALLOC_COUNT(esperado)` / `_MSG` / `_HINT`: Comprueba la cantidad total acumulada de reservas dinámicas (`malloc`, `calloc`).
* `ASSERT_FREE_COUNT(esperado)` / `_MSG` / `_HINT`: Comprueba la cantidad total de liberaciones (`free`).
* `ASSERT_NO_LEAKS()` / `_MSG` / `_HINT`: Verifica que no existan bloques huérfanos sin liberar (`active_blocks == 0`). Ante fugas, reporta el número exacto de bloques y bytes retenidos en el *heap*.

```c
TEST(prueba_balance_memoria) {
    int *buffer = malloc(10 * sizeof(int));
    ASSERT_PTR_NOT_NULL(buffer);
    ASSERT_ALLOC_COUNT(1);

    free(buffer);
    ASSERT_FREE_COUNT(1);
    ASSERT_NO_LEAKS();
}
```

---

## 5. Organización Avanzada: Hooks, Subcasos y Semillas

### 5.1. Hooks de Ciclo de Vida (`BEFORE_EACH` / `AFTER_EACH`)
Permite inicializar y liberar estructuras dinámicas (ej: memoria con `malloc`/`free`) automáticamente antes y después de cada prueba.

```c
static MiEstructura *ctx = NULL;

void setup(void) {
    ctx = crear_estructura();
}

void teardown(void) {
    destruir_estructura(ctx);
    ctx = NULL;
}

int main(int argc, char **argv) {
    TEST_SUITE_BEGIN_ARGS("Suite con Fixtures", argc, argv);
    BEFORE_EACH(setup);
    AFTER_EACH(teardown); // Se ejecuta incluso si el test aborta por fallo de assert

    RUN_TEST(test_uno);
    RUN_TEST(test_dos);
    return TEST_REPORT();
}
```

### 5.2. Subcasos Descriptivos (`SUBCASE`)
Etiqueta bloques lógicos dentro de un test para identificar con exactitud qué rama falló:

```c
TEST(validar_usuario) {
    SUBCASE("Usuario nulo");
    ASSERT_INT_EQ(-1, validar(NULL));

    SUBCASE("Usuario sin permisos");
    ASSERT_INT_EQ(0, validar(invitado));
}
```

### 5.3. Tests Omitidos o Pendientes (`SKIP_TEST` / `TEST_SKIP`)
* `SKIP_TEST(nombre, "motivo")`: Registra el test como salteado desde la suite.
* `TEST_SKIP("motivo")`: Aborta la prueba desde adentro de la función sin contar como fallo.

### 5.4. Pruebas con Semilla Determinística (`RUN_TEST_SEEDED`)
Fija `srand(semilla)` inmediatamente antes de correr la prueba para garantizar reproducibilidad algorítmica:

```c
RUN_TEST_SEEDED(prueba_barajar, 12345);
```

---

## 6. Detección de Fugas de Memoria con Valgrind

El framework y los proyectos estructurados cuentan con el comando `make memcheck`.

```bash
# En la raíz de la librería o de cualquier ejercicio:
make memcheck
```
Ejecuta la suite con `--leak-check=full --show-leak-kinds=all --error-exitcode=1`. Si un alumno olvida liberar memoria dinámica con `free()`, el proceso retorna código de error impidiendo que la prueba sea dada por válida.

---

## 7. Integración con `plantilla-TP` y Gestor `tp.sh`

El gestor `./tp.sh` del Trabajo Práctico incluye soporte nativo para `p1_test`:

* **Crear esqueleto de prueba:**
  ```bash
  ./tp.sh add-test ejercicio1 test_matriz
  ```
  Genera `ejercicios/ejercicio1/test_matriz.c` listo para compilar.

* **Ejecutar pruebas del TP:**
  ```bash
  ./tp.sh test
  ./tp.sh test ejercicio1
  ```

* **Auditar memoria con Valgrind en todo el TP:**
  ```bash
  ./tp.sh memcheck
  ./tp.sh memcheck ejercicio1
  ```

---

## 8. Cabeceras Modulares Avanzadas

Para mantener el núcleo ligero y desacoplado, las aserciones y utilidades avanzadas se distribuyen en cabeceras modulares satélite:

### 8.1. `p1_arrays.h` — Arreglos, Ordenamiento y Búsqueda
Proporciona comprobaciones tipadas sobre colecciones y vectores en memoria contigua:
* `ASSERT_ARRAY_INT_EQ(esperado, obtenido, longitud)`: Compara arreglos de enteros elemento a elemento indicando el primer índice con discrepancia.
* `ASSERT_ARRAY_INT_SORTED_ASC(arr, longitud)`: Valida que el arreglo esté ordenado en forma no decreciente ($a[i] \le a[i+1]$).
* `ASSERT_ARRAY_INT_SORTED_DESC(arr, longitud)`: Valida orden no creciente ($a[i] \ge a[i+1]$).
* `ASSERT_ARRAY_DOUBLE_EQ(esperado, obtenido, longitud, epsilon)`: Compara arreglos reales bajo tolerancia absoluta.
* `ASSERT_STR_ARRAY_EQ(esperado, obtenido)`: Compara arreglos de cadenas (`char *[]`) terminados en `NULL`.
* `ASSERT_ARRAY_INT_CONTAINS(arr, longitud, valor)`: Verifica que `valor` pertenezca al arreglo.
* `ASSERT_ARRAY_INT_NOT_CONTAINS(arr, longitud, valor)`: Verifica que `valor` no pertenezca al arreglo.

```c
#include "p1_arrays.h"

int datos[] = {10, 20, 30, 40};
ASSERT_ARRAY_INT_SORTED_ASC(datos, 4);
ASSERT_ARRAY_INT_CONTAINS(datos, 4, 30);
```

### 8.2. `p1_files.h` — Archivos de Texto, Binarios y Archivos Temporales
Permite evaluar persistencia en disco y gestionar archivos de prueba sin ensuciar el repositorio:
* **Aserciones sobre archivos:**
  * `ASSERT_FILE_EXISTS("ruta/archivo.txt")`: Comprueba existencia y legibilidad en disco.
  * `ASSERT_FILE_NOT_EXISTS("ruta/archivo.txt")`: Comprueba ausencia del archivo.
  * `ASSERT_FILE_EQ("esperado.txt", "obtenido.txt")`: Comparación de contenido línea a línea, señalando la primera discrepancia.
  * `ASSERT_FILE_CONTAINS("salida.log", "PALABRA_CLAVE")`: Búsqueda de patrones en archivos de texto.
  * `ASSERT_FILE_BINARY_EQ("exp.bin", "act.bin")`: Comparación binaria byte a byte con volcado hexadecimal del primer desvío.
* **Gestión de archivos temporales (Fixtures en `/tmp`):**
  * `p1_temp_file_create(contenido_texto)`: Crea un archivo temporal con el contenido provisto y retorna su ruta.
  * `p1_temp_file_create_named(prefijo, contenido_texto)`: Crea un archivo temporal con un prefijo personalizado en el nombre.
  * `p1_temp_file_create_binary(prefijo, datos, bytes)`: Crea un archivo temporal con una carga binaria arbitraria.
  * `p1_temp_files_cleanup()`: Elimina automáticamente todos los archivos temporales creados por el módulo.

```c
#include "p1_files.h"

TEST(prueba_persistencia) {
    const char *ruta_tmp = p1_temp_file_create("id,nombre\n1,Ana\n2,Luis\n");
    ASSERT_FILE_EXISTS(ruta_tmp);
    ASSERT_FILE_CONTAINS(ruta_tmp, "Ana");
    p1_temp_files_cleanup();
}
```

### 8.3. `p1_stdio.h` — Mocks y Simulación de Entrada/Salida
Herramientas para probar programas interactivos de consola y funciones que leen de `stdin` o imprimen en `stdout`/`stderr`:
* **Aserciones integradas:**
  * `ASSERT_STDIO_EQ(sentencia, "entrada\n", "salida esperada\n")`: Inyecta la entrada en `stdin`, ejecuta la sentencia y valida la salida capturada en `stdout` en un solo paso.
  * `ASSERT_STDOUT_EQ(sentencia, "salida esperada\n")`: Captura y valida `stdout`.
  * `ASSERT_STDERR_EQ(sentencia, "error esperado\n")`: Captura y valida `stderr`.
  * `ASSERT_STDOUT_CONTAINS(sentencia, "subcadena")`: Verifica que la salida de consola contenga el texto indicado.
  * `ASSERT_STDERR_CONTAINS(sentencia, "subcadena")`: Verifica que el flujo de error contenga el texto indicado.
  * `ASSERT_STDIN_CONSUMED()`: Asegura que la función probada haya leído y consumido la totalidad de los datos provistos en `stdin` (alcanzando `EOF`).
* **Primitivas de control de flujos paso a paso:**
  * `p1_mock_stdin_feed(cadena)` / `p1_mock_stdin_restore()`: Inyección y restauración manual para `scanf` o `fgets`.
  * `p1_capture_stdout_begin()` / `p1_capture_stdout_end(buffer, max_len)`: Captura explícita de `stdout`.
  * `p1_capture_stderr_begin()` / `p1_capture_stderr_end(buffer, max_len)`: Captura explícita de `stderr`.

```c
#include "p1_stdio.h"

void pedir_y_saludar(void) {
    char nombre[32];
    if (scanf("%31s", nombre) == 1) {
        printf("Hola, %s!\n", nombre);
    }
}

TEST(prueba_interactiva) {
    ASSERT_STDIO_EQ(pedir_y_saludar(), "Martin\n", "Hola, Martin!\n");
    ASSERT_STDIN_CONSUMED();
}
```

### 8.4. `tda/contracts.h` — Framework de Invariantes y Contratos en Runtime (C11)
Diseñado para la enseñanza y verificación formal de **Tipos de Datos Abstractos (TDA)** mediante el paradigma de *Diseño por Contrato*:

* **Primitivas de Contrato:**
  * `TDA_REQUIERE(condicion)`: **Precondición**. Valida que los argumentos recibidos cumplan los requisitos de la interfaz pública (por ejemplo, puntero no nulo o índice válido). Si se viola, imprime el archivo, línea y condición en `stderr` y aborta la prueba.
  * `TDA_GARANTIZA(condicion)`: **Postcondición**. Valida las promesas que la función hace sobre su resultado o sobre el estado final antes de retornar.
  * `TDA_INVARIANTE(estructura, validador)`: **Invariante de representación**. Invoca la función validadora interna (de firma `static bool validador(const tipo_t *)`) para certificar que el TDA permanece en un estado consistente. Cada evaluación exitosa incrementa un contador interno.
  * `tda_contratos_evaluados()`: Retorna el número de invariantes evaluados en la unidad de traducción, permitiendo a la suite de pruebas verificar que el estudiante no omitió las comprobaciones de invariantes.

* **Inmunidad en Producción (`-DNDEBUG`):**
  Al compilar con `-DNDEBUG`, todas las macros se expanden a `((void)0)`, asegurando **cero sobrecosto en tiempo de ejecución**.

* **Validación Estructural de Nodos Enlazados:**
  Proporciona utilidades genéricas independientes del tipo de dato concreto:
  * `tda_siguiente(nodo, offset_sig)`: Devuelve el puntero al nodo sucesor leyendo el offset del miembro `siguiente` sin violar el encapsulamiento.
  * `tda_tiene_ciclo(primero, offset_sig, max_nodos)`: Algoritmo acotado que detecta ciclos o punteros circulares en listas enlazadas, previniendo cuelgues o bucles infinitos en la suite de pruebas si la estructura del estudiante está corrupta.

```c
#include "tda/contracts.h"

// En tiempo de compilación: struct completo solo en el .c (tipo opaco)
struct lista {
    struct nodo *cabeza;
    size_t cantidad;
};

static bool lista_es_valida(const lista_t *l) {
    if (l == NULL) return false;
    // Verifica ausencia de ciclos en la cadena de nodos
    if (tda_tiene_ciclo(l->cabeza, offsetof(struct nodo, siguiente), 1000)) {
        return false;
    }
    return true;
}

bool lista_insertar(lista_t *l, int valor) {
    TDA_REQUIERE(l != NULL);
    TDA_INVARIANTE(l, lista_es_valida);

    // ... inserción ...

    TDA_GARANTIZA(l->cantidad > 0);
    TDA_INVARIANTE(l, lista_es_valida);
    return true;
}
```

