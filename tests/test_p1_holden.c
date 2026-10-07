/**
 * @file test_p1_holden.c
 * @brief Prueba de P1_FALLAR_EN con y sin holden.
 *
 * Sin -DP1_HOLDEN (holden no instalado), los tests que piden un fallo se
 * saltean. Con -DP1_HOLDEN y los mocks enlazados (`make test-holden`), el
 * fallo ocurre en la llamada pedida y se desarma al terminar el test.
 */

#include "p1_test.h"

TEST(test_malloc_falla_en_la_segunda)
{
    P1_FALLAR_EN(malloc, 2);
    void *primero = malloc(8);
    void *segundo = malloc(8);
    ASSERT_PTR_NOT_NULL(primero);
    ASSERT_PTR_NULL(segundo);
    free(primero);
}

TEST(test_fopen_falla)
{
    P1_FALLAR_EN(fopen, 1);
    FILE *archivo = fopen("/dev/null", "r");
    ASSERT_PTR_NULL(archivo);
}

TEST(test_desarmado_entre_tests)
{
    void *bloque = malloc(8);
    ASSERT_PTR_NOT_NULL(bloque);
    free(bloque);
    FILE *archivo = fopen("/dev/null", "r");
    ASSERT_PTR_NOT_NULL(archivo);
    fclose(archivo);
}

int main(int argc, char **argv)
{
    TEST_SUITE_BEGIN_ARGS("Suite de P1_FALLAR_EN (holden)", argc, argv);

    RUN_TEST(test_malloc_falla_en_la_segunda);
    RUN_TEST(test_fopen_falla);
    RUN_TEST(test_desarmado_entre_tests);

    int exit_code = TEST_REPORT();
#ifdef P1_HOLDEN
    if (exit_code == 0 && _p1_global_state.tests_passed == 3)
    {
        printf("Comportamiento verificado: los mocks de holden fallan y se desarman.\n");
        return 0;
    }
#else
    if (exit_code == 0 && _p1_global_state.tests_skipped == 2 && _p1_global_state.tests_passed == 1)
    {
        printf("Comportamiento verificado: sin holden, los tests con P1_FALLAR_EN se saltean.\n");
        return 0;
    }
#endif
    return 1;
}
