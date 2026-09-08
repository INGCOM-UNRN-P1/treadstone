/**
 * @file test_p1_signals.c
 * @brief Verificación de captura de señales fatales (SIGSEGV, SIGFPE) y timeout.
 */

#include "p1_test.h"

/* Test que provoca SIGSEGV (acceso a puntero nulo) */
TEST(test_crash_sigsegv) {
    SUBCASE("Subcaso con puntero inválido");
    volatile int *ptr = NULL;
    *ptr = 42; /* Provoca SIGSEGV intencional */
    ASSERT_FAIL("No debería alcanzarse");
}

/* Test que provoca SIGFPE (división por cero) */
TEST(test_crash_sigfpe) {
    volatile int a = 10;
    volatile int b = 0;
    volatile int c = a / b; /* Provoca SIGFPE intencional */
    (void)c;
    ASSERT_FAIL("No debería alcanzarse");
}

/* Test que corre después de los dos crashes y debe aprobar normalmente */
TEST(test_recuperacion_posterior) {
    ASSERT_INT_EQ(100, 50 + 50);
    ASSERT_TRUE(1 == 1);
}

int main(int argc, char **argv) {
    TEST_SUITE_BEGIN_ARGS("Suite de Verificación de Rescate de Señales", argc, argv);

    RUN_TEST(test_crash_sigsegv);
    RUN_TEST(test_crash_sigfpe);
    RUN_TEST(test_recuperacion_posterior);

    int exit_code = TEST_REPORT();
    /*
     * Esta suite debe detectar exactamente 2 fallos (los 2 crashes).
     * Si exit_code == 1 y tests_failed == 2, el rescate fue exitoso.
     */
    if (exit_code == 1 && _p1_global_state.tests_failed == 2 && _p1_global_state.tests_passed == 1) {
        printf("Rescate verificado: Los crashes (SIGSEGV y SIGFPE) fueron capturados y la suite sobrevivió.\n");
        return 0;
    }
    return 1;
}
