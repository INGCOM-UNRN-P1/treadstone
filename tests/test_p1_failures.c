/**
 * @file test_p1_failures.c
 * @brief Prueba de recuperación ante fallos de aserción y diagnósticos.
 * 
 * Verifica que cuando un assert falla:
 * 1. Aborta únicamente el caso de prueba actual y NO mata el proceso.
 * 2. La suite continúa ejecutando los tests subsiguientes.
 * 3. El reporte final contabiliza los fallos y retorna código 1.
 */

#include "p1_test.h"

TEST(test_fallo_int_eq) {
    ASSERT_INT_EQ(10, 20);
    ASSERT_FAIL("No debería alcanzarse");
}

TEST(test_fallo_int_between) {
    ASSERT_INT_BETWEEN(99, 10, 20);
    ASSERT_FAIL("No debería alcanzarse");
}

TEST(test_fallo_array) {
    int exp[] = {1, 2, 99, 4};
    int act[] = {1, 2, 3, 4};
    ASSERT_ARRAY_INT_EQ(exp, act, 4);
    ASSERT_FAIL("No debería alcanzarse");
}

TEST(test_fallo_memoria) {
    char b1[] = "HOLA";
    char b2[] = "HOTA";
    ASSERT_MEM_EQ(b1, b2, 4);
    ASSERT_FAIL("No debería alcanzarse");
}

TEST(test_fallo_double_rel) {
    ASSERT_DOUBLE_NEAR_REL(100.0, 110.0, 0.01); /* 10% diff vs 1% tol */
    ASSERT_FAIL("No debería alcanzarse");
}

TEST(test_fallo_str_case) {
    ASSERT_STR_CASE_EQ("manzana", "pera");
    ASSERT_FAIL("No debería alcanzarse");
}

TEST(test_que_pasa_al_final) {
    ASSERT_TRUE(1 == 1);
    ASSERT_STR_EQ("ok", "ok");
}

int main(int argc, char **argv) {
    TEST_SUITE_BEGIN_ARGS("Suite de Prueba de Recuperación ante Fallos", argc, argv);

    RUN_TEST(test_fallo_int_eq);
    RUN_TEST(test_fallo_int_between);
    RUN_TEST(test_fallo_array);
    RUN_TEST(test_fallo_memoria);
    RUN_TEST(test_fallo_double_rel);
    RUN_TEST(test_fallo_str_case);
    RUN_TEST(test_que_pasa_al_final);

    int exit_code = TEST_REPORT();
    /*
     * Esta suite está diseñada para que fallen 6 tests y pase 1.
     */
    if (exit_code == 1 && _p1_global_state.tests_failed == 6 && _p1_global_state.tests_passed == 1) {
        printf("Comportamiento verificado: Los 6 fallos fueron capturados y la suite continuo normalmente.\n");
        return 0;
    }
    return 1;
}
