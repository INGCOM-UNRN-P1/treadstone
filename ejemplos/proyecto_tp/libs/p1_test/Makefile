# Makefile para librería header-only p1_test
CC ?= gcc
CFLAGS ?= -Wall -Wextra -pedantic -std=c99 -g
BUILD_DIR = build
LIB_A = $(BUILD_DIR)/libp1_test.a

all: $(LIB_A)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Genera un archivo estático mínimo para resolver dependencias de enlazado -lp1_test
$(LIB_A): | $(BUILD_DIR)
	@echo "Empaquetando librería p1_test en $@"
	@echo "void _p1_test_stub(void) {}" | $(CC) $(CFLAGS) -x c -c - -o $(BUILD_DIR)/stub.o
	@ar rcs $@ $(BUILD_DIR)/stub.o

clean:
	@echo "Limpiando p1_test..."
	rm -rf $(BUILD_DIR)

test:
	@echo "p1_test es una librería header-only lista para ser incluida."

.PHONY: all clean test
