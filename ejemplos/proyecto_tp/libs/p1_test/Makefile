# Makefile para librería header-only p1_test
CC ?= gcc
CFLAGS ?= -Wall -Wextra -pedantic -std=c99 -g
LIB_NAME := $(notdir $(CURDIR))
LIBRARY_NAME = lib$(LIB_NAME).a
BUILD_DIR = build
LIB_A = $(BUILD_DIR)/$(LIBRARY_NAME)

TEST_TARGET = test_bin
TEST_SRCS = prueba.c
TEST_OBJS = $(TEST_SRCS:.c=.o)

all: $(LIBRARY_NAME) $(LIB_A) $(TEST_TARGET)

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

# Compilar ejecutable de pruebas de la librería
$(TEST_TARGET): $(TEST_OBJS)
	@echo "Compilando pruebas de $(LIB_NAME)..."
	$(CC) $(CFLAGS) -I. -Iinclude -o $@ $^

%.o: %.c
	@echo "Compilando $<"
	$(CC) $(CFLAGS) -I. -Iinclude -c $<

clean:
	@echo "Limpiando $(LIB_NAME)..."
	rm -rf $(BUILD_DIR) $(LIBRARY_NAME) $(TEST_TARGET) *.o

test: $(TEST_TARGET)
	@echo "Probando librería $(LIB_NAME)..."
	./$(TEST_TARGET)

memcheck: $(TEST_TARGET)
	@echo "Verificando $(LIB_NAME) con Valgrind..."
	valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 ./$(TEST_TARGET)

.PHONY: all clean test memcheck
