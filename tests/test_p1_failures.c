/**
 * @file test_p1_failures.c
 * @brief Prueba de recuperación ante fallos de aserción.
 * 
 * Verifica que cuando un assert falla:
 * 1. Aborta únicamente el caso de prueba actual y NO mata el proceso.
 * 2. La suite continúa ejecutando los tests subsiguientes.
 * 3. El reporte final contabiliza el fallo y retorna código 1.
 */

#include "p1_test.h"

TEST(test_que_falla_a_proposito) {
    ASSERT_INT_EQ(10, 20);
    /* Esta línea nunca debe alcanzarse debido al longjmp */
    ASSERT_FAIL("No debería ejecutarse después de un fallo");
}

TEST(test_que_pasa_despues_del_fallo) {
    ASSERT_TRUE(1 == 1);
    ASSERT_STR_EQ("ok", "ok");
}

int main(void) {
    TEST_SUITE_BEGIN("Suite de Prueba de Recuperación ante Fallos");

    RUN_TEST(test_que_falla_a_proposito);
    RUN_TEST(test_que_pasa_despues_del_fallo);

    int exit_code = TEST_REPORT();
    /*
     * Esta suite está diseñada para que falle un test (código 1).
     * Si exit_code es 1, el mecanismo de captura funcionó correctamente.
     * Retornamos 0 hacia el SO para indicar que el test del framework fue exitoso.
     */
    if (exit_code == 1) {
        printf("Comportamiento verificado: El fallo fue capturado y la suite continuo normalmente.\n");
        return 0;
    }
    return 1;
}
