#!/bin/bash
set -euo pipefail

# Cargar especificación si existe para obtener LIB_NAME
LIB_NAME="plantilla"
if [ -f "library.spec" ]; then
    # shellcheck source=/dev/null
    source "library.spec"
fi

# Funciones de ayuda
usage() {
    echo "Uso: $0 <comando> [argumentos]"
    echo "Comandos disponibles:"
    echo "  rename <nuevo_nombre>   Renombra la librería y actualiza los archivos del proyecto."
    echo "  add-module <modulo>     Crea un nuevo módulo (.c en src/ y .h en include/)."
    echo "  add-test <nombre>       Agrega una nueva prueba unitaria."
    echo "  info                    Muestra la configuración actual de la librería."
    echo "  build                   Compila la librería, ejemplos y pruebas."
    echo "  test                    Ejecuta las pruebas unitarias."
    echo "  clean                   Limpia los archivos generados."
    exit 1
}

if [ "$#" -lt 1 ]; then
    usage
fi

COMMAND="$1"
shift

case "$COMMAND" in
    rename)
        if [ "$#" -lt 1 ]; then
            echo "Error: Se requiere el nuevo nombre de la librería."
            exit 1
        fi
        NEW_NAME="$1"
        OLD_NAME="${LIB_NAME}"
        
        if [ "$OLD_NAME" = "$NEW_NAME" ]; then
            echo "La librería ya tiene el nombre '$NEW_NAME'."
            exit 0
        fi
        
        echo "Renombrando librería de '${OLD_NAME}' a '${NEW_NAME}'..."
        
        # 1. Renombrar archivos físicos si existen
        if [ -f "include/${OLD_NAME}.h" ]; then
            mv "include/${OLD_NAME}.h" "include/${NEW_NAME}.h"
        fi
        if [ -f "src/${OLD_NAME}.c" ]; then
            mv "src/${OLD_NAME}.c" "src/${NEW_NAME}.c"
        fi
        if [ -f "tests/test_${OLD_NAME}.c" ]; then
            mv "tests/test_${OLD_NAME}.c" "tests/test_${NEW_NAME}.c"
        fi
        
        # 2. Reemplazar texto en archivos
        OLD_UPPER=$(echo "$OLD_NAME" | tr '[:lower:]' '[:upper:]')
        NEW_UPPER=$(echo "$NEW_NAME" | tr '[:lower:]' '[:upper:]')
        
        files_to_update=(
            "Makefile"
            "library.json"
            "library.spec"
            "README.md"
            "ejemplos/main.c"
        )
        
        # Agregar archivos renombrados si existen
        [ -f "include/${NEW_NAME}.h" ] && files_to_update+=("include/${NEW_NAME}.h")
        [ -f "src/${NEW_NAME}.c" ] && files_to_update+=("src/${NEW_NAME}.c")
        [ -f "tests/test_${NEW_NAME}.c" ] && files_to_update+=("tests/test_${NEW_NAME}.c")
        
        for f in "${files_to_update[@]}"; do
            if [ -f "$f" ]; then
                # Reemplazar guards en mayúscula
                sed -i "s/${OLD_UPPER}/${NEW_UPPER}/g" "$f"
                # Reemplazar nombre en minúscula
                sed -i "s/${OLD_NAME}/${NEW_NAME}/g" "$f"
                echo "Actualizado: $f"
            fi
        done
        
        echo "Librería renombrada con éxito."
        ;;
        
    add-module)
        if [ "$#" -lt 1 ]; then
            echo "Error: Se requiere el nombre del módulo."
            exit 1
        fi
        MODULE_NAME="$1"
        MODULE_UPPER=$(echo "$MODULE_NAME" | tr '[:lower:]' '[:upper:]')
        
        HEADER_FILE="include/${MODULE_NAME}.h"
        SRC_FILE="src/${MODULE_NAME}.c"
        
        if [ -f "$HEADER_FILE" ] || [ -f "$SRC_FILE" ]; then
            echo "Error: El módulo '${MODULE_NAME}' ya existe."
            exit 1
        fi
        
        # Crear header
        cat <<EOF > "$HEADER_FILE"
#ifndef ${MODULE_UPPER}_H
#define ${MODULE_UPPER}_H

// Declaraciones de funciones para el módulo ${MODULE_NAME}

#endif // ${MODULE_UPPER}_H
EOF
        
        # Crear src
        cat <<EOF > "$SRC_FILE"
#include "${MODULE_NAME}.h"

// Implementación de funciones para el módulo ${MODULE_NAME}
EOF
        
        echo "Módulo '${MODULE_NAME}' creado:"
        echo "  - $HEADER_FILE"
        echo "  - $SRC_FILE"
        ;;
        
    add-test)
        if [ "$#" -lt 1 ]; then
            echo "Error: Se requiere el nombre de la prueba."
            exit 1
        fi
        TEST_NAME="$1"
        TEST_FILE="tests/test_${LIB_NAME}.c"
        
        if [ ! -f "$TEST_FILE" ]; then
            echo "Error: No se encontró el archivo de pruebas '$TEST_FILE'."
            exit 1
        fi
        
        # Verificar si el test ya existe en el archivo
        if grep -q "void test_${TEST_NAME}" "$TEST_FILE"; then
            echo "Error: La prueba '${TEST_NAME}' ya existe."
            exit 1
        fi
        
        echo "Agregando prueba '${TEST_NAME}' a '${TEST_FILE}'..."
        
        # Insertar la función de prueba antes de int main
        TEMP_FILE=$(mktemp)
        awk -v name="${TEST_NAME}" '
        /int main/ {
            print "void test_" name "(void) {"
            print "    // TODO: Implementar aserciones"
            print "    assert(1 == 1);"
            print "    printf(\"test_" name ": PASSED\\n\");"
            print "}"
            print ""
        }
        { print }
        ' "$TEST_FILE" > "$TEMP_FILE"
        mv "$TEMP_FILE" "$TEST_FILE"
        
        # Insertar la llamada a la función dentro de main(void)
        TEMP_FILE=$(mktemp)
        awk -v name="${TEST_NAME}" '
        /Corriendo pruebas unitarias.../ {
            print
            print "    test_" name "();"
            next
        }
        { print }
        ' "$TEST_FILE" > "$TEMP_FILE"
        mv "$TEMP_FILE" "$TEST_FILE"
        
        echo "Prueba '${TEST_NAME}' agregada correctamente."
        ;;
        
    build)
        echo "Compilando proyecto..."
        make
        ;;
        
    test)
        echo "Ejecutando pruebas..."
        make test
        ;;
        
    info)
        echo "=== Configuración de la Librería ==="
        echo "Nombre:       ${LIB_NAME:-No definido}"
        echo "Versión:      ${LIB_VERSION:-No definida}"
        echo "Build CMD:    ${LIB_BUILD_CMD:-No definido}"
        echo "Clean CMD:    ${LIB_CLEAN_CMD:-No definido}"
        echo "Cabeceras exportadas:"
        if [ -n "${LIB_HEADERS+x}" ] && [ ${#LIB_HEADERS[@]} -gt 0 ]; then
            for h in "${LIB_HEADERS[@]}"; do
                echo "  - $h"
            done
        else
            echo "  Ninguna"
        fi
        echo "Binarios exportados:"
        if [ -n "${LIB_BINARIES+x}" ] && [ ${#LIB_BINARIES[@]} -gt 0 ]; then
            for b in "${LIB_BINARIES[@]}"; do
                echo "  - $b"
            done
        else
            echo "  Ninguno"
        fi
        ;;

    clean)
        echo "Limpiando compilación..."
        make clean
        ;;
        
    *)
        echo "Comando no reconocido: $COMMAND"
        usage
        ;;
esac
