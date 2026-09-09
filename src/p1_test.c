/**
 * @file p1_test.c
 * @brief Implementación de símbolos de enlace y runtime para la librería p1_test.
 *
 * Programación 1 - Ingeniería en Computación - UNRN
 */

#include "p1_test.h"

const char *p1_test_version(void) {
    return "2.0.0";
}

void p1_test_runtime_init(void) {
    (void)_p1_global_state;
}
