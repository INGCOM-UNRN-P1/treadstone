# Propuestas de Mejoras Quality of Life (QoL) para `p1_test`

Este documento detalla 60 mejoras funcionales y de experiencia de uso (QoL) para la suite de pruebas unitarias [`p1_test.h`](file:///home/mrtin/dev/p1/practicas/plantillas/lib_test/include/p1_test.h) y sus módulos satélite ([`p1_arrays.h`](file:///home/mrtin/dev/p1/practicas/plantillas/lib_test/include/p1_arrays.h), [`p1_files.h`](file:///home/mrtin/dev/p1/practicas/plantillas/lib_test/include/p1_files.h), [`p1_stdio.h`](file:///home/mrtin/dev/p1/practicas/plantillas/lib_test/include/p1_stdio.h)).

---

## 1. Reportes y Formatos de Salida CI/CD (1-10)

### 01. Exportador JUnit XML (`--junit <ruta>`)
* **Problema:** Los servidores de CI/CD (GitHub Actions, GitLab) requieren XML estándar para visualizar fallos en paneles gráficos.
* **Solución:** Escribir un parser de eventos en `p1_run_suite()` que vuelque `<testsuite>` y `<testcase>` al finalizar.
* **Resultado medible:** Pipeline de GitHub Actions reporta tests nativamente en la solapa *Checks*.

### 02. Exportador JSON estructurado (`--json <ruta>`)
* **Problema:** Los scripts evaluadores de cátedra deben parsear texto plano mediante regex para extraer métricas.
* **Solución:** Serializar suite, tests, aserciones evaluadas, tiempos y fallos a un archivo JSON sin dependencias externas.
* **Resultado medible:** Archivo JSON generado con salida estructurada de tests aprobados, desaprobados y fallos.

### 03. Barra de progreso interactiva para TTY
* **Problema:** Suites largas generan scroll innecesario en terminales interactivas de alumnos.
* **Solución:** Detectar `isatty(fileno(stdout))` y refrescar una barra `[=====>    ] 60%` con retornos de carro (`\r`).
* **Resultado medible:** Pantalla limpia sin scroll continuo hasta emitir el informe final de fallos.

### 04. Diff ANSI unificado en discrepancias de cadenas y archivos
* **Problema:** Textos largos o multilínea dificultan identificar el carácter o línea discrepante.
* **Solución:** Implementar algoritmo Myers simplificado para imprimir líneas con `+` (verde) y `-` (rojo).
* **Resultado medible:** Reporte de fallo visual con contexto de líneas contiguas idéntico a `diff -u`.

### 05. Filtro de exclusión de tests (`-e, --exclude <patrón>`)
* **Problema:** La bandera `-k` solo permite inclusión; aislar suites lentas requiere filtrar una por una.
* **Solución:** Añadir opción CLI para descartar tests cuyo nombre coincida con la subcadena indicada.
* **Resultado medible:** `./test_bin -e "lento"` ejecuta la suite completa omitiendo tests marcados como lentos.

### 06. Generador de informe Markdown (`--md-report <ruta>`)
* **Problema:** El docente debe formatear manualmente los resultados para publicar devoluciones en foros o issues.
* **Solución:** Emitir resumen en tablas Markdown con insignias de aprobación y bloques colapsables para fallos.
* **Resultado medible:** Archivo `.md` generado directamente consumible por GitHub Classroom o Moodle.

### 07. Exportador tabular CSV (`--csv <ruta>`)
* **Problema:** Cargar notas en planillas de cálculo administrativas requiere procesamiento manual.
* **Solución:** Escribir fila CSV con columnas `suite,test,estado,tiempo_ms,aserciones,fallo`.
* **Resultado medible:** Importación directa en LibreOffice Calc o Google Sheets sin parseos adicionales.

### 08. Agrupación y filtrado por etiquetas (`TEST_TAG` / `--tag <tag>`)
* **Problema:** Filtrar solo por subcadena del nombre es insuficiente para clasificaciones cruzadas (ej: "tda", "memoria").
* **Solución:** Asociar metadata textual a cada test mediante macro auxiliar y filtrar vía CLI.
* **Resultado medible:** `./test_bin --tag tda` corre únicamente tests asociados a esa etiqueta.

### 09. Auditoría de tiempos de ejecución (`--durations <N>`)
* **Problema:** Dificultad para detectar soluciones ineficientes (ej: algoritmos $O(N^2)$ en vez de $O(N)$).
* **Solución:** Registrar `clock_gettime` por test y emitir tabla con los $N$ tests más lentos de la corrida.
* **Resultado medible:** Ranking de tests lentos visible al pie del resumen final.

### 10. Modo silencioso estricto (`--silent` / `-s`)
* **Problema:** Integraciones en scripts bash de corrección masiva se ven contaminadas por texto informativo.
* **Solución:** Suprimir toda salida estándar excepto en caso de error fatal; comunicar estado únicamente vía código de retorno.
* **Resultado medible:** `echo $?` devuelve `0` sin imprimir caracteres en stdout ante suites exitosas.

---

## 2. Diagnóstico, Pistas y Depuración Didáctica (11-20)

### 11. Pistas pedagógicas condicionales (`ASSERT_*_HINT`)
* **Problema:** El alumno ve un valor numérico erróneo pero desconoce qué error conceptual suele causarlo.
* **Solución:** Sobrecargar macros para aceptar un mensaje orientativo impreso exclusivamente cuando el assert falla.
* **Resultado medible:** Diagnóstico extendido: `Pista didáctica: Revisá la inicialización del acumulador en 0`.

### 12. Traza de llamadas en colapsos (`backtrace` en crashes)
* **Problema:** Un `SIGSEGV` indica la función que falló pero no la ruta de llamadas de funciones previas.
* **Solución:** Invocar `backtrace()` y `backtrace_symbols()` dentro del manejador de señales POSIX antes de `longjmp`.
* **Resultado medible:** Pila de llamadas con nombres de funciones y offsets impresa en consola ante un puntero nulo.

### 13. Buffer de contexto de variables locales (`P1_INFO(fmt, ...)`)
* **Problema:** En bucles de prueba con muchas iteraciones, el fallo no muestra qué valor tenía el índice `i`.
* **Solución:** Guardar mensajes temporales en un buffer circular que se descarta si el test pasa y se vuelca si falla.
* **Resultado medible:** El fallo expone: `Contexto local: iteración i = 42, valor acumulado = -1`.

### 14. Modo verboso con trazabilidad (`-v, -vv`)
* **Problema:** Para depurar el alumno necesita confirmar qué aserciones intermedias pasaron antes de un cuelgue.
* **Solución:** CLI flag que imprime cada aserción evaluada con sus operandos resueltos en tiempo real.
* **Resultado medible:** Salida interactiva mostrando `[PASS] ASSERT_INT_EQ(10, 10)` a medida que se ejecutan.

### 15. Aserción de orden estricto de llamadas (`ASSERT_CALL_ORDER`)
* **Problema:** Dificultad para verificar que el alumno llame a `inicializar()` antes que a `insertar()`.
* **Solución:** Registro interno de identificadores numéricos y verificación de secuencia monótona creciente.
* **Resultado medible:** Fallo descriptivo: `Llamada a 'insertar()' ejecutada antes de 'inicializar()'`.

### 16. Ejecución paso a paso interactiva (`--step`)
* **Problema:** Suites extensas no permiten inspeccionar la consola entre pruebas individuales.
* **Solución:** Pausar la ejecución tras cada test individual y esperar que el usuario presione `ENTER`.
* **Resultado medible:** Control secuencial manual sin necesidad de configurar breakpoints en GDB.

### 17. Punto de interrupción condicional para GDB (`P1_BREAK_ON_FAIL`)
* **Problema:** En depurador interactivo, el test fallido salta a `longjmp` sin dejar el frame de pila abierto.
* **Solución:** Emitir `raise(SIGTRAP)` antes de registrar el fallo cuando se define bandera de compilación o entorno.
* **Resultado medible:** GDB detiene el proceso exactamente en la línea donde ocurrió el assert fallido.

### 18. Visualizador de árboles y grafos en consola ASCII
* **Problema:** Visualizar un árbol binario discrepante inspeccionando punteros desorienta al estudiante.
* **Solución:** Función auxiliar didáctica que renderiza árboles o estructuras en formato ASCII dendrograma.
* **Resultado medible:** Gráfico en consola mostrando la discrepancia exacta de hijos izquierdos/derechos.

### 19. Aserción con función impresora personalizada (`ASSERT_STRUCT_EQ`)
* **Problema:** `ASSERT_MEM_EQ` vuelca bytes hexadecimales ilegibles para tipos `struct Alumno` o `struct Vector`.
* **Solución:** Macro que recibe puntero a función de formateo `void (*repr)(const void*, char*, size_t)`.
* **Resultado medible:** Fallo legible: `Esperado: {padron: 1234, nota: 8} - Obtenido: {padron: 1234, nota: 4}`.

### 20. Aserciones booleanas con mensaje enriquecido (`ASSERT_TRUE_MSG`)
* **Problema:** `ASSERT_TRUE(cond)` solo imprime la expresión literal sin contexto dinámico.
* **Solución:** Macro con argumentos variables estilo `printf` concatenados al reporte de fallo.
* **Resultado medible:** Mensaje detallado: `Fallo en ASSERT_TRUE: cola vacía luego de encolar elemento 5`.

---

## 3. Mocks, Inyección de Fallas y Simulación de Entorno (21-30)

### 21. Mock y aislamiento de variables de entorno (`p1_mock_setenv`)
* **Problema:** Modificar variables con `setenv()` ensucia el entorno de tests subsiguientes.
* **Solución:** Wrapper que almacena el valor previo y programa su restauración en el bloque `AFTER_EACH`.
* **Resultado medible:** Las variables de entorno vuelven a su estado original al finalizar el test.

### 22. Mock de reloj del sistema (`p1_mock_time`)
* **Problema:** Probar algoritmos dependientes de `time()` o fechas genera tests frágiles y no deterministas.
* **Solución:** Sobrescritura simbólica o wrapper de `time()` que devuelve una marca temporal fija configurable.
* **Resultado medible:** Funciones temporales testeadas con fechas estáticas reproducibles.

### 23. Inyector de fallas en asignación de memoria (`p1_mock_malloc_fail_after`)
* **Problema:** Dificultad para validar que el alumno verifique retornos `NULL` de `malloc` sin simular falta de memoria.
* **Solución:** Contador interno que devuelve `NULL` a partir de la $N$-ésima llamada a asignadores de memoria.
* **Resultado medible:** Verificación sistemática de ramas de liberación ante errores de `malloc`.

### 24. Simulación de fallos en sistema de archivos (`p1_mock_fopen_fail`)
* **Problema:** Testear que una función maneje archivos inexistentes o sin permisos requiere manipular el disco real.
* **Solución:** Hook para `fopen()` que rechaza aperturas sobre patrones de rutas prefijadas asignando `errno = EACCES`.
* **Resultado medible:** Prueba de ramas de error en I/O sin alterar permisos en el sistema operativo anfitrión.

### 25. Aserción de terminación de proceso (`ASSERT_EXIT_CODE`)
* **Problema:** No se puede evaluar si una función abortiva llama a `exit(EXIT_FAILURE)` sin cerrar la suite completa.
* **Solución:** Ejecutar la función en un proceso hijo vía `fork()`, capturando el status en `waitpid()`.
* **Resultado medible:** Validación directa del código numérico emitido por `exit()`.

### 26. Mock de `stdin` con pausas y cortes de stream
* **Problema:** `p1_mock_stdin_feed` alimenta buffers completos pero no permite simular demoras o EOF espontáneos.
* **Solución:** Generador de stream interactivo mediante pipes POSIX con inyección temporizada de bytes.
* **Resultado medible:** Prueba fidedigna de parsers interactivos y bucles de lectura por terminal.

### 27. Mock de secuencias deterministas de `rand()`
* **Problema:** `srand(seed)` altera el generador global, afectando librerías que dependan de aleatoriedad.
* **Solución:** Vector de valores precargados devueltos secuencialmente por un wrapper de `rand()`.
* **Resultado medible:** Tests de algoritmos probabilísticos testeados contra secuencias fijas de enteros.

### 28. Intercepción de llamadas a `system()` (`ASSERT_SYSTEM_CMD`)
* **Problema:** Los alumnos invocan comandos de shell peligrosos o no portables (`system("cls")`, `system("rm -rf ...")`).
* **Solución:** Wrapper didáctico que intercepta el comando enviado a `system()` y valida su sintaxis.
* **Resultado medible:** Captura y comprobación del comando de consola sin ejecutarlo en el sistema anfitrión.

### 29. Captura de descriptores arbitrarios (`p1_capture_fd`)
* **Problema:** Solo se capturan `stdout` y `stderr`; logs escritos en descriptores de archivos abiertos escapan al control.
* **Solución:** Utilizar `dup2()` generalizado para redirigir cualquier número de file descriptor a un buffer en memoria.
* **Resultado medible:** Aserciones directas sobre salidas dirigidas a sockets o logs en `fd = 3`.

### 30. Simulación de argumentos de consola (`p1_run_main`)
* **Problema:** Testear la función `main(int argc, char *argv[])` del alumno requiere compilar ejecutables separados.
* **Solución:** Macro auxiliar que encapsula `argc`, matriz de `argv` y mock de `stdout` para invocar `main` como función.
* **Resultado medible:** Pruebas unitarias directas sobre el punto de entrada de los ejercicios de los alumnos.

---

## 4. Auditoría de Recursos, Memoria y Concurrencia (31-40)

### 31. Aserción local de fugas de memoria (`ASSERT_NO_LEAKS`)
* **Problema:** Correr Valgrind en cada ejecución es lento; se requiere detección liviana dentro del propio test.
* **Solución:** Envolver llamadas en macros que lleven un balance neto de bytes solicitados versus liberados.
* **Resultado medible:** Fallo inmediato si un test concluye con bloques heap sin liberar: `Fuga detectada: 48 bytes`.

### 32. Aserción de cierre de descriptores (`ASSERT_NO_FD_LEAKS`)
* **Problema:** Funciones con `fopen` que no ejecutan `fclose` consumen descriptores silenciosamente.
* **Solución:** Contar entradas en `/proc/self/fd/` antes y después del bloque de prueba.
* **Resultado medible:** Alerta explícita: `Descriptor de archivo filtrado: fd = 4 permaneció abierto`.

### 33. Aislamiento estricto de tests mediante subprocesos (`--fork`)
* **Problema:** Una corrupción de memoria severa puede arruinar variables del runner de pruebas.
* **Solución:** Bandera CLI para bifurcar cada test con `fork()`, capturando código de salida o señal mediante pipe.
* **Resultado medible:** Protección total de la memoria del runner frente a sobrescrituras ilegales de punteros.

### 34. Ejecución paralela distribuida (`-j <N>`)
* **Problema:** Suites con cientos de pruebas unitarias tardan segundos en entornos monohilo.
* **Solución:** Pool de procesos trabajadores que consumen tests de una cola compartida y consolidan resultados.
* **Resultado medible:** Reducción lineal del tiempo de ejecución total en máquinas multinúcleo.

### 35. Validación de punteros mapeados (`ASSERT_PTR_VALID`)
* **Problema:** Diferenciar si un puntero no nulo apunta a memoria válida o es un puntero colgante (*dangling*).
* **Solución:** Consulta segura al sistema operativo mediante `mincore()` o parsing rápido de `/proc/self/maps`.
* **Resultado medible:** Fallo preventivo antes de intentar desreferenciar memoria liberada: `Puntero 0x55 no mapeado`.

### 36. Guardia contra desbordamiento de pila en recursión (`ASSERT_STACK_GUARD`)
* **Problema:** Recursiones infinitas provocan `SIGSEGV` al agotar el stack del proceso.
* **Solución:** Configurar pila alternativa mediante `sigaltstack()` para atender la señal sin voltear el binario.
* **Resultado medible:** Diagnóstico pedagógico claro: `Desbordamiento de pila (Stack Overflow) en función recursiva`.

### 37. Repetidor contra condiciones de carrera e indeterminismo (`--repeat <N>`)
* **Problema:** Fallos intermitentes (*flaky tests*) pasan desapercibidos en ejecuciones individuales.
* **Solución:** Ejecutar cada test $N$ veces seguidas con re-inicialización de hooks.
* **Resultado medible:** Detección de estados estáticos no re-inicializados al fallar en la corrida $k$.

### 38. Límite de consumo de memoria heap (`ASSERT_MEM_USAGE_LE`)
* **Problema:** Algoritmos que resuelven ejercicios cargando volúmenes exorbitantes en memoria sin justificación.
* **Solución:** Monitoreo del consumo acumulado mediante contadores en los hooks de asignación.
* **Resultado medible:** Fallo si el alumno utiliza más memoria que la cota definida: `Uso: 15 MB, Límite: 2 MB`.

### 39. Aserción de tiempo límite de CPU (`ASSERT_RUNTIME_LE`)
* **Problema:** Validar cotas de complejidad algorítmica empírica en ejercicios de ordenamiento y búsqueda.
* **Solución:** Medición de tiempo de CPU consumido (`clock()`) comparado contra umbral en milisegundos.
* **Resultado medible:** Fallo si el algoritmo excede el tiempo presupuestado: `Consumido: 120 ms, Máximo: 50 ms`.

### 40. Rescate y reporte de señales adicionales (`SIGBUS`, `SIGILL`, `SIGABRT`)
* **Problema:** Instrucciones ilegales, alineaciones inválidas o llamadas explícitas a `abort()` no tienen mensaje formativo.
* **Solución:** Registrar manejadores para `SIGBUS`, `SIGILL` y `SIGABRT` con traducción didáctica al español.
* **Resultado medible:** Mensaje pedagógico: `[CRASH: SIGABRT (Aborto explícito o fallo en aserción interna)]`.

---

## 5. Aserciones Tipadas Avanzadas y Estructuras de Datos (41-50)

### 41. Aserción de matrices bidimensionales de enteros (`ASSERT_MATRIX_INT_EQ`)
* **Problema:** Comparar matrices elemento por elemento mediante bucles ensucia el código del test.
* **Solución:** Macro que itera filas y columnas, reportando las coordenadas exactas `[fila][col]` de la discrepancia.
* **Resultado medible:** Diagnóstico: `Discrepancia en celda [2][3]: esperado 0, obtenido 1`.

### 42. Aserción de matrices de punto flotante (`ASSERT_MATRIX_DOUBLE_EQ`)
* **Problema:** Matrices de coeficientes numéricos requieren tolerancia en cada celda.
* **Solución:** Comparación bidimensional aplicando margen delta sobre valores `double`.
* **Resultado medible:** Coordenada, valor esperado, obtenido y delta reportados en un único bloque legible.

### 43. Aserciones para enteros de 64 bits (`ASSERT_INT64_EQ`, `ASSERT_UINT64_EQ`)
* **Problema:** Comparar `int64_t` con `ASSERT_INT_EQ` provoca desbordamientos o warnings de casteo.
* **Solución:** Macros dedicadas que emplean macros de formato estándar C99 `PRIi64` y `PRIu64`.
* **Resultado medible:** Comparación e impresión limpia de valores enteros grandes sin truncamiento.

### 44. Aserciones de números flotantes especiales (`ASSERT_FLOAT_IS_NAN`, `ASSERT_FLOAT_IS_INF`)
* **Problema:** Validar casos borde matemáticos (división por cero flotante) es engorroso con operadores de igualdad.
* **Solución:** Uso directo de las funciones estándar `isnan()` e `isinf()`.
* **Resultado medible:** Aserciones declarativas directas: `ASSERT_FLOAT_IS_NAN(resultado)`.

### 45. Aserción para listas enlazadas simples (`ASSERT_LIST_INT_EQ`)
* **Problema:** Para verificar una lista enlazada se escriben bucles redundantes de extracción de nodos.
* **Solución:** Macro auxiliar que recibe nodo inicial, offset de puntero siguiente, offset de valor y arreglo esperado.
* **Resultado medible:** Validación de la lista completa con reporte de nodo y posición donde falla la secuencia.

### 46. Aserción para pilas y colas (`ASSERT_TDA_EMPTY`, `ASSERT_TDA_NOT_EMPTY`)
* **Problema:** Validar estados de TDAs con llamadas manuales a funciones de longitud agrega boilerplate.
* **Solución:** Aserción declarativa que recibe un puntero opaco y puntero a función validadora booleana.
* **Resultado medible:** Claridad expresiva en pruebas de estructuras lineales.

### 47. Aserción de conjuntos de enteros sin orden (`ASSERT_ARRAY_INT_SET_EQ`)
* **Problema:** Algoritmos donde el orden de los elementos resultantes no está fijado provocan falsos negativos.
* **Solución:** Comparar presencia y cardinalidad de elementos ordenando copias internas antes del cotejo.
* **Resultado medible:** La aserción aprueba `{3, 1, 2}` contra `{1, 2, 3}` sin importar el orden.

### 48. Aserción de cadenas con expresiones regulares POSIX (`ASSERT_STR_MATCHES`)
* **Problema:** Validar salidas con marcas de tiempo variables o formatos estructurados requiere parsers ad-hoc.
* **Solución:** Compilación de expresiones regulares estándar mediante `regcomp()` y evaluación con `regexec()`.
* **Resultado medible:** `ASSERT_STR_MATCHES("^[0-9]{4}-[0-9]{2}-[0-9]{2}$", fecha_str)`.

### 49. Aserción de prefijo y sufijo en cadenas (`ASSERT_STR_STARTS_WITH`, `ASSERT_STR_ENDS_WITH`)
* **Problema:** Uso engorroso de `strncmp` y cálculo manual de offsets para comprobar encabezados de texto.
* **Solución:** Macros directas con verificación de punteros nulos y cálculo automático de longitudes.
* **Resultado medible:** Fallo autoexplicativo: `La cadena "error: archivo no encontrado" no comienza con "OK:"`.

### 50. Aserción de rango numérico en punto flotante (`ASSERT_DOUBLE_BETWEEN`)
* **Problema:** Acotar valores continuos (probabilidades en $[0.0, 1.0]$) requiere combinar dos macros.
* **Solución:** Macro atómica que valida cota inferior y superior con mensaje unificado de intervalo.
* **Resultado medible:** Mensaje claro: `Valor 1.05 fuera del intervalo [0.0, 1.0]`.

---

## 6. Herramientas de Cátedra, Docencia y Evaluación (51-60)

### 51. Ponderación de puntajes por prueba (`TEST_WEIGHT`)
* **Problema:** Todos los tests cuentan igual en el informe final, pero los ejercicios tienen distinta complejidad.
* **Solución:** Asignar pesos numéricos a cada test y calcular la nota final en escala universitaria (0 a 10).
* **Resultado medible:** Resumen final con cálculo automático: `Nota final: 8.50 / 10.00 (85% aprobado)`.

### 52. Modo examen estricto (`--exam`)
* **Problema:** Los alumnos pueden usar la salida detallada de diferencias para adivinar respuestas sin programar.
* **Solución:** Suprimir valores esperados y volcados de memoria en los fallos; mostrar únicamente si el caso pasó o falló.
* **Resultado medible:** Consola de examen limpia que mitiga ingeniería inversa de casos de prueba ocultos.

### 53. Generador de esqueletos de prueba (`p1-test-gen`)
* **Problema:** Los alumnos pierden tiempo configurando el boilerplate del archivo de test.
* **Solución:** Script CLI que analiza prototipos de un archivo `.h` y genera el esqueleto de tests automáticamente.
* **Resultado medible:** Creación instantánea de `test_modulo.c` con casos nominales preconfigurados.

### 54. Verificador de cobertura de aserciones
* **Problema:** Casos de prueba sin asserts que aprueban falsamente por no haber evaluado nada.
* **Solución:** Alerta o fallo en el runner si un test no evaluó al menos una aserción antes de retornar.
* **Resultado medible:** Test marcado como inválido: `[ADVERTENCIA] El test 'prueba_foo' no evaluó aserciones`.

### 55. Auditor estático de código de pruebas
* **Problema:** Alumnos que introducen `exit(0)` manual dentro del test para forzar la aprobación.
* **Solución:** Chequeo estático que audita el código fuente de pruebas antes de compilar.
* **Resultado medible:** Detección y bloqueo de llamadas prohibidas en archivos de prueba.

### 56. Categorización por objetivos de aprendizaje de Bloom
* **Problema:** Dificultad para balancear guías prácticas entre ejercicios mecánicos y de diseño.
* **Solución:** Metadatos en cada test (`BLOOM_LEVEL(APPLY)`, `BLOOM_LEVEL(ANALYZE)`) para auditar la dificultad.
* **Resultado medible:** Reporte de balance de evaluación emitido junto con la suite.

### 57. Reintento automático para tests con I/O (`--retries <N>`)
* **Problema:** En servidores saturados, operaciones de archivos temporales pueden fallar ocasionalmente por contención.
* **Solución:** Permitir reintentar tests marcados como dependientes de I/O hasta $N$ veces antes de declararlos fallidos.
* **Resultado medible:** Mitigación de falsos negativos en entornos de evaluación continua compartidos.

### 58. Sanitización de rutas locales en reportes
* **Problema:** Los mensajes de error exponen rutas absolutas del sistema de archivos local (`/home/docente/...`).
* **Solución:** Normalizar todas las rutas impresas haciéndolas relativas a la raíz del repositorio.
* **Resultado medible:** Trazas limpias que muestran `src/ejercicio1.c:45` en lugar de la ruta absoluta del host.

### 59. Linter de convenciones de cátedra integrado (`p1_test --lint`)
* **Problema:** Los alumnos presentan código que aprueba tests pero viola convenciones de estilo (variables en mayúsculas, etc.).
* **Solución:** Reglas livianas de estilo evaluables durante el comando `make test`.
* **Resultado medible:** Advertencias pedagógicas impresas al finalizar la ejecución de los tests.

### 60. Integración con Moodle VPL (`--vpl`)
* **Problema:** Integrar la librería en Moodle Virtual Programming Lab requiere scripts intermediarios para parsear notas.
* **Solución:** Bandera que emite directamente las directivas `Grade :=>> <nota>` y comentarios compatibles con VPL.
* **Resultado medible:** Calificación y feedback cargados automáticamente en el libro de calificaciones de Moodle.

---

## 7. Propuesta de Toolchain Externo para el Framework de Testing

### 7.1. Justificación y Arquitectura Global
[`p1_test.h`](file:///home/mrtin/dev/p1/practicas/plantillas/lib_test/include/p1_test.h) resuelve la aserción y el reporte en tiempo de ejecución, pero no audita comportamientos indefinidos a nivel instrucción, desbordamientos de pila fuera de alcance de `longjmp`, cobertura real de ramas ni contención de procesos durante correcciones masivas.

El toolchain externo rodea a la suite en cinco capas concéntricas:
1. **Capa Estática:** Linter, analizador semántico y formateador de estilo.
2. **Capa Dinámica Rápida:** Sanitizers del compilador (ASan, UBSan, LSan).
3. **Capa de Auditoría Profunda:** Valgrind (Memcheck y Massif).
4. **Capa de Cobertura y Mutación:** `gcov`, `gcovr` y tests de mutación.
5. **Capa de Sandboxing y CI/CD:** Bubblewrap (`bwrap`), GitHub Actions y Moodle VPL.

---

### 7.2. Matriz de Componentes del Toolchain

| Subsistema | Herramientas | Rol en la Cátedra | Invocación Estandarizada | Artefacto Generado |
| :--- | :--- | :--- | :--- | :--- |
| **Estilo y Reglas** | `clang-format` | Formato uniforme y limpio del código | `make format` | Código reescrito según `.clang-format` |
| **Análisis Estático** | `clang-tidy`, `cppcheck` | Auditoría de punteros sin inicializar, casts y variables globales | `make lint` | Diagnóstico de advertencias en terminal |
| **Sanitizers (LLVM/GCC)** | ASan + UBSan + LSan | Detección instantánea de accesos fuera de rango y UB en stack/heap | `make sanitize` | Traza en stderr con línea exacta de la infracción |
| **Auditoría Heap** | Valgrind Memcheck | Verificación estricta de ciclo de vida de punteros | `make memcheck` | Log sin leaks (`--error-exitcode=1`) |
| **Perfilado Memoria** | Valgrind Massif | Medición de consumo pico de heap en estructuras de datos | `make profile-mem` | `massif.out.<pid>` y gráfico ASCII con `ms_print` |
| **Cobertura de Código**| `gcov`, `gcovr` | Medición de ramas y líneas ejercitadas por los tests | `make coverage` | Reporte HTML en `build/coverage/index.html` |
| **Mutation Testing** | Mutador liviano en C/Python | Validación de la efectividad de las aserciones del alumno | `make test-mutants` | Puntuación de mutantes eliminados vs sobrevivientes |
| **Aislamiento Seguro** | Bubblewrap (`bwrap`) | Enjaulado estricto para corrección masiva sin root | `make sandbox-test` | Proceso ejecutado sin red ni acceso a FS |
| **Integración CI/CD** | GitHub Actions / VPL | Automatización de entrega y feedback continuo | Pipeline `.github/workflows/` | Reporte JUnit/Markdown y badges de aprobación |

---

### 7.3. Capa 1: Análisis Estático y Convenciones C99

#### 1. Configuración de `clang-tidy`
Inspecciona construcciones riesgosas antes de la compilación de pruebas:
```bash
clang-tidy src/*.c ejercicios/**/*.c \
  -checks='-*,readability-*,bugprone-*,clang-analyzer-*,cert-*' \
  -- -std=c99 -Iinclude -Ilibs/p1_test/include
```
* **Comprobaciones activas:** detección de variables locales no inicializadas (`bugprone-unhandled-exception`), pérdidas implícitas de precisión en castings y uso indebido de funciones inseguras (`cert-env33-c`).

#### 2. Auditoría rápida con `cppcheck`
Auditoría sin dependencias de cabeceras completas:
```bash
cppcheck --enable=all --inconclusive --std=c99 --error-exitcode=1 \
  --suppress=missingIncludeSystem -Iinclude src/
```

#### 3. Formateador `clang-format`
Reglas canónicas de cátedra aplicadas automáticamente:
```yaml
BasedOnStyle: LLVM
IndentWidth: 4
UseTab: Never
ColumnLimit: 100
AllowShortFunctionsOnASingleLine: None
BreakBeforeBraces: Custom
BraceWrapping:
  AfterFunction: true
  AfterControlStatement: false
```

---

### 7.4. Capa 2: Sanitizers del Compilador (ASan + UBSan)

A diferencia de Valgrind, los sanitizers se instrumentan en las instrucciones máquina durante la compilación, con una sobrecarga temporal de sólo 2x (frente a 20x de Valgrind), capturando desbordamientos en la pila (*stack-buffer-overflow*) que Valgrind ignora.

#### Flag de compilación unificada
```makefile
CFLAGS_SAN = $(CFLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer -g
```

#### Errores capturados de inmediato:
* **AddressSanitizer (ASan):**
  * `stack-buffer-overflow`: arrays locales desbordados en funciones recursivas o iterativas.
  * `global-buffer-overflow`: arreglos estáticos consultados más allá de su límite.
  * `use-after-free`: accesos a memoria dinámica previamente liberada.
* **UndefinedBehaviorSanitizer (UBSan):**
  * `signed-integer-overflow`: operaciones `INT_MAX + 1` desbordadas.
  * `shift-out-of-bounds`: desplazamientos a nivel de bits con magnitudes mayores al ancho del tipo.
  * `null-pointer-dereference`: desreferencias directas antes de llegar al manejador de señales.

---

### 7.5. Capa 3: Auditoría y Perfilado con Valgrind

#### 1. Memcheck (Alineación y pérdidas)
```bash
valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         --errors-for-leak-kinds=all \
         --error-exitcode=1 \
         ./build/test_bin
```
* `track-origins=yes` localiza la instrucción exacta donde se declaró o asignó una variable no inicializada que provocó un salto condicional errático.

#### 2. Massif (Consumo pico de Heap)
```bash
valgrind --tool=massif --massif-out-file=build/massif.out ./build/test_bin
ms_print build/massif.out | head -n 35
```
* Permite certificar que estructuras dinámicas (árboles, listas) liberen memoria en tiempo de ejecución sin acumular mesetas de consumo innecesarias.

---

### 7.6. Capa 4: Medición de Cobertura de Código (`gcov` / `gcovr`)

Medir la efectividad del conjunto de pruebas para comprobar qué porcentaje del código del estudiante fue efectivamente ejercitado.

#### Flujo de instrumentación:
```bash
# 1. Compilación con instrumentación de cobertura
gcc -Wall -Wextra -std=c99 --coverage -Iinclude src/ejercicio1.c tests/test_ejercicio1.c -o build/test_cov

# 2. Ejecución de la suite p1_test
./build/test_cov

# 3. Consolidación y reporte con gcovr excluyendo la propia librería de tests
gcovr --root . \
      --exclude 'tests/' \
      --exclude 'libs/p1_test/' \
      --fail-under-line 80 \
      --fail-under-branch 75 \
      --html-details build/coverage/index.html \
      --print-summary
```

* **Resultado medible:** Si el estudiante escribió tests que solo cubren el 50% de las ramas de su propia biblioteca, el comando falla (`--fail-under-line 80`) bloqueando la entrega.

---

### 7.7. Capa 5: Aislamiento Seguro en Sandbox (Bubblewrap)

Durante la corrección desatendida de cientos de entregas de estudiantes, el runner debe mitigar riesgos de:
* Bucles con llamadas a `fork()` infinitos (*fork bombs*).
* Sobrescritura de archivos del sistema o del docente (`rm -rf ~`).
* Intentos de conexión a Internet para enviar código o datos.

#### Invocación segura mediante `bwrap`:
```bash
bwrap \
  --ro-bind /usr /usr \
  --ro-bind /lib /lib \
  --ro-bind /lib64 /lib64 \
  --ro-bind /bin /bin \
  --ro-bind /etc/alternatives /etc/alternatives \
  --ro-bind $(pwd) /workspace \
  --tmpfs /tmp \
  --unshare-all \
  --unshare-net \
  --die-with-parent \
  --dir /workspace \
  --chdir /workspace \
  /usr/bin/prlimit --as=134217728 --cpu=5 --nproc=32 -- \
  ./build/test_bin -t 3 -f
```

* **Restricciones aplicadas:**
  * `--unshare-net`: anulación completa del stack TCP/IP y sockets de red.
  * `--unshare-all`: aislamiento de IPC, UTS, PIDs y montajes.
  * `--ro-bind`: el sistema operativo y el workspace se montan como solo lectura; escrituras solo en un `/tmp` en RAM efímero.
  * `prlimit --as=134217728`: límite duro de memoria virtual de 128 MB.
  * `prlimit --cpu=5`: máximo 5 segundos de CPU totales.
  * `prlimit --nproc=32`: restricción estricta de subprocesos contra *fork bombs*.

---

### 7.8. Capa 6: Validación de Pruebas mediante Mutation Testing

Para evaluar si los tests provistos por el alumno son rigurosos, el toolchain inyecta mutaciones sintácticas en su código de producción (ej: cambiar `>` por `<`, invertir retornos booleanos, alterar constantes de bucle):

```
Código Original:          Mutante 1 (Inversión condicional):
if (valor > 0)            if (valor <= 0)
    return 1;                 return 1;
return 0;                 return 0;
```

#### Algoritmo de evaluación:
1. Se compila y ejecuta la suite de tests del alumno contra su solución original (debe pasar al 100%).
2. Se inyectan $M$ mutantes de forma automatizada en el código fuente.
3. Se recompila y ejecuta la suite contra cada mutante:
   * **Mutante Cazado (*Killed*):** Al menos una aserción de `p1_test` falló. Correcto.
   * **Mutante Sobreviviente (*Survived*):** Todos los tests pasaron a pesar de la falla inyectada. Indica un test case faltante.
4. **Puntaje de Mutación:**
   $$\text{Score} = \frac{\text{Mutantes Cazados}}{\text{Mutantes Totales}} \times 100$$
* Umbral de aprobación didáctico: $\ge 80\%$.

---

### 7.9. Capa 7: Automatización CI/CD (GitHub Actions / Moodle VPL)

#### Flujo unificado en `.github/workflows/p1_test.yml`:
```yaml
name: Evaluacion P1 Test Suite

on: [push, pull_request]

jobs:
  verificacion:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4

      - name: Instalar dependencias
        run: sudo apt-get update && sudo apt-get install -y valgrind clang-tidy gcovr libseccomp-dev bubblewrap

      - name: 1. Verificacion de Estilo y Lint
        run: make lint

      - name: 2. Pruebas con Sanitizers (ASan + UBSan)
        run: make sanitize

      - name: 3. Auditoria de Memoria con Valgrind
        run: make memcheck

      - name: 4. Evaluacion de Cobertura
        run: make coverage

      - name: 5. Ejecucion en Sandbox
        run: make sandbox-test

      - name: 6. Publicar Reporte de Cobertura
        uses: actions/upload-artifact@v4
        with:
          name: coverage-report
          path: build/coverage/
```

---

### 7.10. Targets de Integración en el `Makefile` Principal

Para que el estudiante y los docentes utilicen el toolchain con una única interfaz, se añaden las siguientes metas estandarizadas al [`Makefile`](file:///home/mrtin/dev/p1/practicas/plantillas/lib_test/Makefile):

```makefile
# --- Toolchain Externo p1_test ---

# 1. Compilación con Sanitizers
sanitize: CFLAGS += -fsanitize=address,undefined -fno-omit-frame-pointer -g
sanitize: clean $(TEST_BINS)
	@echo "=== Ejecutando pruebas bajo ASan + UBSan ==="
	@set -e; for bin in $(TEST_BINS); do ./$$bin; done

# 2. Análisis Estático
lint:
	@echo "=== Analizando código con cppcheck y clang-tidy ==="
	cppcheck --enable=warning,performance,portability --error-exitcode=1 -Iinclude tests/
	clang-tidy tests/*.c -- -std=c99 -Iinclude

# 3. Cobertura de Código
coverage: CFLAGS += --coverage -g
coverage: clean $(TEST_BINS)
	@echo "=== Generando métricas de cobertura ==="
	@set -e; for bin in $(TEST_BINS); do ./$$bin; done
	mkdir -p build/coverage
	gcovr --root . --html-details build/coverage/index.html --print-summary

# 4. Aislamiento Sandbox
sandbox-test: $(TEST_BINS)
	@echo "=== Ejecutando tests dentro de Sandbox Bubblewrap ==="
	@set -e; for bin in $(TEST_BINS); do \
		bwrap --ro-bind / / --tmpfs /tmp --unshare-all --unshare-net --die-with-parent ./$$bin; \
	done
```

