# Especificación e Interrogación de Análisis: `p1-companion`

Companion App para orquestación, gestión de pruebas y generación de reportes analíticos sobre el framework [`p1_test.h`](file:///home/mrtin/dev/p1/practicas/plantillas/lib_test/include/p1_test.h) y el ecosistema de cátedra de Programación 1 (UNRN).

> **Aviso de alcance:** Este documento contiene **únicamente especificación funcional, arquitectura de software e interrogación analítica de diseño**. No contiene código de implementación.

---

## 1. Visión y Propósito

[`p1_test.h`](file:///home/mrtin/dev/p1/practicas/plantillas/lib_test/include/p1_test.h) opera como un motor de ejecución C11 puro en tiempo de compilación/ejecución nativa. Carece deliberadamente de capacidades de análisis estadístico, renderizado visual enriquecido, generación automática de esqueletos de prueba, integración directa con plataformas web (Moodle) y orquestación de mutaciones.

`p1-companion` es una herramienta CLI complementaria escrita en Python y gestionada mediante **UV**, cuyo propósito es desacoplar toda la lógica pesada de reportes, análisis pedagógico y automatización administrativa del código C de los alumnos.

---

## 2. Entorno y Empaquetado con `uv`

### 2.1. Configuración de Entorno
* **Gestor de entorno:** [`uv`](https://docs.astral.sh/uv/) (Astral) como único motor de empaquetado, dependencias y ejecución de entornos virtuales (`uv venv`, `uv run`, `uv tool`).
* **Runtime:** Python 3.11+.
* **Metadatos de proyecto:** Definidos en [`pyproject.toml`](file:///home/mrtin/dev/p1/practicas/plantillas/lib_test/pyproject.toml) bajo estándar PEP 621, usando `hatchling` como build backend.

### 2.2. Modos de Ejecución Previstos
1. **Modo Alumno (Cero configuración):**
   ```bash
   uv run p1 test run
   ```
   `uv` resuelve dependencias en un entorno efímero aislado sin requerir `pip install` global ni activación manual de virtualenv.
2. **Modo Cátedra / Evaluación Masiva (`uv tool`):**
   ```bash
   uv tool install --editable ./companion
   p1 grade batch --input-dir submissions/
   ```

### 2.3. Árbol de Paquete Proyectado
```
companion/
├── pyproject.toml
├── README.md
└── p1_companion/
    ├── __init__.py
    ├── cli.py               # Entrada principal Typer/Click
    ├── core/
    │   ├── compiler.py      # Invocación de GCC/Clang y Makefile
    │   ├── runner.py        # Ejecución y supervisión de procesos
    │   ├── parser.py        # Ingesta de streams TAP v13 y JSON
    │   └── sandbox.py       # Wrapper de contención Bubblewrap
    ├── scaffold/
    │   ├── inspector.py     # Análisis de cabeceras C (.h)
    │   └── generator.py     # Síntesis de archivos test_*.c
    ├── reports/
    │   ├── terminal.py      # Tablas y diffs ANSI con Rich
    │   ├── markdown.py      # Resumen para GitHub Classroom / PRs
    │   ├── html.py          # Dashboard interactivo autocontenido
    │   └── vpl.py           # Formateador para Moodle VPL
    └── analytics/
        ├── linter.py        # Detección de antipatrones en tests C
        ├── mutants.py       # Inyección de mutantes y mutation score
        └── metrics.py       # Cobertura, tiempos y balance taxonómico
```

---

## 3. Especificación de Módulos Funcionales

### 3.1. Módulo `test`: Orquestación y Supervisión de Ejecución
* **Subcomandos:**
  * `p1 test run [ejercicio] [--filter <k>] [--timeout <s>] [--sanitize] [--memcheck]`:
    * Compila el ejercicio o suite correspondiente mediante `make`.
    * Invoca el binario resultante consumiendo la salida estructurada `--tap` o `--json`.
    * Supervisa variables de proceso en `/proc/<pid>/status` (consumo pico de memoria física RSS) en caso de ejecuciones que superen umbrales.
  * `p1 test watch [ejercicio]`:
    * Monitorea cambios en archivos `.c` y `.h` usando eventos `inotify`, recompilando y ejecutando la suite en tiempo real.

### 3.2. Módulo `report`: Generación y Renderizado de Informes
* **Subcomandos:**
  * `p1 report terminal [--summary|--verbose]`:
    * Interpreta el último volcado de ejecución y renderiza un dashboard con tablas de pruebas, badges por subcaso y diferencias unificadas sintácticas.
  * `p1 report html --output <ruta.html>`:
    * Genera un reporte HTML autocontenido (sin CDNs externos) con gráficos de tiempos, tasas de éxito y volcados hexadecimales colapsables.
  * `p1 report md --output <ruta.md>`:
    * Emite un informe estructurado compatible con comentarios de Pull Request en GitHub o devoluciones de tareas.

### 3.3. Módulo `scaffold`: Generador Pedagógico de Pruebas
* **Subcomandos:**
  * `p1 scaffold init <modulo.h>`:
    * Inspecciona las firmas de funciones del archivo de cabecera especificado.
    * Genera un archivo `tests/test_<modulo>.c` con la estructura canónica de [`p1_test.h`](file:///home/mrtin/dev/p1/practicas/plantillas/lib_test/include/p1_test.h), registrando casos nominales, casos borde (punteros `NULL`, valores $0$) y hooks `BEFORE_EACH`/`AFTER_EACH`.

### 3.4. Módulo `mutate`: Análisis de Robustez de Tests (Mutation Testing)
* **Subcomandos:**
  * `p1 mutate run <src_file.c> --test-bin <build/test_bin>`:
    * Genera mutaciones sintácticas AST o a nivel de token en el código fuente (inversión de comparadores relacionales, reemplazo de constantes, alteración de retornos booleanos).
    * Recompila y ejecuta la suite de tests provista por el estudiante contra cada variante mutada.
    * Calcula la tasa de mutantes interceptados (*kill rate*), evaluando si los tests del estudiante detectan errores intencionales.

### 3.5. Módulo `grade`: Corrección y Calificación Académica
* **Subcomandos:**
  * `p1 grade evaluate [--exam] [--rubric <rubrica.json>]`:
    * Ejecuta la suite bajo entorno enjaulado Bubblewrap.
    * Pondera pesos numéricos de pruebas y deduce penalizaciones por fugas de memoria reportadas por Valgrind o violaciones de estilo.
    * Emite formato de salida compatible con Moodle VPL: `Grade :=>> <nota>`.

---

## 4. Interrogación de Análisis y Trade-offs Críticos

Esta sección plantea los dilemas de diseño técnico y metodológico que deben resolverse antes de proceder con cualquier implementación:

### Bloque A: Arquitectura e Integración con el Sistema Operativo

1. **Protocolo de comunicación C $\leftrightarrow$ Python:**
   * *Pregunta:* ¿El companion app debe capturar la salida de texto plano (`--tap` / `--json` emitido por el binario C a través de stdout), o debe abrirse un canal de comunicación dedicado (ej: descriptor Unix de logging o socket local efímero)?
   * *Tensión de diseño:* Si el código de un alumno genera un `SIGSEGV` o corrompe los buffers de `stdio`, un canal en stdout compartido puede truncarse o mezclarse con volcados de error, impidiendo el parseo del JSON final.

2. **Rol frente al `Makefile` existente:**
   * *Pregunta:* ¿`p1-companion` debe invocar directamente `make test` / `make memcheck` delegando toda la compilación en GNU Make, o debe asumir el control directo de la compilación invocando `gcc`/`clang` mediante subprocesos?
   * *Tensión de diseño:* Depender de Make respeta la convención Unix y los scripts existentes de los alumnos, pero dificulta instrumentar flags dinámicos en tiempo real (ej: compilar 50 mutantes con nombres de binarios efímeros sin ensuciar el `Makefile`).

3. **Contención y Sandbox:**
   * *Pregunta:* ¿Debe el companion app imponer el sandbox Bubblewrap por defecto en cualquier ejecución local del alumno, o reservarse únicamente para modos de evaluación desatendida (`p1 grade`)?
   * *Tensión de diseño:* Ejecutar en sandbox en entornos de desarrollo local introduce fricción en distribuciones Linux sin namespaces de usuario habilitados y bloquea el uso de depuradores interactivos como GDB.

### Bloque B: Metodología Pedagógica y Fricción Estudiantil

4. **Nivel de detalle en el reporte vs. Ingeniería inversa:**
   * *Pregunta:* Al correr tests en modo práctica versus modo examen, ¿hasta qué punto el reporte generado por el companion app debe mostrar los valores de entrada y salida esperados?
   * *Tensión de diseño:* Si el companion app formatea un diff gráfico detallado del archivo esperado vs. obtenido, el alumno puede hardcodear sentencias `if` para satisfacer el caso sin resolver el problema general.

5. **Instalación y dependencias en máquinas de alumnos:**
   * *Pregunta:* ¿Es viable requerir `uv` y Python 3.11+ en las máquinas personales de los estudiantes de primer año (Programación 1)?
   * *Tensión de diseño:* Muchos ingresantes cuentan con distribuciones Linux desactualizadas o usan entornos mínimos. Si el companion app agrega fricción de instalación de Python/uv, los alumnos evitarán usar la herramienta. ¿Debe el framework C ser 100% autosuficiente sin requerir Python para el flujo básico?

6. **Calidad de Tests de Alumnos (Scaffolding vs. Autonomía):**
   * *Pregunta:* Si el comando `p1 scaffold init` genera automáticamente el 80% de los casos de prueba a partir del archivo `.h`, ¿no se neutraliza el objetivo pedagógico de que el estudiante aprenda a diseñar sus propios casos de frontera?
   * *Tensión de diseño:* Generar plantillas acelera el arranque, pero puede inducir pasividad analítica si el alumno se limita a rellenar números en código preconcebido.

### Bloque C: Análisis Estático y Mutation Testing

7. **Estrategia de análisis de código C desde Python:**
   * *Pregunta:* Para el scaffolding y la inyección de mutantes, ¿debe usarse un parser C formal (bindings de `libclang` o `tree-sitter-c`) o un analizador léxico/regex liviano?
   * *Tensión de diseño:* `libclang` impone dependencias nativas complejas en el empaquetado de Python y requiere conocer todas las rutas de cabeceras de sistema. Expresiones regulares o parsers basados en tokens son más livianos pero fallan ante macros complejas y directivas `#ifdef` anidadas.

8. **Costo computacional del Mutation Testing:**
   * *Pregunta:* Si un ejercicio genera 60 mutantes y la suite de tests tarda 500 ms por corrida, evaluar la mutación tardará al menos 30 segundos por entrega. ¿Cómo escalar esto para 150 alumnos en servidores de corrección masiva?
   * *Tensión de diseño:* Requiere paralelismo estricto (`multiprocessing`), pre-filtrado de mutantes equivalentes y selección selectiva de tests vinculados a la función modificada.

### Bloque D: Persistencia y Gestión de Datos

9. **Almacenamiento de histórico y telemetría local:**
   * *Pregunta:* ¿Debe el companion app registrar un archivo SQLite local (`.p1_history.db`) con cada corrida de tests del alumno?
   * *Tensión de diseño:* Un historial local permite graficar curvas de aprendizaje (ej: cuántos intentos demandó resolver un ejercicio con fallos de memoria). Sin embargo, añade complejidad de migración de esquemas y puede generar sospechas de invasión si los datos se sincronizan con la cátedra.

10. **Seguridad en integración con Moodle:**
    * *Pregunta:* ¿Cómo debe gestionar el companion app la autenticación si se desea subir notas o feedback directamente a Moodle?
    * *Tensión de diseño:* Almacenar tokens de API o contraseñas de docentes en scripts locales expone credenciales en repositorios compartidos. La exportación debe limitarse a formatos estándar (ZIP, CSV, o stdout VPL) sin gestión directa de credenciales de red.

---

## 5. Checkpoints de Validación y Criterios de Decisión

Antes de iniciar la codificación de cualquier componente de `p1-companion`, deben verificarse los siguientes checkpoints técnicos y pedagógicos:

| Checkpoint | Pregunta de Aceptación | Métrica / Evidencia Requerida |
| :--- | :--- | :--- |
| **CP-01: Cero Fricción Alumno** | ¿El alumno puede correr la herramienta con un solo comando sin instalar dependencias globales? | `uv run --quiet p1 ...` ejecuta en $< 1.5$ segundos en máquina limpia. |
| **CP-02: Robustez ante Crashes** | Si el binario C sufre `SIGSEGV` abrupto, ¿el companion app reporta el fallo limpiamente sin colapsar el proceso Python? | Excepción capturada; reporte con exit code `1` y código de señal decodificado. |
| **CP-03: Portabilidad POSIX** | ¿La herramienta funciona sin modificaciones en Debian, Ubuntu, Arch y Fedora? | Matrix de validación CI ejecutando sobre imágenes Docker de las 4 distribuciones. |
| **CP-04: Aislamiento Opcional** | ¿El runner detecta la presencia de `bwrap` y se degrada controladamente si no está instalado? | Flag `--sandbox` arroja advertencia explicativa en lugar de abortar con traceback. |
| **CP-05: Cero Dependencia Mandatoria** | ¿El framework C ([`p1_test.h`](file:///home/mrtin/dev/p1/practicas/plantillas/lib_test/include/p1_test.h)) sigue siendo 100% operativo sin el companion app? | `make test` continúa funcionando de forma autónoma con sus reportes nativos. |
