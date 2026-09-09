/**
 * @file prueba.c
 * @brief Pruebas unitarias de autoverificación para la librería p1_test.
 *
 * Programación 1 - Ingeniería en Computación - UNRN
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "p1_test.h"
#include "p1_arrays.h"
#include "p1_files.h"
#include "p1_stdio.h"

/* --- Pruebas de aserciones básicas --- */
TEST(prueba_aserciones_basicas) {
    SUBCASE("Booleanas y valores lógicos");
    ASSERT_TRUE(10 > 2);
    ASSERT_FALSE(2 > 10);

    SUBCASE("Enteros y límites de rango");
    ASSERT_INT_EQ(42, 40 + 2);
    ASSERT_INT_NE(42, 0);
    ASSERT_INT_BETWEEN(15, 10, 20);

    SUBCASE("Números reales con tolerancia");
    ASSERT_DOUBLE_EQ(3.1415, 3.14159, 0.001);
    ASSERT_DOUBLE_NEAR_REL(1000.0, 1000.5, 0.001);

    SUBCASE("Cadenas de texto");
    ASSERT_STR_EQ("hola", "hola");
    ASSERT_STR_CASE_EQ("HOLA", "hola");
    ASSERT_STR_CONTAINS("programacion 1", "grama");

    SUBCASE("Punteros");
    int x = 5;
    ASSERT_PTR_NOT_NULL(&x);
    ASSERT_PTR_NULL(NULL);

    SUBCASE("Versión y runtime");
    p1_test_runtime_init();
    ASSERT_STR_EQ("2.0.0", p1_test_version());
}

/* --- Pruebas de aserciones de arreglos --- */
TEST(prueba_aserciones_arreglos) {
    int asc[] = {1, 2, 4, 8};
    int desc[] = {8, 4, 2, 1};

    ASSERT_ARRAY_INT_EQ(asc, asc, 4);
    ASSERT_ARRAY_INT_SORTED_ASC(asc, 4);
    ASSERT_ARRAY_INT_SORTED_DESC(desc, 4);
    ASSERT_ARRAY_INT_CONTAINS(asc, 4, 4);
    ASSERT_ARRAY_INT_NOT_CONTAINS(asc, 4, 99);

    /* Casos de frontera N=0 y N=1 */
    ASSERT_ARRAY_INT_EQ(NULL, NULL, 0);
    ASSERT_ARRAY_INT_SORTED_ASC(NULL, 0);
}

/* --- Pruebas de operaciones sobre archivos --- */
TEST(prueba_aserciones_archivos) {
    char f_path[128];
    snprintf(f_path, sizeof(f_path), "/tmp/p1_test_prueba_%d.txt", (int)getpid());

    FILE *f = fopen(f_path, "w");
    ASSERT_PTR_NOT_NULL(f);
    fputs("UNRN P1 Test Suite\n", f);
    fclose(f);

    ASSERT_FILE_EXISTS(f_path);
    ASSERT_FILE_CONTAINS(f_path, "UNRN P1");

    remove(f_path);
    ASSERT_FILE_NOT_EXISTS(f_path);
}

/* --- Pruebas de mocks y captura de E/S --- */
static void imprimir_mensaje(void) {
    printf("salida de prueba\n");
}

TEST(prueba_captura_y_mock_stdio) {
    ASSERT_STDOUT_EQ(imprimir_mensaje(), "salida de prueba\n");

    p1_mock_stdin_feed("12345\n");
    int num = 0;
    int ret = scanf("%d\n", &num);
    ASSERT_INT_EQ(1, ret);
    ASSERT_INT_EQ(12345, num);
    ASSERT_STDIN_CONSUMED();
    p1_mock_stdin_restore();
}

int main(int argc, char **argv) {
    TEST_SUITE_BEGIN_ARGS("Suite de Pruebas: Librería p1_test", argc, argv);

    RUN_TEST(prueba_aserciones_basicas);
    RUN_TEST(prueba_aserciones_arreglos);
    RUN_TEST(prueba_aserciones_archivos);
    RUN_TEST(prueba_captura_y_mock_stdio);

    return TEST_REPORT();
}
