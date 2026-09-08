# Makefile de la librería p1_test
CC ?= gcc
CFLAGS ?= -Wall -Wextra -Werror -std=c99 -pedantic -Iinclude
BUILD_DIR = build
TESTS_DIR = tests
EJEMPLO_DIR = ejemplos/proyecto_tp

# Descubrimiento automático de tests unitarios
TEST_SRCS = $(wildcard $(TESTS_DIR)/test_*.c)
TEST_BINS = $(patsubst $(TESTS_DIR)/%.c, $(BUILD_DIR)/%, $(TEST_SRCS))

.PHONY: all test test-ndebug test-debug test-ejemplo run-ejemplo memcheck clean

all: $(TEST_BINS) $(BUILD_DIR)/prueba

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%: $(TESTS_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/prueba: prueba.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

# Ejecutar todas las pruebas unitarias
test: $(TEST_BINS) $(BUILD_DIR)/prueba
	@set -e; for bin in $(TEST_BINS) $(BUILD_DIR)/prueba; do \
		echo "=== Ejecutando $$bin ==="; \
		./$$bin; \
	done

# Ejecutar pruebas con bandera NDEBUG (verificar persistencia de asserts)
test-ndebug: | $(BUILD_DIR)
	@echo "=== Compilando y ejecutando con -DNDEBUG ==="
	$(CC) $(CFLAGS) -DNDEBUG tests/test_p1_test.c -o $(BUILD_DIR)/test_p1_test_ndebug
	./$(BUILD_DIR)/test_p1_test_ndebug

# Ejecutar pruebas con bandera DEBUG
test-debug: | $(BUILD_DIR)
	@echo "=== Compilando y ejecutando con -DDEBUG ==="
	$(CC) $(CFLAGS) -DDEBUG tests/test_p1_test.c -o $(BUILD_DIR)/test_p1_test_debug
	./$(BUILD_DIR)/test_p1_test_debug

# Probar el proyecto de ejemplo basado en plantilla-TP
test-ejemplo:
	@echo "=== Probando proyecto de ejemplo TP ==="
	$(MAKE) -C $(EJEMPLO_DIR) test

# Ejecutar el programa del proyecto de ejemplo
run-ejemplo:
	@echo "=== Ejecutando programa del proyecto de ejemplo TP ==="
	$(MAKE) -C $(EJEMPLO_DIR) run

# Verificación de memoria con Valgrind (QoL 17)
memcheck: $(BUILD_DIR)/test_p1_test
	@echo "=== Verificando suite p1_test con Valgrind ==="
	valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 ./$(BUILD_DIR)/test_p1_test
	@echo "=== Verificando proyecto de ejemplo con Valgrind ==="
	$(MAKE) -C $(EJEMPLO_DIR) memcheck

clean:
	@echo "Limpiando directorio de compilación..."
	rm -rf $(BUILD_DIR)
	$(MAKE) -C $(EJEMPLO_DIR) clean
