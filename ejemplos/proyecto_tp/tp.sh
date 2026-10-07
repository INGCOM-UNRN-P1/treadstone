#!/usr/bin/env bash

# tp.sh - Gestor dinámico de librerías y ejercicios para Trabajos Prácticos de Programación 1
# Diseñado en español rioplatense.

set -euo pipefail

# Colores para la consola
VERDE='\033[0;32m'
AZUL='\033[0;34m'
AMARILLO='\033[1;33m'
ROJO='\033[0;31m'
NC='\033[0m' # Sin Color

# Directorios base
LIBS_DIR="libs"
EX_DIR="ejercicios"

# Mensaje de ayuda
mostrar_ayuda() {
    echo -e "${AZUL}Gestor del Trabajo Práctico (tp.sh)${NC}"
    echo -e "Uso: ./tp.sh <comando> [argumentos]\n"
    echo "Comandos disponibles:"
    echo -e "  ${VERDE}sync${NC}                            Sincroniza y regenera todos los Makefiles del proyecto."
    echo -e "  ${VERDE}add-lib <nombre> [url-git]${NC}      Agrega una librería. Si pasás una URL de Git, la clona."
    echo -e "                                    Si no, crea una plantilla local."
    echo -e "  ${VERDE}remove-lib <nombre>${NC}             Remueve una librería del proyecto."
    echo -e "  ${VERDE}add-ex <nombre> [libs]${NC}          Agrega un ejercicio localmente. 'libs' es una lista de librerías"
    echo -e "                                    separadas por comas (ej: arreglos,cadenas)."
    echo -e "  ${VERDE}remove-ex <nombre>${NC}              Remueve un ejercicio del proyecto."
    echo -e "  ${VERDE}list${NC}                            Lista todas las librerías y ejercicios instalados."
    echo -e "  ${VERDE}build${NC}                           Compila todo el proyecto."
    echo -e "  ${VERDE}run [ejercicio]${NC}                 Ejecuta un ejercicio en particular o todos si no indicás nada."
    echo -e "  ${VERDE}test [nombre]${NC}                   Ejecuta los tests de un ejercicio o librería específica,"
    echo -e "                                    o de todo el proyecto si no indicás nada."
    echo -e "  ${VERDE}ripley [modulo]${NC}                 Audita con el motor Ripley (análisis AST, reglas P1 y"
    echo -e "                                    AddressSanitizer) un ejercicio, librería o archivo .c,"
    echo -e "                                    o todo el proyecto si no indicás nada."
    echo -e "  ${VERDE}help${NC}                            Muestra este mensaje de ayuda."
}

# Verificar directorios base
mkdir -p "$LIBS_DIR" "$EX_DIR"

# Función para chequear si un argumento es una URL de Git
es_url_git() {
    local url="$1"
    if [[ "$url" =~ ^(https?://|git@|git://) ]] || [[ "$url" == *.git ]]; then
        return 0
    else
        return 1
    fi
}

# Generar Makefile de la raíz
escribir_makefile_raiz() {
    cat << 'EOF' > Makefile
# Makefile Principal Dinámico

# Detectar librerías en libs/
LIB_DIRS := $(wildcard libs/*)

# Detectar ejercicios en ejercicios/
EX_DIRS := $(wildcard ejercicios/*)

.PHONY: all librerias clean run test memcheck fallos $(EX_DIRS) $(LIB_DIRS)

all: librerias $(EX_DIRS)

librerias: $(LIB_DIRS)

# Regla para compilar cada librería de forma independiente si tiene un Makefile
$(LIB_DIRS):
	@if [ -f $@/Makefile ]; then \
		echo "Compilando librería en $@..."; \
		$(MAKE) -C $@ || exit 1; \
	fi

# Regla para compilar cada ejercicio, asegurando que primero se compilen las librerías
$(EX_DIRS): librerias
	@if [ -f $@/Makefile ]; then \
		echo "Compilando ejercicio en $@..."; \
		$(MAKE) -C $@ || exit 1; \
	fi

run: librerias
	@echo "Ejecutando programas de ejercicios..."
	@for dir in $(EX_DIRS); do \
		if [ -f $$dir/Makefile ]; then \
			echo "--- Ejecutando $$dir ---"; \
			$(MAKE) -C $$dir run || exit 1; \
		fi; \
	done

# Corre el objetivo $(1) en cada librería y ejercicio sin cortar en la primera
# suite que falla: al final lista las que fallaron y sale con 1.
define recorrer_suites
	@fallas=""; \
	for dir in $(LIB_DIRS) $(EX_DIRS); do \
		if [ -f $$dir/Makefile ]; then \
			echo "--- $(2) $$dir ---"; \
			$(MAKE) -C $$dir $(1) </dev/null || fallas="$$fallas $$dir"; \
		fi; \
	done; \
	if [ -n "$$fallas" ]; then \
		echo "Fallaron:$$fallas"; \
		exit 1; \
	fi; \
	echo "Todas las suites terminaron bien."
endef

# Una librería que no compila no impide probar el resto (-k)
test:
	-@$(MAKE) --no-print-directory -k librerias
	$(call recorrer_suites,test,Probando)

memcheck:
	-@$(MAKE) --no-print-directory -k librerias
	$(call recorrer_suites,memcheck,Memcheck en)

# Inyección de fallos con vasquez (cada suite la saltea si no está instalado)
fallos:
	-@$(MAKE) --no-print-directory -k librerias
	$(call recorrer_suites,fallos,Fallos en)

clean:
	@echo "Limpiando todos los ejecutables, librerías estáticas y archivos objeto..."
	@for dir in $(LIB_DIRS) $(EX_DIRS); do \
		if [ -f $$dir/Makefile ]; then \
			$(MAKE) -C $$dir clean; \
		fi; \
	done

# Incluir personalizaciones locales si existen
-include local.mk
EOF
}

# Generar Makefile de librería
escribir_makefile_lib() {
    local destino="$1"
    local dependencias_test="${2:-p1_test}"
    cat << EOF > "$destino/Makefile"
# Variables generales
CC ?= gcc
CFLAGS ?= -Wall -Wextra -std=c11 -pedantic -g

# Dependencias para las pruebas (separadas por espacio)
TEST_LIB_NAME ?= $dependencias_test
# Solo las que están en libs/ (en TP1 no hay p1_test)
TEST_LIBS ?= \$(foreach lib,\$(TEST_LIB_NAME),\$(if \$(wildcard \$(LIBS_DIR)/\$(lib)),\$(lib)))

# Directorio raíz del proyecto para referenciar libs
ROOT_DIR = ../..
LIBS_DIR = \$(ROOT_DIR)/libs

# Detección dinámica de los directorios de headers y biblioteca según el esquema (plano o estructurado)
TEST_INCLUDE_DIRS = \$(foreach lib,\$(TEST_LIBS),\$(if \$(wildcard \$(LIBS_DIR)/\$(lib)/include),-I\$(LIBS_DIR)/\$(lib)/include -I\$(LIBS_DIR)/\$(lib),-I\$(LIBS_DIR)/\$(lib)))
TEST_LIBRARY_DIRS = \$(foreach lib,\$(TEST_LIBS),\$(if \$(wildcard \$(LIBS_DIR)/\$(lib)/build),-L\$(LIBS_DIR)/\$(lib)/build,\$(if \$(wildcard \$(LIBS_DIR)/\$(lib)/lib),-L\$(LIBS_DIR)/\$(lib)/lib,-L\$(LIBS_DIR)/\$(lib))))
TEST_LIBRARY = \$(addprefix -l,\$(TEST_LIBS))

# Fallos inyectados (opcionales): P1_FALLAR_EN usa los mocks de holden solo si
# holden está instalado y prueba.c la usa; "make fallos" corre vasquez si está.
HOLDEN_FUNCIONES = malloc fopen fread fwrite fclose
ifneq (\$(shell command -v holden 2>/dev/null),)
ifneq (\$(shell grep -l P1_FALLAR_EN prueba.c 2>/dev/null),)
TEST_CFLAGS += -DP1_HOLDEN
TEST_EXTRA_OBJS += holden_mocks.o
TEST_LDFLAGS += \$(foreach f,\$(HOLDEN_FUNCIONES),-Wl,--wrap=\$(f))
endif
endif

# Archivos comunes, se autodetectan los .c y .h
SRCS = \$(filter-out prueba.c holden_mocks.c, \$(wildcard *.c))
HDRS = \$(wildcard *.h)

# Nombre de la librería estática (se deduce del nombre de la carpeta)
LIB_NAME := \$(notdir \$(CURDIR))
LIBRARY_NAME = lib\$(LIB_NAME).a

# Archivos para tests
TEST_TARGET = test_bin
TEST_SRCS = \$(SRCS) prueba.c
TEST_OBJS = \$(TEST_SRCS:.c=.o)

# Archivos objeto de la librería
OBJS = \$(SRCS:.c=.o)

# Solo la librería: un prueba.c que no compila no frena a los ejercicios
all: \$(LIBRARY_NAME)

# Crear la librería estática .a
\$(LIBRARY_NAME): \$(OBJS)
	@echo "Generando librería estática \$@"
	ar rcs \$@ \$^

# Compilar el ejecutable de pruebas
\$(TEST_TARGET): \$(TEST_OBJS) \$(TEST_EXTRA_OBJS)
	@for lib in \$(TEST_LIBS); do \\
		if [ -d "\$(LIBS_DIR)/\$\$lib" ] && [ -f "\$(LIBS_DIR)/\$\$lib/Makefile" ] && [ ! -f "\$(LIBS_DIR)/\$\$lib/lib\$\$lib.a" ] && [ ! -f "\$(LIBS_DIR)/\$\$lib/build/lib\$\$lib.a" ]; then \\
			echo "Compilando dependencia \$\$lib..."; \\
			\$(MAKE) -C "\$(LIBS_DIR)/\$\$lib" >/dev/null 2>&1 || exit 1; \\
		fi; \\
	done
	@echo "Compilando \$@"
	\$(CC) \$(CFLAGS) -o \$@ \$(TEST_OBJS) \$(TEST_EXTRA_OBJS) \$(TEST_LIBRARY_DIRS) \$(TEST_LIBRARY) \$(TEST_LDFLAGS)

# Regla específica para compilar el archivo de pruebas con sus dependencias
prueba.o: prueba.c \$(HDRS)
	@echo "Compilando \$<"
	\$(CC) \$(CFLAGS) \$(TEST_CFLAGS) \$(TEST_INCLUDE_DIRS) -c \$<

# Regla genérica para compilar archivos .o a partir de .c
%.o: %.c \$(HDRS)
	@echo "Compilando \$<"
	\$(CC) \$(CFLAGS) -c \$<

# Mocks de holden para P1_FALLAR_EN
holden_mocks.c:
	holden generate \$(HOLDEN_FUNCIONES) -n 0 -o \$@ >/dev/null

# Inyección de fallos con vasquez
.PHONY: fallos
fallos: \$(TEST_TARGET)
	@if command -v vasquez >/dev/null 2>&1; then \\
		vasquez check ./\$(TEST_TARGET); \\
	else \\
		echo "vasquez no está instalado: se saltea la inyección de fallos"; \\
	fi

# Ejecutar las pruebas
.PHONY: test
test: \$(TEST_TARGET)
	@echo "Probando librería..."
	./\$(TEST_TARGET)

# Verificación con Valgrind
.PHONY: memcheck
memcheck: \$(TEST_TARGET)
	@echo "Verificando con Valgrind..."
	valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 ./\$(TEST_TARGET)

# Limpiar archivos objeto y ejecutables
.PHONY: clean
clean:
	@echo "Limpiando..."
	rm -f *.o holden_mocks.c \$(LIBRARY_NAME) \$(TEST_TARGET)

# Incluir personalizaciones locales si existen
-include local.mk
EOF
}

# Generar Makefile de ejercicio
escribir_makefile_ex() {
    local destino="$1"
    local dependencias="$2"
    cat << EOF > "$destino/Makefile"
# Variables generales
CC ?= gcc
CFLAGS ?= -Wall -Wextra -pedantic -std=c11 -g

# Nombres de las librerías que usa este ejercicio (separadas por espacio)
LIB_NAME ?= $dependencias

# Directorio raíz del proyecto para referenciar libs
ROOT_DIR = ../..

# Archivos de las librerías
LIBRARY = \$(addprefix -l,\$(LIB_NAME))

# Detección dinámica de los directorios de headers y biblioteca según el esquema (plano o estructurado)
INCLUDE_DIRS = \$(foreach lib,\$(LIB_NAME),\$(if \$(wildcard \$(ROOT_DIR)/libs/\$(lib)/include),-I\$(ROOT_DIR)/libs/\$(lib)/include,-I\$(ROOT_DIR)/libs/\$(lib)))
LIBRARY_DIRS = \$(foreach lib,\$(LIB_NAME),\$(if \$(wildcard \$(ROOT_DIR)/libs/\$(lib)/build),-L\$(ROOT_DIR)/libs/\$(lib)/build,\$(if \$(wildcard \$(ROOT_DIR)/libs/\$(lib)/lib),-L\$(ROOT_DIR)/libs/\$(lib)/lib,-L\$(ROOT_DIR)/libs/\$(lib))))

# p1_test para las pruebas, si está en libs/ y no figura ya en LIB_NAME
TEST_LIBS ?= \$(filter-out \$(LIB_NAME),\$(notdir \$(wildcard \$(ROOT_DIR)/libs/p1_test)))
TEST_INCLUDE_DIRS = \$(foreach lib,\$(TEST_LIBS),-I\$(ROOT_DIR)/libs/\$(lib)/include -I\$(ROOT_DIR)/libs/\$(lib))
TEST_LIBRARY_DIRS = \$(foreach lib,\$(TEST_LIBS),-L\$(ROOT_DIR)/libs/\$(lib)/build -L\$(ROOT_DIR)/libs/\$(lib))
TEST_LIBRARY = \$(addprefix -l,\$(TEST_LIBS))

# Fallos inyectados (opcionales): P1_FALLAR_EN usa los mocks de holden solo si
# holden está instalado y prueba.c la usa; "make fallos" corre vasquez si está.
HOLDEN_FUNCIONES = malloc fopen fread fwrite fclose
ifneq (\$(shell command -v holden 2>/dev/null),)
ifneq (\$(shell grep -l P1_FALLAR_EN prueba.c 2>/dev/null),)
TEST_CFLAGS += -DP1_HOLDEN
TEST_EXTRA_OBJS += holden_mocks.o
TEST_LDFLAGS += \$(foreach f,\$(HOLDEN_FUNCIONES),-Wl,--wrap=\$(f))
endif
endif

# Archivos comunes (código fuente y cabeceras del ejercicio)
SRCS = \$(filter-out main.c prueba.c holden_mocks.c, \$(wildcard *.c))
HDRS = \$(wildcard *.h)

# Objetivo del programa principal
PROG_TARGET = programa
PROG_SRCS = main.c
PROG_OBJS = \$(PROG_SRCS:.c=.o)

# Objetivo de los tests
TEST_TARGET = test_bin
TEST_SRCS = prueba.c
TEST_OBJS = \$(TEST_SRCS:.c=.o)

# Compilar ambos: programa y tests
all: \$(PROG_TARGET) \$(TEST_TARGET)

# Compilar el programa principal
\$(PROG_TARGET): \$(PROG_OBJS) \$(SRCS:.c=.o)
	\$(CC) \$(CFLAGS) -o \$@ \$(PROG_OBJS) \$(SRCS:.c=.o) \$(LIBRARY_DIRS) \$(LIBRARY)

# Compilar el ejecutable de pruebas
\$(TEST_TARGET): \$(TEST_OBJS) \$(SRCS:.c=.o) \$(TEST_EXTRA_OBJS)
	\$(CC) \$(CFLAGS) -o \$@ \$(TEST_OBJS) \$(SRCS:.c=.o) \$(TEST_EXTRA_OBJS) \$(LIBRARY_DIRS) \$(TEST_LIBRARY_DIRS) \$(LIBRARY) \$(TEST_LIBRARY) \$(TEST_LDFLAGS)

# El archivo de pruebas ve también los headers de p1_test
prueba.o: prueba.c \$(HDRS)
	@echo "Compilando \$<"
	\$(CC) \$(CFLAGS) \$(TEST_CFLAGS) \$(INCLUDE_DIRS) \$(TEST_INCLUDE_DIRS) -c \$<

# Regla genérica para compilar archivos .o a partir de .c
%.o: %.c \$(HDRS)
	@echo "Compilando \$<"
	\$(CC) \$(CFLAGS) \$(INCLUDE_DIRS) -c \$<

# Mocks de holden para P1_FALLAR_EN
holden_mocks.c:
	holden generate \$(HOLDEN_FUNCIONES) -n 0 -o \$@ >/dev/null

# Inyección de fallos con vasquez
.PHONY: fallos
fallos: \$(TEST_TARGET)
	@if command -v vasquez >/dev/null 2>&1; then \\
		vasquez check ./\$(TEST_TARGET); \\
	else \\
		echo "vasquez no está instalado: se saltea la inyección de fallos"; \\
	fi

# Ejecutar el programa principal
.PHONY: run
run: \$(PROG_TARGET)
	@echo "Ejecutando el programa"
	./\$(PROG_TARGET)

# Ejecutar las pruebas
.PHONY: test
test: \$(TEST_TARGET)
	@echo "Ejecutando pruebas"
	./\$(TEST_TARGET)

# Verificación con Valgrind
.PHONY: memcheck
memcheck: \$(TEST_TARGET)
	@echo "Ejecutando pruebas con Valgrind"
	valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 ./\$(TEST_TARGET)

# Limpiar archivos objeto y ejecutables
.PHONY: clean
clean:
	@echo "Limpiando archivos objeto y ejecutables..."
	rm -f *.o holden_mocks.c \$(PROG_TARGET) \$(TEST_TARGET)

# Incluir personalizaciones locales si existen
-include local.mk
EOF
}

# Comando de sincronización
sync_project() {
    echo -e "${AZUL}Sincronizando y regenerando todos los Makefiles...${NC}"
    
    # 1. Regenerar Makefile de la raíz
    escribir_makefile_raiz
    echo -e "Makefile principal -> ${VERDE}Regenerado${NC}"
    
    # 2. Regenerar Makefiles de librerías (solo si son planas, es decir, sin estructura compleja)
    if [ -d "$LIBS_DIR" ]; then
        for dir in "$LIBS_DIR"/*; do
            if [ -d "$dir" ]; then
                local lib_nombre
                lib_nombre=$(basename "$dir")
                if [ -f "$dir/library.spec" ] || [ -f "$dir/library.json" ] || [ -d "$dir/src" ] || [ -d "$dir/include" ]; then
                    echo -e "Librería estructurada '$lib_nombre' -> ${AMARILLO}Se conserva su Makefile original${NC}"
                else
                    local dep_test="p1_test"
                    if [ -f "$dir/Makefile" ]; then
                        local dep_ext
                        dep_ext=$( (grep -E '^(TEST_LIB_NAME|TEST_LIBS) \?=' "$dir/Makefile" || true) | head -n1 | cut -d'=' -f2- | tr -d '\r' | xargs)
                        if [ -n "$dep_ext" ]; then
                            dep_test="$dep_ext"
                        fi
                    fi
                    escribir_makefile_lib "$dir" "$dep_test"
                    echo -e "Librería plana '$lib_nombre' -> ${VERDE}Makefile Sincronizado${NC} (tests: ${AMARILLO}${dep_test:-ninguna}${NC})"
                fi
            fi
        done
    fi
    
    # 3. Regenerar Makefiles de ejercicios
    if [ -d "$EX_DIR" ]; then
        for dir in "$EX_DIR"/*; do
            if [ -d "$dir" ]; then
                local ex_nombre
                ex_nombre=$(basename "$dir")
                
                # Intentar extraer dependencias existentes del Makefile actual
                local dependencias=""
                if [ -f "$dir/Makefile" ]; then
                    dependencias=$(grep -E '^LIB_NAME \?=' "$dir/Makefile" | cut -d'=' -f2- | tr -d '\r' | xargs)
                    if [ -z "$dependencias" ]; then
                        dependencias=$(grep -E '^LIB_NAME =' "$dir/Makefile" | cut -d'=' -f2- | tr -d '\r' | xargs)
                    fi
                fi
                
                escribir_makefile_ex "$dir" "$dependencias"
                echo -e "Ejercicio '$ex_nombre' -> ${VERDE}Makefile Sincronizado${NC} (dependencias: ${AMARILLO}${dependencias:-ninguna}${NC})"
            fi
        done
    fi
    echo -e "${VERDE}¡Sincronización completada! Ya podés usar 'make' en cualquier nivel del proyecto.${NC}"
}

# Escribir prueba.c: con p1_test si está en libs/, si no con assert
# Uso: escribir_prueba <archivo> <titulo> [headers...]
escribir_prueba() {
    local archivo="$1"
    local titulo="$2"
    shift 2
    {
        echo "/*"
        echo " * Pruebas de $titulo"
        echo " */"
        if [ -d "$LIBS_DIR/p1_test" ]; then
            echo "#include \"p1_test.h\""
            for header in "$@"; do
                echo "#include \"$header.h\""
            done
            echo ""
            echo "TEST(prueba_inicial)"
            echo "{"
            echo "    // Agregá tus aserciones acá (ASSERT_INT_EQ, ASSERT_STR_EQ, ...)"
            echo "    ASSERT_TRUE(1);"
            echo "}"
            echo ""
            echo "int main(int argc, char **argv)"
            echo "{"
            echo "    TEST_SUITE_BEGIN_ARGS(\"$titulo\", argc, argv);"
            echo "    RUN_TEST(prueba_inicial);"
            echo "    return TEST_REPORT();"
            echo "}"
        else
            echo "#include <assert.h>"
            echo "#include <stdio.h>"
            for header in "$@"; do
                echo "#include \"$header.h\""
            done
            echo ""
            echo "int main(void)"
            echo "{"
            echo "    printf(\"Corriendo pruebas de $titulo...\\n\");"
            echo "    // Agregá tus aserciones acá"
            echo "    printf(\"¡Pruebas de $titulo pasaron con éxito!\\n\");"
            echo "    return 0;"
            echo "}"
        fi
    } > "$archivo"
}

# Agregar Librería
add_lib() {
    if [ -z "${1:-}" ]; then
        echo -e "${ROJO}Error: Che, te olvidaste de pasar el nombre de la librería.${NC}"
        echo "Uso: ./tp.sh add-lib <nombre> [url-git]"
        exit 1
    fi

    local nombre="$1"
    local destino="$LIBS_DIR/$nombre"

    if [ -d "$destino" ]; then
        echo -e "${AMARILLO}La librería '$nombre' ya existe en '$destino'.${NC}"
        exit 1
    fi

    if [ -n "${2:-}" ] && es_url_git "$2"; then
        local url="$2"
        echo -e "${AZUL}Clonando librería remota desde: $url...${NC}"
        git clone "$url" "$destino"
        
        # Validar y corregir nombre de directorio si difiere del especificado en la librería estructurada
        local nombre_real="$nombre"
        if [ -f "$destino/library.spec" ]; then
            local spec_lib_name
            spec_lib_name=$(grep -E '^LIB_NAME=' "$destino/library.spec" | cut -d'=' -f2- | tr -d '"' | tr -d "'" | tr -d '\r' | xargs)
            if [ -n "$spec_lib_name" ]; then
                nombre_real="$spec_lib_name"
            fi
        elif [ -f "$destino/library.json" ]; then
            local json_lib_name
            if command -v jq &> /dev/null; then
                json_lib_name=$(jq -r '.name' "$destino/library.json")
            else
                json_lib_name=$(grep -E '"name":' "$destino/library.json" | head -n1 | cut -d':' -f2 | tr -d '"' | tr -d ',' | tr -d ' ' | tr -d '\r')
            fi
            if [ -n "$json_lib_name" ]; then
                nombre_real="$json_lib_name"
            fi
        fi
        
        if [ "$nombre" != "$nombre_real" ]; then
            echo -e "${AMARILLO}Advertencia: El nombre solicitado '$nombre' no coincide con el nombre real de la librería '$nombre_real'.${NC}"
            local nuevo_destino="$LIBS_DIR/$nombre_real"
            if [ -d "$nuevo_destino" ]; then
                echo -e "${ROJO}Error: El directorio destino '$nuevo_destino' ya existe. No se puede renombrar.${NC}"
                rm -rf "$destino"
                exit 1
            else
                mv "$destino" "$nuevo_destino"
                destino="$nuevo_destino"
                nombre="$nombre_real"
                echo -e "${VERDE}Se renombró el directorio de la librería a '$nombre'.${NC}"
            fi
        fi
        
        # Regenerar Makefiles después de clonar
        sync_project
        echo -e "${VERDE}Librería '$nombre' clonada con éxito en '$destino'.${NC}"
    else
        echo -e "${AZUL}Creando plantilla para la librería local '$nombre'...${NC}"
        mkdir -p "$destino"

        # Escribir el Makefile
        escribir_makefile_lib "$destino"

        # Crear cabecera .h
        cat << EOF > "$destino/$nombre.h"
#ifndef $(echo "$nombre" | tr '[:lower:]' '[:upper:]')_H
#define $(echo "$nombre" | tr '[:lower:]' '[:upper:]')_H

// Declará tus funciones acá

#endif
EOF

        # Crear fuente .c
        cat << EOF > "$destino/$nombre.c"
#include "$nombre.h"
#include <stdio.h>

// Implementá tus funciones acá
EOF

        # Crear archivo de pruebas prueba.c
        escribir_prueba "$destino/prueba.c" "la librería '$nombre'" "$nombre"

        # Sincronizar el Makefile principal
        escribir_makefile_raiz
        echo -e "${VERDE}Librería '$nombre' creada correctamente en '$destino'.${NC}"
    fi
}

# Remover Librería
remove_lib() {
    if [ -z "${1:-}" ]; then
        echo -e "${ROJO}Error: Decime qué librería querés remover.${NC}"
        echo "Uso: ./tp.sh remove-lib <nombre>"
        exit 1
    fi

    local nombre="$1"
    local destino="$LIBS_DIR/$nombre"

    if [ ! -d "$destino" ]; then
        echo -e "${ROJO}Error: La librería '$nombre' no existe.${NC}"
        exit 1
    fi

    read -p "¿De verdad querés borrar '$nombre' en '$destino'? Se va a perder todo [s/N]: " -r confirmacion
    if [[ "$confirmacion" =~ ^[sS]$ ]]; then
        rm -rf "$destino"
        sync_project
        echo -e "${VERDE}Librería '$nombre' eliminada sin piedad.${NC}"
    else
        echo -e "${AMARILLO}Operación cancelada. Menos mal.${NC}"
    fi
}

# Agregar Ejercicio
add_ex() {
    if [ -z "${1:-}" ]; then
        echo -e "${ROJO}Error: Falta el nombre del ejercicio.${NC}"
        echo "Uso: ./tp.sh add-ex <nombre> [dependencias_libs]"
        exit 1
    fi

    local nombre="$1"
    local destino="$EX_DIR/$nombre"

    if [ -d "$destino" ]; then
        echo -e "${AMARILLO}El ejercicio '$nombre' ya existe en '$destino'.${NC}"
        exit 1
    fi

    local dependencias=""
    if [ -n "${2:-}" ]; then
        dependencias="${2//,/ }" # Reemplaza comas por espacios
    fi

    echo -e "${AZUL}Creando ejercicio local '$nombre'...${NC}"
    mkdir -p "$destino"

    # Escribir el Makefile
    escribir_makefile_ex "$destino" "$dependencias"

    # Crear main.c básico con includes correspondientes
    {
        echo "/*"
        echo " * Ejercicio: $nombre"
        echo " */"
        echo "#include <stdio.h>"
        echo "#include <assert.h>"
        echo ""
        for lib in $dependencias; do
            echo "#include \"$lib.h\""
        done
        echo ""
        echo "int main(void) {"
        echo "    printf(\"¡Hola desde el ejercicio '$nombre'!\\n\");"
        echo "    return 0;"
        echo "}"
    } > "$destino/main.c"

    # Crear prueba.c para tests del ejercicio
    # shellcheck disable=SC2086
    escribir_prueba "$destino/prueba.c" "el ejercicio '$nombre'" $dependencias

    # Sincronizar el Makefile principal
    escribir_makefile_raiz
    echo -e "${VERDE}Ejercicio '$nombre' creado correctamente en '$destino'.${NC}"
    if [ -n "$dependencias" ]; then
        echo -e "${AZUL}Configurado para usar las librerías: $dependencias${NC}"
    fi
}

# Remover Ejercicio
remove_ex() {
    if [ -z "${1:-}" ]; then
        echo -e "${ROJO}Error: Decime qué ejercicio querés remover.${NC}"
        echo "Uso: ./tp.sh remove-ex <nombre>"
        exit 1
    fi

    local nombre="$1"
    local destino="$EX_DIR/$nombre"

    if [ ! -d "$destino" ]; then
        echo -e "${ROJO}Error: El ejercicio '$nombre' no existe.${NC}"
        exit 1
    fi

    read -p "¿De verdad querés borrar el ejercicio '$nombre' en '$destino'? [s/N]: " -r confirmacion
    if [[ "$confirmacion" =~ ^[sS]$ ]]; then
        rm -rf "$destino"
        sync_project
        echo -e "${VERDE}Ejercicio '$nombre' eliminado. Chau chau adiós.${NC}"
    else
        echo -e "${AMARILLO}Operación cancelada.${NC}"
    fi
}

# Listar
list_items() {
    echo -e "${AZUL}=== Librerías Instaladas ===${NC}"
    if [ -d "$LIBS_DIR" ] && [ "$(ls -A "$LIBS_DIR")" ]; then
        for dir in "$LIBS_DIR"/*; do
            if [ -d "$dir" ]; then
                echo -e " - ${VERDE}$(basename "$dir")${NC}"
            fi
        done
    else
        echo "  Ninguna librería instalada."
    fi

    echo ""
    echo -e "${AZUL}=== Ejercicios Instalados ===${NC}"
    if [ -d "$EX_DIR" ] && [ "$(ls -A "$EX_DIR")" ]; then
        for dir in "$EX_DIR"/*; do
            if [ -d "$dir" ]; then
                # Buscar dependencias leyendo el Makefile
                local deps=""
                if [ -f "$dir/Makefile" ]; then
                    deps=$(grep -E '^LIB_NAME \?=' "$dir/Makefile" | cut -d'=' -f2- | tr -d '\r' | xargs)
                fi
                if [ -n "$deps" ]; then
                    echo -e " - ${VERDE}$(basename "$dir")${NC} (usa: ${AMARILLO}$deps${NC})"
                else
                    echo -e " - ${VERDE}$(basename "$dir")${NC}"
                fi
            fi
        done
    else
        echo "  Ningún ejercicio instalado."
    fi
}

# Compilar
build_project() {
    echo -e "${AZUL}Compilando todo el proyecto TP...${NC}"
    make
}

# Ejecutar
run_project() {
    if [ -n "${1:-}" ]; then
        local ejercicio="$1"
        if [ -d "$EX_DIR/$ejercicio" ]; then
            echo -e "${AZUL}Ejecutando ejercicio: $ejercicio...${NC}"
            make -C "$EX_DIR/$ejercicio" run
        else
            echo -e "${ROJO}Error: Che, no encontré el ejercicio '$ejercicio'.${NC}"
            exit 1
        fi
    else
        echo -e "${AZUL}Ejecutando todos los ejercicios...${NC}"
        make run
    fi
}

# Probar
test_project() {
    if [ -n "${1:-}" ]; then
        local target="$1"
        if [ -d "$EX_DIR/$target" ]; then
            echo -e "${AZUL}Probando ejercicio: $target...${NC}"
            make -C "$EX_DIR/$target" test
        elif [ -d "$LIBS_DIR/$target" ]; then
            echo -e "${AZUL}Probando librería: $target...${NC}"
            make -C "$LIBS_DIR/$target" test
        else
            echo -e "${ROJO}Error: No se encontró ni el ejercicio ni la librería '$target'.${NC}"
            exit 1
        fi
    else
        echo -e "${AZUL}Probando todo el proyecto...${NC}"
        make test
    fi
}

# Auditar con el motor Ripley (Fase 3: integración entorno ↔ ripley)
resolver_ripley() {
    if command -v ripley >/dev/null 2>&1; then
        RIPLEY_CMD=(ripley)
    elif [ -n "${PORTABLE_ROOT:-}" ] && [ -x "$PORTABLE_ROOT/bin/ripley" ]; then
        RIPLEY_CMD=("$PORTABLE_ROOT/bin/ripley")
    elif [ -n "${PORTABLE_ROOT:-}" ] && [ -f "$PORTABLE_ROOT/bin/ripley.pyz" ]; then
        if command -v python3 >/dev/null 2>&1; then
            RIPLEY_CMD=(python3 "$PORTABLE_ROOT/bin/ripley.pyz")
        elif command -v python >/dev/null 2>&1; then
            RIPLEY_CMD=(python "$PORTABLE_ROOT/bin/ripley.pyz")
        else
            return 1
        fi
    else
        return 1
    fi
}

ripley_project() {
    if ! resolver_ripley; then
        echo -e "${ROJO}Error: El motor Ripley no está disponible en este entorno.${NC}"
        echo "Actualizá el entorno portable (update-env.sh) para descargar ripley.pyz,"
        echo "o instalalo manualmente con: uv tool install git+https://github.com/martinvilu/ripley"
        exit 1
    fi

    if [ -n "${1:-}" ]; then
        local target="$1"
        if [ -d "$EX_DIR/$target" ]; then
            echo -e "${AZUL}Auditando ejercicio: $target...${NC}"
            "${RIPLEY_CMD[@]}" check "$EX_DIR/$target"
        elif [ -d "$LIBS_DIR/$target" ]; then
            echo -e "${AZUL}Auditando librería: $target...${NC}"
            "${RIPLEY_CMD[@]}" check "$LIBS_DIR/$target"
        elif [ -f "$target" ] && [[ "$target" == *.c ]]; then
            echo -e "${AZUL}Auditando archivo: $target...${NC}"
            "${RIPLEY_CMD[@]}" check "$target"
        else
            echo -e "${ROJO}Error: No se encontró el ejercicio, la librería o el archivo .c '$target'.${NC}"
            exit 1
        fi
    else
        echo -e "${AZUL}Auditando todo el proyecto con Ripley...${NC}"
        local fallos=0
        local dir
        for dir in "$LIBS_DIR"/* "$EX_DIR"/*; do
            if [ -d "$dir" ]; then
                echo -e "${AZUL}--- Auditando $dir ---${NC}"
                if ! "${RIPLEY_CMD[@]}" check "$dir"; then
                    fallos=$((fallos + 1))
                fi
            fi
        done
        if [ "$fallos" -gt 0 ]; then
            echo -e "${ROJO}Ripley encontró observaciones en $fallos módulo(s).${NC}"
            exit 1
        fi
        echo -e "${VERDE}Auditoría completa sin errores bloqueantes.${NC}"
    fi
}

# Parsear comandos principales
if [ $# -lt 1 ]; then
    mostrar_ayuda
    exit 0
fi

cmd="$1"
shift

case "$cmd" in
    sync)
        sync_project
        ;;
    add-lib)
        add_lib "$@"
        ;;
    remove-lib)
        remove_lib "$@"
        ;;
    add-ex)
        add_ex "$@"
        ;;
    remove-ex)
        remove_ex "$@"
        ;;
    list)
        list_items
        ;;
    build)
        build_project
        ;;
    run)
        run_project "$@"
        ;;
    test)
        test_project "$@"
        ;;
    ripley)
        ripley_project "$@"
        ;;
    help|--help|-h)
        mostrar_ayuda
        ;;
    *)
        echo -e "${ROJO}Error: El comando '$cmd' no existe.${NC}"
        mostrar_ayuda
        exit 1
        ;;
esac
