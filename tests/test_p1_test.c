/**
 * @file test_p1_test.c
 * @brief Pruebas unitarias para el framework p1_test.h.
 * 
 * Verifica que todas las aserciones tipadas funcionen correctamente,
 * que el runner capture los resultados y que las aserciones no se desactiven
 * bajo banderas como -DNDEBUG o -DDEBUG.
 */

#include "p1_test.h"
#include <string.h>

/* --- Pruebas de Aserciones Booleanas --- */
TEST(aserciones_booleanas) {
    ASSERT_TRUE(1 == 1);
    ASSERT_TRUE(10 > 5);
    ASSERT_TRUE_MSG(42 != 0, "42 debe ser distinto de cero");

    ASSERT_FALSE(0 == 1);
    ASSERT_FALSE(5 > 10);
    ASSERT_FALSE_MSG(0 != 0, "0 == 0 debe ser falso");
}

/* --- Pruebas de Aserciones de Enteros --- */
TEST(aserciones_enteros) {
    int a = 10;
    int b = 10;
    int c = 20;

    ASSERT_INT_EQ(10, a);
    ASSERT_INT_EQ(a, b);
    ASSERT_INT_EQ_MSG(20, c, "c debe valer 20");

    ASSERT_INT_NE(a, c);
    ASSERT_INT_NE_MSG(b, c, "b y c deben ser distintos");

    ASSERT_INT_LT(a, c);
    ASSERT_INT_LE(a, b);
    ASSERT_INT_LE(a, c);
    ASSERT_INT_GT(c, a);
    ASSERT_INT_GE(b, a);
    ASSERT_INT_GE(c, b);
}

/* --- Pruebas de Enteros sin Signo --- */
TEST(aserciones_unsigned) {
    size_t len = 5;
    unsigned int u = 100U;

    ASSERT_UINT_EQ(5, len);
    ASSERT_UINT_EQ(100U, u);
    ASSERT_UINT_NE(50U, u);
}

/* --- Pruebas de Coma Flotante --- */
TEST(aserciones_double) {
    double pi_aprox = 3.14159;
    double pi_real = 3.14159265;

    ASSERT_DOUBLE_EQ(pi_real, pi_aprox, 0.0001);
    ASSERT_DOUBLE_EQ_MSG(1.0, 0.999999, 0.001, "Aproximación a 1.0 dentro del margen");
    ASSERT_DOUBLE_NE(1.0, 2.0, 0.001);
}

/* --- Pruebas de Cadenas de Texto --- */
TEST(aserciones_strings) {
    const char *saludo = "hola mundo";
    const char *copia = "hola mundo";
    const char *otro = "chau mundo";
    const char *nulo1 = NULL;
    const char *nulo2 = NULL;

    ASSERT_STR_EQ(saludo, copia);
    ASSERT_STR_EQ(nulo1, nulo2);
    ASSERT_STR_NE(saludo, otro);
    ASSERT_STR_NE(saludo, nulo1);

    ASSERT_STR_CONTAINS(saludo, "mundo");
    ASSERT_STR_CONTAINS(saludo, "hola");
}

/* --- Pruebas de Punteros --- */
TEST(aserciones_punteros) {
    int valor = 123;
    int *ptr = &valor;
    int *nulo = NULL;

    ASSERT_PTR_NOT_NULL(ptr);
    ASSERT_PTR_NULL(nulo);
    ASSERT_PTR_EQ(&valor, ptr);
    ASSERT_PTR_NE(ptr, nulo);
}

/* --- Función auxiliar para verificar que setjmp/longjmp rescata asserts anidados --- */
static void funcion_auxiliar_que_valida(int x) {
    ASSERT_INT_GT(x, 0);
}

TEST(aserciones_en_funciones_auxiliares) {
    funcion_auxiliar_que_valida(10);
    funcion_auxiliar_que_valida(99);
}

/* --- Verificación de persistencia bajo NDEBUG --- */
TEST(verificacion_persistencia_evaluacion) {
    int evaluado = 0;
    /* Comprobar que la expresión dentro del assert SIEMPRE se evalúa */
    ASSERT_TRUE((evaluado = 1) == 1);
    ASSERT_INT_EQ(1, evaluado);
}

int main(void) {
    TEST_SUITE_BEGIN("Suite de Autoverificación de p1_test");

    RUN_TEST(aserciones_booleanas);
    RUN_TEST(aserciones_enteros);
    RUN_TEST(aserciones_unsigned);
    RUN_TEST(aserciones_double);
    RUN_TEST(aserciones_strings);
    RUN_TEST(aserciones_punteros);
    RUN_TEST(aserciones_en_funciones_auxiliares);
    RUN_TEST(verificacion_persistencia_evaluacion);

    return TEST_REPORT();
}
