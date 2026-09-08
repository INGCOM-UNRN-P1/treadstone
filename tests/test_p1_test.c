/**
 * @file test_p1_test.c
 * @brief Pruebas unitarias para el framework p1_test.h con las mejoras QoL.
 */

#include "p1_test.h"
#include <string.h>

/* Variables de control para verificar hooks BEFORE_EACH y AFTER_EACH */
static int g_hook_counter = 0;

static void setup_hook(void) {
    g_hook_counter += 10;
}

static void teardown_hook(void) {
    g_hook_counter += 1;
}

/* --- Pruebas de Aserciones Booleanas --- */
TEST(aserciones_booleanas) {
    ASSERT_TRUE(1 == 1);
    ASSERT_TRUE(10 > 5);
    ASSERT_TRUE_MSG(42 != 0, "42 debe ser distinto de cero");

    ASSERT_FALSE(0 == 1);
    ASSERT_FALSE(5 > 10);
    ASSERT_FALSE_MSG(0 != 0, "0 == 0 debe ser falso");
}

/* --- Pruebas de Aserciones de Enteros y Rangos --- */
TEST(aserciones_enteros) {
    int a = 10;
    int b = 10;
    int c = 20;

    ASSERT_INT_EQ(10, a);
    ASSERT_INT_EQ(a, b);
    ASSERT_INT_EQ_MSG(20, c, "c debe valer 20");

    ASSERT_INT_NE(a, c);
    ASSERT_INT_LT(a, c);
    ASSERT_INT_LE(a, b);
    ASSERT_INT_GT(c, a);
    ASSERT_INT_GE(b, a);

    /* Aserción de rango numérico (QoL 14) */
    ASSERT_INT_BETWEEN(15, 10, 20);
    ASSERT_INT_BETWEEN(10, 10, 20);
    ASSERT_INT_BETWEEN(20, 10, 20);
}

/* --- Pruebas de Enteros sin Signo --- */
TEST(aserciones_unsigned) {
    size_t len = 5;
    unsigned int u = 100U;

    ASSERT_UINT_EQ(5, len);
    ASSERT_UINT_EQ(100U, u);
    ASSERT_UINT_NE(50U, u);
}

/* --- Pruebas de Coma Flotante y Tolerancia Relativa --- */
TEST(aserciones_double) {
    double pi_aprox = 3.14159;
    double pi_real = 3.14159265;

    ASSERT_DOUBLE_EQ(pi_real, pi_aprox, 0.0001);
    ASSERT_DOUBLE_EQ_MSG(1.0, 0.999999, 0.001, "Aproximación a 1.0 dentro del margen");
    ASSERT_DOUBLE_NE(1.0, 2.0, 0.001);

    /* Tolerancia relativa (QoL 12) */
    ASSERT_DOUBLE_NEAR_REL(1000000.0, 1000050.0, 0.0001);
    ASSERT_DOUBLE_NEAR_REL(0.0001, 0.00010001, 0.001);
}

/* --- Pruebas de Cadenas de Texto (incluye Case-Insensitive) --- */
TEST(aserciones_strings) {
    const char *saludo = "hola mundo";
    const char *copia = "hola mundo";
    const char *mayus = "HOLA MUNDO";
    const char *otro = "chau mundo";
    const char *nulo1 = NULL;
    const char *nulo2 = NULL;

    ASSERT_STR_EQ(saludo, copia);
    ASSERT_STR_EQ(nulo1, nulo2);
    ASSERT_STR_NE(saludo, otro);
    ASSERT_STR_NE(saludo, nulo1);

    ASSERT_STR_CONTAINS(saludo, "mundo");
    ASSERT_STR_CONTAINS(saludo, "hola");

    /* Comparación insensible a mayúsculas (QoL 15) */
    ASSERT_STR_CASE_EQ(saludo, mayus);
    ASSERT_STR_CASE_EQ("TEST_cmd", "test_CMD");
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

/* --- Pruebas de Arreglos de Enteros (QoL 11) --- */
TEST(aserciones_arreglos) {
    int esperado[] = {1, 2, 3, 4, 5};
    int obtenido[] = {1, 2, 3, 4, 5};
    ASSERT_ARRAY_INT_EQ(esperado, obtenido, 5);
}

/* --- Pruebas de Memoria Binaria con Hexdump (QoL 13) --- */
TEST(aserciones_memoria) {
    struct {
        char tag[4];
        int id;
    } s1 = {"ABC", 42}, s2 = {"ABC", 42};

    ASSERT_MEM_EQ(&s1, &s2, sizeof(s1));
}

/* --- Captura de salida estándar (QoL 18) --- */
#if _P1_HAS_POSIX
static void funcion_que_imprime(void) {
    printf("Hola desde funcion_que_imprime\n");
}

TEST(captura_stdout) {
    ASSERT_STDOUT_EQ(funcion_que_imprime(), "Hola desde funcion_que_imprime\n");
}
#endif

/* --- Subcasos descriptivos (QoL 9) --- */
TEST(subcasos_descriptivos) {
    SUBCASE("Caso A: valor positivo");
    ASSERT_INT_GT(10, 0);

    SUBCASE("Caso B: valor negativo");
    ASSERT_INT_LT(-5, 0);
}

/* --- Test con semilla determinística (QoL 19) --- */
TEST(semilla_pseudoaleatoria) {
    int r1 = rand();
    int r2 = rand();
    ASSERT_INT_NE(r1, r2);
}

/* --- Verificación de persistencia bajo NDEBUG --- */
TEST(verificacion_persistencia_evaluacion) {
    int evaluado = 0;
    ASSERT_TRUE((evaluado = 1) == 1);
    ASSERT_INT_EQ(1, evaluado);
}

/* --- Test salteado dinámicamente desde el cuerpo (QoL 10) --- */
TEST(test_que_se_saltea_adentro) {
    ASSERT_TRUE(1 == 1);
    TEST_SKIP("Omitido por requerir recurso de red");
    ASSERT_FAIL("Esta línea nunca debe ejecutarse");
}

int main(int argc, char **argv) {
    TEST_SUITE_BEGIN_ARGS("Suite Completa p1_test QoL", argc, argv);

    BEFORE_EACH(setup_hook);
    AFTER_EACH(teardown_hook);

    RUN_TEST(aserciones_booleanas);
    RUN_TEST(aserciones_enteros);
    RUN_TEST(aserciones_unsigned);
    RUN_TEST(aserciones_double);
    RUN_TEST(aserciones_strings);
    RUN_TEST(aserciones_punteros);
    RUN_TEST(aserciones_arreglos);
    RUN_TEST(aserciones_memoria);
#if _P1_HAS_POSIX
    RUN_TEST(captura_stdout);
#endif
    RUN_TEST(subcasos_descriptivos);
    RUN_TEST_SEEDED(semilla_pseudoaleatoria, 12345);
    RUN_TEST(verificacion_persistencia_evaluacion);
    RUN_TEST(test_que_se_saltea_adentro);

    /* Test salteado desde la suite (QoL 10) */
    SKIP_TEST(test_omitido_desde_suite, "Aún no implementado por el alumno");

    /* Comprobar que los hooks se ejecutaron */
    if (g_hook_counter == 0) {
        fprintf(stderr, "Error: los hooks BEFORE/AFTER_EACH no corrieron!\n");
        return 1;
    }

    return TEST_REPORT();
}
