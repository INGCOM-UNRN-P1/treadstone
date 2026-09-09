# Especificación de la librería para integración por shell script

LIB_NAME="p1_test"
LIB_VERSION="2.0.0"
LIB_BUILD_CMD="make"
LIB_CLEAN_CMD="make clean"

# Archivos a exportar (formato: "origen:destino")
LIB_HEADERS=(
  "include/p1_test.h:include/p1_test.h"
  "include/p1_arrays.h:include/p1_arrays.h"
  "include/p1_files.h:include/p1_files.h"
  "include/p1_stdio.h:include/p1_stdio.h"
  "include/tda/contracts.h:include/tda/contracts.h"
)
LIB_BINARIES=(
  "build/libp1_test.a:lib/libp1_test.a"
)

