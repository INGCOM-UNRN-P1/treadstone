# Proyecto de Ejemplo: TP con Pruebas Unitarias `p1_test`

Este directorio contiene un proyecto completo estructurado según las especificaciones de la **`plantilla-TP`** de la cátedra de Programación 1 (UNRN), adaptado para ilustrar la integración y el uso del framework de pruebas **`p1_test`**.

---

## 📁 Estructura del Proyecto

* **`tp.sh`**: Script gestor dinámico del Trabajo Práctico (administra librerías, ejercicios, compilación y pruebas).
* **`Makefile`**: Orquestador principal dinámico que detecta automáticamente las librerías en `libs/` y los ejercicios en `ejercicios/`.
* **`libs/p1_test/`**: Módulo de la librería `p1_test`:
  * `include/p1_test.h`: Cabecera del framework de pruebas unitarias.
  * `Makefile`: Genera la biblioteca estática para resolver dependencias de enlace.
  * `library.spec`: Metadatos de integración.
* **`ejercicios/ejercicio1/`**: Ejercicio modelo con funciones matemáticas y de cadenas:
  * `operaciones.h` / `operaciones.c`: Lógica de negocio (paridad, factoriales, promedios, cadenas y búsqueda en arreglos).
  * `main.c`: Programa de ejecución principal (`./programa`).
  * `prueba.c`: Suite de pruebas unitarias implementada con `p1_test.h` (`./test_bin`).
  * `Makefile`: Compila el programa principal y el ejecutable de pruebas enlazando `p1_test`.

---

## 🚀 Comandos de Uso

### 1. Compilar todo el proyecto
```bash
make
# O utilizando el gestor del TP:
./tp.sh build
```

### 2. Ejecutar todas las pruebas unitarias
```bash
make test
# O utilizando el gestor del TP:
./tp.sh test
```

### 3. Ejecutar el programa principal
```bash
make run
# O utilizando el gestor del TP:
./tp.sh run ejercicio1
```

### 4. Limpiar artefactos compilados
```bash
make clean
```
