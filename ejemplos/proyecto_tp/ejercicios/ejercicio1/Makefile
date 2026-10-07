# Variables generales
CC ?= gcc
CFLAGS ?= -Wall -Wextra -pedantic -std=c11 -g

# Nombres de las librerías que usa este ejercicio (separadas por espacio)
LIB_NAME ?= p1_test

# Directorio raíz del proyecto para referenciar libs
ROOT_DIR = ../..

# Archivos de las librerías
LIBRARY = $(addprefix -l,$(LIB_NAME))

# Detección dinámica de los directorios de headers y biblioteca según el esquema (plano o estructurado)
INCLUDE_DIRS = $(foreach lib,$(LIB_NAME),$(if $(wildcard $(ROOT_DIR)/libs/$(lib)/include),-I$(ROOT_DIR)/libs/$(lib)/include,-I$(ROOT_DIR)/libs/$(lib)))
LIBRARY_DIRS = $(foreach lib,$(LIB_NAME),$(if $(wildcard $(ROOT_DIR)/libs/$(lib)/build),-L$(ROOT_DIR)/libs/$(lib)/build,$(if $(wildcard $(ROOT_DIR)/libs/$(lib)/lib),-L$(ROOT_DIR)/libs/$(lib)/lib,-L$(ROOT_DIR)/libs/$(lib))))

# p1_test para las pruebas, si está en libs/ y no figura ya en LIB_NAME
TEST_LIBS ?= $(filter-out $(LIB_NAME),$(notdir $(wildcard $(ROOT_DIR)/libs/p1_test)))
TEST_INCLUDE_DIRS = $(foreach lib,$(TEST_LIBS),-I$(ROOT_DIR)/libs/$(lib)/include -I$(ROOT_DIR)/libs/$(lib))
TEST_LIBRARY_DIRS = $(foreach lib,$(TEST_LIBS),-L$(ROOT_DIR)/libs/$(lib)/build -L$(ROOT_DIR)/libs/$(lib))
TEST_LIBRARY = $(addprefix -l,$(TEST_LIBS))

# Fallos inyectados (opcionales): P1_FALLAR_EN usa los mocks de holden solo si
# holden está instalado y prueba.c la usa; "make fallos" corre vasquez si está.
HOLDEN_FUNCIONES = malloc fopen fread fwrite fclose
ifneq ($(shell command -v holden 2>/dev/null),)
ifneq ($(shell grep -l P1_FALLAR_EN prueba.c 2>/dev/null),)
TEST_CFLAGS += -DP1_HOLDEN
TEST_EXTRA_OBJS += holden_mocks.o
TEST_LDFLAGS += $(foreach f,$(HOLDEN_FUNCIONES),-Wl,--wrap=$(f))
endif
endif

# Archivos comunes (código fuente y cabeceras del ejercicio)
SRCS = $(filter-out main.c prueba.c holden_mocks.c, $(wildcard *.c))
HDRS = $(wildcard *.h)

# Objetivo del programa principal
PROG_TARGET = programa
PROG_SRCS = main.c
PROG_OBJS = $(PROG_SRCS:.c=.o)

# Objetivo de los tests
TEST_TARGET = test_bin
TEST_SRCS = prueba.c
TEST_OBJS = $(TEST_SRCS:.c=.o)

# Compilar ambos: programa y tests
all: $(PROG_TARGET) $(TEST_TARGET)

# Compilar el programa principal
$(PROG_TARGET): $(PROG_OBJS) $(SRCS:.c=.o)
	$(CC) $(CFLAGS) -o $@ $(PROG_OBJS) $(SRCS:.c=.o) $(LIBRARY_DIRS) $(LIBRARY)

# Compilar el ejecutable de pruebas
$(TEST_TARGET): $(TEST_OBJS) $(SRCS:.c=.o) $(TEST_EXTRA_OBJS)
	$(CC) $(CFLAGS) -o $@ $(TEST_OBJS) $(SRCS:.c=.o) $(TEST_EXTRA_OBJS) $(LIBRARY_DIRS) $(TEST_LIBRARY_DIRS) $(LIBRARY) $(TEST_LIBRARY) $(TEST_LDFLAGS)

# El archivo de pruebas ve también los headers de p1_test
prueba.o: prueba.c $(HDRS)
	@echo "Compilando $<"
	$(CC) $(CFLAGS) $(TEST_CFLAGS) $(INCLUDE_DIRS) $(TEST_INCLUDE_DIRS) -c $<

# Regla genérica para compilar archivos .o a partir de .c
%.o: %.c $(HDRS)
	@echo "Compilando $<"
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) -c $<

# Mocks de holden para P1_FALLAR_EN
holden_mocks.c:
	holden generate $(HOLDEN_FUNCIONES) -n 0 -o $@ >/dev/null

# Inyección de fallos con vasquez
.PHONY: fallos
fallos: $(TEST_TARGET)
	@if command -v vasquez >/dev/null 2>&1; then \
		vasquez check ./$(TEST_TARGET); \
	else \
		echo "vasquez no está instalado: se saltea la inyección de fallos"; \
	fi

# Ejecutar el programa principal
.PHONY: run
run: $(PROG_TARGET)
	@echo "Ejecutando el programa"
	./$(PROG_TARGET)

# Ejecutar las pruebas
.PHONY: test
test: $(TEST_TARGET)
	@echo "Ejecutando pruebas"
	./$(TEST_TARGET)

# Verificación con Valgrind
.PHONY: memcheck
memcheck: $(TEST_TARGET)
	@echo "Ejecutando pruebas con Valgrind"
	valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 ./$(TEST_TARGET)

# Limpiar archivos objeto y ejecutables
.PHONY: clean
clean:
	@echo "Limpiando archivos objeto y ejecutables..."
	rm -f *.o holden_mocks.c $(PROG_TARGET) $(TEST_TARGET)

# Incluir personalizaciones locales si existen
-include local.mk
