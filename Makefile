# Makefile de la librería p1_test
CC ?= gcc
CFLAGS ?= -Wall -Wextra -Werror -std=c11 -pedantic -Iinclude -fno-omit-frame-pointer
AR ?= ar
ARFLAGS ?= rcs

# Directorios
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
TESTS_DIR = tests
EJEMPLO_DIR = ejemplos/proyecto_tp

# Nombres de la librería
LIB_NAME = p1_test
LIBRARY_NAME = lib$(LIB_NAME).a
LIB_A = $(BUILD_DIR)/$(LIBRARY_NAME)
ROOT_LIB = $(LIBRARY_NAME)

# Fuentes y Objetos de la librería
LIB_SRCS = $(wildcard $(SRC_DIR)/*.c)
LIB_OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(LIB_SRCS))

# Descubrimiento automático de tests unitarios
TEST_SRCS = $(wildcard $(TESTS_DIR)/test_*.c)
TEST_BINS = $(patsubst $(TESTS_DIR)/%.c, $(BUILD_DIR)/%, $(TEST_SRCS))

.PHONY: all test test-silencioso test-ndebug test-debug test-holden ejemplo-p1_test test-ejemplo run-ejemplo memcheck clean

all: $(LIB_A) $(ROOT_LIB) $(TEST_BINS) $(BUILD_DIR)/prueba

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compilación de objetos de la librería
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Empaquetar la librería estática en build/
$(LIB_A): $(LIB_OBJS) | $(BUILD_DIR)
	$(AR) $(ARFLAGS) $@ $^

# Copiar la librería estática a la raíz para compatibilidad con esquemas planos
$(ROOT_LIB): $(LIB_A)
	cp $< $@

# Binarios de tests unitarios enlazados contra la librería estática
$(BUILD_DIR)/test_%: $(TESTS_DIR)/test_%.c $(LIB_A) $(wildcard $(INC_DIR)/*.h) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@ -L$(BUILD_DIR) -l$(LIB_NAME)

# Binario de prueba general enlazado contra la librería estática
$(BUILD_DIR)/prueba: prueba.c $(LIB_A) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@ -L$(BUILD_DIR) -l$(LIB_NAME)

# Ejecutar todas las pruebas unitarias
test: $(TEST_BINS) $(BUILD_DIR)/prueba
	@set -e; for bin in $(TEST_BINS) $(BUILD_DIR)/prueba; do \
		echo "=== Ejecutando $$bin ==="; \
		./$$bin; \
	done

# En modo silencioso cada fallo deja una línea P1_FALLO (o P1_CRASH) en stderr: la usa vassili
# para saber qué aserción mató a un mutante.
test-silencioso: $(BUILD_DIR)/test_p1_failures $(BUILD_DIR)/test_p1_signals
	@echo "=== Líneas estructuradas en modo silencioso ==="
	@./$(BUILD_DIR)/test_p1_failures -q 2>$(BUILD_DIR)/silencioso.err >/dev/null || true
	@n=$$(grep -c '^P1_FALLO	' $(BUILD_DIR)/silencioso.err); \
		test "$$n" -eq 6 || { echo "se esperaban 6 líneas P1_FALLO y hubo $$n"; exit 1; }
	@grep -q '^P1_FALLO	test_fallo_int_eq	tests/test_p1_failures.c:[0-9]*	ASSERT_INT_EQ(10, 20)$$' $(BUILD_DIR)/silencioso.err \
		|| { echo "la línea P1_FALLO no trae test, archivo:línea y aserción"; exit 1; }
	@./$(BUILD_DIR)/test_p1_signals -q 2>&1 >/dev/null | grep -q '^P1_CRASH	test_crash_sigsegv	SIGSEGV$$' \
		|| { echo "falta la línea P1_CRASH del SIGSEGV"; exit 1; }
	@echo "ok"

# Ejecutar pruebas con bandera NDEBUG (verificar persistencia de asserts)
test-ndebug: $(LIB_A) | $(BUILD_DIR)
	@echo "=== Compilando y ejecutando con -DNDEBUG ==="
	$(CC) $(CFLAGS) -DNDEBUG tests/test_p1_test.c -o $(BUILD_DIR)/test_p1_test_ndebug -L$(BUILD_DIR) -l$(LIB_NAME)
	./$(BUILD_DIR)/test_p1_test_ndebug

# Ejecutar pruebas con bandera DEBUG
test-debug: $(LIB_A) | $(BUILD_DIR)
	@echo "=== Compilando y ejecutando con -DDEBUG ==="
	$(CC) $(CFLAGS) -DDEBUG tests/test_p1_test.c -o $(BUILD_DIR)/test_p1_test_debug -L$(BUILD_DIR) -l$(LIB_NAME)
	./$(BUILD_DIR)/test_p1_test_debug

# P1_FALLAR_EN con los mocks de holden, si está instalado
HOLDEN_FUNCIONES = malloc fopen fread fwrite fclose
test-holden: $(LIB_A) | $(BUILD_DIR)
	@if command -v holden >/dev/null 2>&1; then \
		echo "=== Compilando y ejecutando con los mocks de holden ==="; \
		holden generate $(HOLDEN_FUNCIONES) -n 0 -o $(BUILD_DIR)/holden_mocks.c >/dev/null && \
		$(CC) $(CFLAGS) -DP1_HOLDEN tests/test_p1_holden.c $(BUILD_DIR)/holden_mocks.c -o $(BUILD_DIR)/test_p1_holden_mocks \
			-L$(BUILD_DIR) -l$(LIB_NAME) $(foreach f,$(HOLDEN_FUNCIONES),-Wl,--wrap=$(f)) && \
		./$(BUILD_DIR)/test_p1_holden_mocks; \
	else \
		echo "holden no está instalado: se saltea test-holden"; \
	fi

# El ejemplo usa los headers y fuentes actuales de p1_test (no una copia
# versionada que se desactualiza): se copian antes de compilarlo.
EJEMPLO_P1_TEST = $(EJEMPLO_DIR)/libs/p1_test
ejemplo-p1_test:
	@mkdir -p $(EJEMPLO_P1_TEST)/include $(EJEMPLO_P1_TEST)/src
	@cp $(INC_DIR)/*.h $(EJEMPLO_P1_TEST)/include/
	@cp $(SRC_DIR)/*.c $(EJEMPLO_P1_TEST)/src/

# Probar el proyecto de ejemplo basado en plantilla-TP
test-ejemplo: $(LIB_A) $(ROOT_LIB) ejemplo-p1_test
	@echo "=== Probando proyecto de ejemplo TP ==="
	$(MAKE) -C $(EJEMPLO_DIR) test

# Ejecutar el programa del proyecto de ejemplo
run-ejemplo: $(LIB_A) $(ROOT_LIB) ejemplo-p1_test
	@echo "=== Ejecutando programa del proyecto de ejemplo TP ==="
	$(MAKE) -C $(EJEMPLO_DIR) run

# Verificación de memoria con Valgrind (QoL 17)
memcheck: $(BUILD_DIR)/test_p1_test $(BUILD_DIR)/prueba ejemplo-p1_test
	@echo "=== Verificando suite p1_test con Valgrind ==="
	valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 ./$(BUILD_DIR)/test_p1_test
	@echo "=== Verificando suite prueba.c con Valgrind ==="
	valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 ./$(BUILD_DIR)/prueba
	@if [ -d "$(EJEMPLO_DIR)" ]; then \
		echo "=== Verificando proyecto de ejemplo con Valgrind ==="; \
		$(MAKE) -C $(EJEMPLO_DIR) memcheck; \
	fi

clean:
	@echo "Limpiando directorio de compilación..."
	rm -rf $(BUILD_DIR) $(ROOT_LIB)
	@if [ -d "$(EJEMPLO_DIR)" ]; then \
		$(MAKE) -C $(EJEMPLO_DIR) clean; \
	fi
