# Makefile para librería header-only p1_test
CC ?= gcc
CFLAGS ?= -Wall -Wextra -pedantic -std=c99 -g
LIB_NAME := $(notdir $(CURDIR))
LIBRARY_NAME = lib$(LIB_NAME).a
BUILD_DIR = build
LIB_A = $(BUILD_DIR)/$(LIBRARY_NAME)

all: $(LIBRARY_NAME) $(LIB_A)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Genera la librería estática en la raíz (estándar plantilla-TP) y en build/
$(LIBRARY_NAME):
	@echo "Empaquetando librería $(LIB_NAME) en $@"
	@echo "void _p1_test_stub(void) {}" | $(CC) $(CFLAGS) -x c -c - -o stub.o
	@ar rcs $@ stub.o
	@rm -f stub.o

$(LIB_A): $(LIBRARY_NAME) | $(BUILD_DIR)
	@cp $(LIBRARY_NAME) $(LIB_A)

clean:
	@echo "Limpiando $(LIB_NAME)..."
	rm -rf $(BUILD_DIR) $(LIBRARY_NAME) stub.o

test:
	@echo "$(LIB_NAME) es una librería header-only lista para ser incluida."

.PHONY: all clean test
