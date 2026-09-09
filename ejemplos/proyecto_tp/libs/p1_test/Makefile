# Makefile para librería p1_test
CC ?= gcc
CFLAGS ?= -Wall -Wextra -pedantic -std=c11 -g
LIB_NAME := $(notdir $(CURDIR))
LIBRARY_NAME = lib$(LIB_NAME).a
BUILD_DIR = build
SRC_DIR = src
INC_DIR = include
LIB_A = $(BUILD_DIR)/$(LIBRARY_NAME)

LIB_SRCS = $(wildcard $(SRC_DIR)/*.c)
LIB_OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(LIB_SRCS))

TEST_TARGET = test_bin
TEST_SRCS = prueba.c
TEST_OBJS = $(TEST_SRCS:.c=.o)

all: $(LIB_A) $(LIBRARY_NAME) $(TEST_TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compilar objetos de la librería
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -I. -I$(INC_DIR) -c $< -o $@

# Generar librería estática en build/ y copiar a la raíz
$(LIB_A): $(LIB_OBJS) | $(BUILD_DIR)
	ar rcs $@ $^

$(LIBRARY_NAME): $(LIB_A)
	cp $< $@

# Compilar ejecutable de pruebas de la librería enlazando contra la librería estática
$(TEST_TARGET): $(TEST_OBJS) $(LIB_A)
	@echo "Compilando pruebas de $(LIB_NAME)..."
	$(CC) $(CFLAGS) -I. -I$(INC_DIR) -o $@ $(TEST_OBJS) -L$(BUILD_DIR) -l$(LIB_NAME)

%.o: %.c
	@echo "Compilando $<"
	$(CC) $(CFLAGS) -I. -I$(INC_DIR) -c $< -o $@

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
