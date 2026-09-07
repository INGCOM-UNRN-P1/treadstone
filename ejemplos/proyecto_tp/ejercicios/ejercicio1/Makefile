# Variables generales
CC ?= gcc
CFLAGS ?= -Wall -Wextra -pedantic -std=c99 -g

# Nombres de las librerías que usa este ejercicio (separadas por espacio)
LIB_NAME ?= p1_test

# Directorio raíz del proyecto para referenciar libs
ROOT_DIR = ../..

# Archivos de las librerías
LIBRARY = $(addprefix -l,$(LIB_NAME))

# Detección dinámica de los directorios de headers y biblioteca según el esquema (plano o estructurado)
INCLUDE_DIRS = $(foreach lib,$(LIB_NAME),$(if $(wildcard $(ROOT_DIR)/libs/$(lib)/include),-I$(ROOT_DIR)/libs/$(lib)/include,-I$(ROOT_DIR)/libs/$(lib)))
LIBRARY_DIRS = $(foreach lib,$(LIB_NAME),$(if $(wildcard $(ROOT_DIR)/libs/$(lib)/build),-L$(ROOT_DIR)/libs/$(lib)/build,$(if $(wildcard $(ROOT_DIR)/libs/$(lib)/lib),-L$(ROOT_DIR)/libs/$(lib)/lib,-L$(ROOT_DIR)/libs/$(lib))))

# Archivos comunes (código fuente y cabeceras del ejercicio)
SRCS = $(filter-out main.c prueba.c, $(wildcard *.c))
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
$(TEST_TARGET): $(TEST_OBJS) $(SRCS:.c=.o)
	$(CC) $(CFLAGS) -o $@ $(TEST_OBJS) $(SRCS:.c=.o) $(LIBRARY_DIRS) $(LIBRARY)

# Regla genérica para compilar archivos .o a partir de .c
%.o: %.c $(HDRS)
	@echo "Compilando $<"
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) -c $<

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

# Limpiar archivos objeto y ejecutables
.PHONY: clean
clean:
	@echo "Limpiando archivos objeto y ejecutables..."
	rm -f *.o $(PROG_TARGET) $(TEST_TARGET)

# Incluir personalizaciones locales si existen
-include local.mk
