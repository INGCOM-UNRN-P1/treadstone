# Especificación de la librería para integración por shell script

LIB_NAME="p1_test"
LIB_VERSION="1.0.0"
LIB_BUILD_CMD="make"
LIB_CLEAN_CMD="make clean"

# Archivos a exportar (formato: "origen:destino")
LIB_HEADERS=(
  "include/p1_test.h:include/p1_test.h"
  "include/tda/contracts.h:include/tda/contracts.h"
)
LIB_BINARIES=()
