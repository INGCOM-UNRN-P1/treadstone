/**
 * @file test_p1_timeout.c
 * @brief Verificación del watchdog de timeout con SIGALRM.
 */

#include "p1_test.h"

/* Test que entra en un bucle infinito intencional */
TEST(test_bucle_infinito) {
    volatile int x = 0;
    while (1) {
        x = x + 1;
        if (x < 0) {
            break;
        }
    }
    ASSERT_FAIL("No debería alcanzarse");
}

/* Test que corre después del timeout y debe aprobar normalmente */
TEST(test_recuperacion_post_timeout) {
    ASSERT_INT_EQ(42, 40 + 2);
    ASSERT_TRUE(1 == 1);
}

int main(int argc, char **argv) {
    TEST_SUITE_BEGIN_ARGS("Suite de Verificación de Timeout (SIGALRM)", argc, argv);

    /* Forzamos timeout de 1 segundo para verificar interrupción rápida */
    _p1_global_state.timeout_seconds = 1;

    RUN_TEST(test_bucle_infinito);
    RUN_TEST(test_recuperacion_post_timeout);

    int exit_code = TEST_REPORT();
    /*
     * Esta suite está diseñada para que el bucle infinito sea interrumpido por SIGALRM
     * registrando exactamente 1 fallo, y el segundo test apruebe normalmente.
     */
    if (exit_code == 1 && _p1_global_state.tests_failed == 1 && _p1_global_state.tests_passed == 1) {
        printf("Watchdog verificado: El bucle infinito fue interrumpido tras 1 segundo y la suite sobrevivió.\n");
        return 0;
    }
    return 1;
}
