/**
 * @file test_p1_advanced.c
 * @brief Pruebas para p1_arrays.h, p1_files.h y p1_stdio.h.
 */

#include <stdio.h>
#include <string.h>

#include "p1_test.h"
#include "p1_arrays.h"
#include "p1_files.h"
#include "p1_stdio.h"

/* --- Pruebas de p1_arrays.h --- */

TEST(arrays_ordenamiento) {
    int asc[] = {1, 3, 5, 5, 9, 12};
    int desc[] = {10, 8, 8, 4, 1, -2};

    ASSERT_ARRAY_INT_SORTED_ASC(asc, 6);
    ASSERT_ARRAY_INT_SORTED_DESC(desc, 6);
}

TEST(arrays_doubles) {
    double exp[] = {1.0, 2.5, 3.333};
    double act[] = {1.0001, 2.4999, 3.3332};

    ASSERT_ARRAY_DOUBLE_EQ(exp, act, 3, 0.001);
}

TEST(arrays_strings_argv) {
    const char *args1[] = {"gcc", "-Wall", "-O2", NULL};
    const char *args2[] = {"gcc", "-Wall", "-O2", NULL};

    ASSERT_STR_ARRAY_EQ(args1, args2);
}

TEST(arrays_contencion) {
    int arr[] = {10, 20, 30, 40};

    ASSERT_ARRAY_INT_CONTAINS(arr, 4, 30);
    ASSERT_ARRAY_INT_NOT_CONTAINS(arr, 4, 99);
}

/* --- Pruebas de p1_files.h --- */

TEST(archivos_texto_y_binarios) {
    const char *f1_path = "/tmp/p1_test_f1.txt";
    const char *f2_path = "/tmp/p1_test_f2.txt";
    const char *f_inexistente = "/tmp/p1_test_no_existe_123.txt";

    FILE *f1 = fopen(f1_path, "w");
    FILE *f2 = fopen(f2_path, "w");
    ASSERT_PTR_NOT_NULL(f1);
    ASSERT_PTR_NOT_NULL(f2);

    fputs("Linea 1: Programacion 1\nLinea 2: UNRN\n", f1);
    fputs("Linea 1: Programacion 1\nLinea 2: UNRN\n", f2);
    fclose(f1);
    fclose(f2);

    ASSERT_FILE_EXISTS(f1_path);
    ASSERT_FILE_NOT_EXISTS(f_inexistente);
    ASSERT_FILE_EQ(f1_path, f2_path);
    ASSERT_FILE_CONTAINS(f1_path, "UNRN");
    ASSERT_FILE_BINARY_EQ(f1_path, f2_path);

    remove(f1_path);
    remove(f2_path);
}

/* --- Pruebas de p1_stdio.h (Mocks y Capturas) --- */

static void funcion_con_printf(void) {
    printf("Salida formateada: %d", 42);
}

static void funcion_con_eprintf(void) {
    fprintf(stderr, "Aviso de advertencia");
}

static void funcion_interactiva(void) {
    int edad = 0;
    char nombre[32] = {0};
    if (scanf("%31s %d", nombre, &edad) == 2) {
        printf("Hola %s, tenes %d anios.\n", nombre, edad);
    }
}

TEST(stdio_captura_stdout) {
    ASSERT_STDOUT_EQ(funcion_con_printf(), "Salida formateada: 42");
}

TEST(stdio_captura_stderr) {
    ASSERT_STDERR_EQ(funcion_con_eprintf(), "Aviso de advertencia");
}

TEST(stdio_mock_interactivo) {
    ASSERT_STDIO_EQ(funcion_interactiva(), "Carlos 21\n", "Hola Carlos, tenes 21 anios.\n");
}

TEST(stdio_consumo_completo) {
    p1_mock_stdin_feed("123\n");
    int val = 0;
    int ret = scanf("%d\n", &val);
    ASSERT_INT_EQ(1, ret);
    ASSERT_INT_EQ(123, val);
    ASSERT_STDIN_CONSUMED();
    p1_mock_stdin_restore();
}

int main(int argc, char **argv) {
    TEST_SUITE_BEGIN_ARGS("Suite Avanzada: Arrays, Files y Stdio Mocks", argc, argv);

    RUN_TEST(arrays_ordenamiento);
    RUN_TEST(arrays_doubles);
    RUN_TEST(arrays_strings_argv);
    RUN_TEST(arrays_contencion);

    RUN_TEST(archivos_texto_y_binarios);

    RUN_TEST(stdio_captura_stdout);
    RUN_TEST(stdio_captura_stderr);
    RUN_TEST(stdio_mock_interactivo);
    RUN_TEST(stdio_consumo_completo);

    return TEST_REPORT();
}
