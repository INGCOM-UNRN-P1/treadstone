/**
 * @file test_p1_str_diff.c
 * @brief La falla de ASSERT_STR_EQ dice dónde difieren las cadenas (QoL #1240).
 *
 * Los dos tests fallan a propósito con stderr redirigido a un archivo temporal; después se
 * verifica que el informe tenga la posición, la línea y la columna de la primera diferencia y,
 * cuando una cadena es prefijo de la otra, cuántos caracteres faltan.
 */

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "p1_test.h"

TEST(difiere_un_espacio) {
    ASSERT_STR_EQ("Nombre: Ana\nEdad: 20\n", "Nombre: Ana\nEdad:  20\n");
}

TEST(le_faltan_caracteres) {
    ASSERT_STR_EQ("hola mundo", "hola");
}

static int contiene(const char *texto, const char *buscado) {
    return strstr(texto, buscado) != NULL;
}

int main(int argc, char **argv) {
    char ruta[] = "/tmp/p1_str_diff_XXXXXX";
    int fd = mkstemp(ruta);
    if (fd < 0) return 1;
    fflush(stderr);
    int original = dup(STDERR_FILENO);
    dup2(fd, STDERR_FILENO);

    TEST_SUITE_BEGIN_ARGS("Diferencia de cadenas", argc, argv);
    RUN_TEST(difiere_un_espacio);
    RUN_TEST(le_faltan_caracteres);
    int codigo = TEST_REPORT();

    fflush(stderr);
    dup2(original, STDERR_FILENO);
    char salida[8192] = {0};
    FILE *f = fopen(ruta, "r");
    size_t leidos = f ? fread(salida, 1, sizeof(salida) - 1, f) : 0;
    if (f) fclose(f);
    unlink(ruta);
    salida[leidos] = '\0';

    int ok = codigo == 1 && _p1_global_state.tests_failed == 2
             && contiene(salida, "diferencia: posición 18 (línea 2, columna 7)")
             && contiene(salida, "Edad:  20\\n")
             && contiene(salida, "al obtenido le faltan 6 caracteres al final");
    printf(ok ? "Diferencia de cadenas verificada.\n" : "FALLÓ la verificación del diff:\n%s\n", salida);
    return ok ? 0 : 1;
}
