/**
 * @file test_p1_str_diff.c
 * @brief La falla de ASSERT_STR_EQ dice dónde difieren las cadenas (QoL #1240).
 *
 * Los dos tests fallan a propósito con stderr redirigido a un archivo temporal; después se
 * verifica que el informe tenga la posición, la línea y la columna de la primera diferencia y,
 * cuando una cadena es prefijo de la otra, cuántos caracteres faltan.
 */

#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "p1_test.h"

TEST(difiere_un_espacio) {
    fprintf(stdout, "[TRACE: entered test difiere_un_espacio]\n"); fflush(stdout);
    ASSERT_STR_EQ("Nombre: Ana\nEdad: 20\n", "Nombre: Ana\nEdad:  20\n");
    fprintf(stdout, "[TRACE: finished test difiere_un_espacio]\n"); fflush(stdout);
}

TEST(le_faltan_caracteres) {
    fprintf(stdout, "[TRACE: entered test le_faltan_caracteres]\n"); fflush(stdout);
    ASSERT_STR_EQ("hola mundo", "hola");
    fprintf(stdout, "[TRACE: finished test le_faltan_caracteres]\n"); fflush(stdout);
}

static int contiene(const char *texto, const char *buscado) {
    return strstr(texto, buscado) != NULL;
}

int main(int argc, char **argv) {
#ifdef _WIN32
    FILE *f_err = freopen("build/p1_str_diff.tmp", "w", stderr);
    if (!f_err) return 1;
#else
    FILE *tmp = tmpfile();
    if (!tmp) return 1;
    fflush(stderr);
    int original = dup(fileno(stderr));
    dup2(fileno(tmp), fileno(stderr));
#endif

    TEST_SUITE_BEGIN_ARGS("Diferencia de cadenas", argc, argv);
    RUN_TEST(difiere_un_espacio);
    RUN_TEST(le_faltan_caracteres);
    int codigo = TEST_REPORT();

    char salida[8192] = {0};
#ifdef _WIN32
    fflush(stderr);
    FILE *f = fopen("build/p1_str_diff.tmp", "r");
    size_t leidos = f ? fread(salida, 1, sizeof(salida) - 1, f) : 0;
    if (f) fclose(f);
    remove("build/p1_str_diff.tmp");
    salida[leidos] = '\0';
#else
    fflush(stderr);
    dup2(original, fileno(stderr));
    close(original);

    rewind(tmp);
    size_t leidos = fread(salida, 1, sizeof(salida) - 1, tmp);
    fclose(tmp);
    salida[leidos] = '\0';
#endif

    int ok = codigo == 1 && _p1_global_state.tests_failed == 2
             && contiene(salida, "diferencia: posición 18 (línea 2, columna 7)")
             && contiene(salida, "Edad:  20\\n")
             && contiene(salida, "al obtenido le faltan 6 caracteres al final");
    printf(ok ? "Diferencia de cadenas verificada.\n" : "FALLÓ la verificación del diff:\n%s\n", salida);
    return ok ? 0 : 1;
}
