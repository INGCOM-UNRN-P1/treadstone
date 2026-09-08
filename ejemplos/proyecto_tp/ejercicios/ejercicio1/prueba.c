/**
 * @file prueba.c
 * @brief Suite de pruebas unitarias para el Ejercicio 1 utilizando p1_test.h.
 */

#include <stdio.h>
#include "p1_test.h"
#include "operaciones.h"

/* --- Pruebas de paridad (Aserciones booleanas y subcasos) --- */
TEST(prueba_es_par) {
    SUBCASE("Números pares");
    ASSERT_TRUE(es_par(0));
    ASSERT_TRUE(es_par(2));
    ASSERT_TRUE(es_par(-4));

    SUBCASE("Números impares");
    ASSERT_FALSE(es_par(1));
    ASSERT_FALSE(es_par(7));
    ASSERT_FALSE(es_par(-9));
}

/* --- Pruebas de factorial (Aserciones de enteros y rangos) --- */
TEST(prueba_factorial) {
    ASSERT_INT_EQ(1, factorial(0));
    ASSERT_INT_EQ(1, factorial(1));
    ASSERT_INT_EQ(2, factorial(2));
    ASSERT_INT_EQ(6, factorial(3));
    ASSERT_INT_EQ(120, factorial(5));
    ASSERT_INT_EQ(720, factorial(6));

    /* Casos de error o bordes */
    ASSERT_INT_EQ(-1, factorial(-1));
    ASSERT_INT_LT(factorial(-5), 0);
    ASSERT_INT_BETWEEN(factorial(4), 20, 30);
}

/* --- Pruebas de promedio (Aserciones de coma flotante y tolerancia relativa) --- */
TEST(prueba_calcular_promedio) {
    double valores[] = {10.0, 15.0, 20.0};
    double res = 0.0;

    int ret = calcular_promedio(valores, 3, &res);
    ASSERT_INT_EQ(0, ret);
    ASSERT_DOUBLE_EQ(15.0, res, 0.0001);
    ASSERT_DOUBLE_NEAR_REL(15.0, res, 0.001);

    double un_solo_valor[] = {7.5};
    ret = calcular_promedio(un_solo_valor, 1, &res);
    ASSERT_INT_EQ(0, ret);
    ASSERT_DOUBLE_EQ(7.5, res, 0.0001);

    /* Casos inválidos */
    ASSERT_INT_EQ(-1, calcular_promedio(NULL, 5, &res));
    ASSERT_INT_EQ(-1, calcular_promedio(valores, 0, &res));
    ASSERT_INT_EQ(-1, calcular_promedio(valores, 3, NULL));
}

/* --- Pruebas de saludo (Aserciones de cadenas, case-insensitive y punteros) --- */
TEST(prueba_obtener_saludo) {
    char buffer[64];
    char *ret = obtener_saludo("Matias", buffer, sizeof(buffer));

    ASSERT_PTR_NOT_NULL(ret);
    ASSERT_PTR_EQ(buffer, ret);
    ASSERT_STR_EQ("Hola, Matias!", buffer);
    ASSERT_STR_CASE_EQ("hola, MATIAS!", buffer);
    ASSERT_STR_CONTAINS(buffer, "Matias");
    ASSERT_STR_NE("Hola, Juan!", buffer);

    /* Casos de error */
    ASSERT_PTR_NULL(obtener_saludo(NULL, buffer, sizeof(buffer)));
    ASSERT_PTR_NULL(obtener_saludo("Ana", NULL, sizeof(buffer)));
    ASSERT_PTR_NULL(obtener_saludo("Ana", buffer, 0));
    ASSERT_PTR_NULL(obtener_saludo("Ana", buffer, 5));
}

/* --- Pruebas de búsqueda (Aserciones de punteros e índices) --- */
TEST(prueba_buscar_elemento) {
    int arr[] = {10, 25, 42, 99};

    const int *hallado = buscar_elemento(arr, 4, 42);
    ASSERT_PTR_NOT_NULL(hallado);
    ASSERT_PTR_EQ(&arr[2], hallado);
    ASSERT_INT_EQ(42, *hallado);

    const int *no_hallado = buscar_elemento(arr, 4, 999);
    ASSERT_PTR_NULL(no_hallado);

    ASSERT_PTR_NULL(buscar_elemento(NULL, 4, 10));
}

int main(int argc, char **argv) {
    TEST_SUITE_BEGIN_ARGS("Suite de Pruebas: Ejercicio 1", argc, argv);

    RUN_TEST(prueba_es_par);
    RUN_TEST(prueba_factorial);
    RUN_TEST(prueba_calcular_promedio);
    RUN_TEST(prueba_obtener_saludo);
    RUN_TEST(prueba_buscar_elemento);

    return TEST_REPORT();
}
