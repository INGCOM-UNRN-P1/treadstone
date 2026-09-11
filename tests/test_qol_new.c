/**
 * @file test_qol_new.c
 * @brief Pruebas unitarias para validar las 5 mejoras QoL implementadas en lib_test:
 *        1. Aserciones de memoria (ASSERT_ALLOC_COUNT, ASSERT_FREE_COUNT, ASSERT_NO_LEAKS).
 *        2. Mocks de E/S estándar (ASSERT_STDOUT_CONTAINS, ASSERT_STDERR_CONTAINS, ASSERT_STDIO_EQ).
 *        3. Gestor de archivos temporales (p1_temp_file_create, p1_temp_file_create_binary, auto-cleanup).
 *        4. Pistas pedagógicas contextuales (ASSERT_*_HINT).
 *        5. Generador de reportes en Markdown (--md-report).
 */

#include "p1_test.h"
#include "p1_files.h"
#include "p1_stdio.h"

/* --- 1. Aserciones de Gestión de Memoria (QoL 7) --- */

TEST(memoria_conteo_asignacion_y_liberacion) {
    ASSERT_ALLOC_COUNT(0);
    ASSERT_FREE_COUNT(0);
    ASSERT_NO_LEAKS();

    void *p1 = p1_malloc(64);
    ASSERT_PTR_NOT_NULL(p1);
    ASSERT_ALLOC_COUNT(1);
    ASSERT_FREE_COUNT(0);

    void *p2 = p1_calloc(4, sizeof(int));
    ASSERT_PTR_NOT_NULL(p2);
    ASSERT_ALLOC_COUNT(2);

    void *p3 = p1_realloc(p1, 128);
    ASSERT_PTR_NOT_NULL(p3);
    /* realloc cuenta alloc nuevo y free anterior */
    ASSERT_ALLOC_COUNT(3);
    ASSERT_FREE_COUNT(1);

    p1_free(p2);
    p1_free(p3);
    ASSERT_FREE_COUNT(3);
    ASSERT_NO_LEAKS();
}

TEST(memoria_aislamiento_entre_testcases) {
    /* Verifica que RUN_TEST resetea contadores al iniciar */
    ASSERT_ALLOC_COUNT(0);
    ASSERT_FREE_COUNT(0);
    ASSERT_NO_LEAKS();

    char *buf = (char *)p1_malloc(32);
    ASSERT_PTR_NOT_NULL(buf);
    strcpy(buf, "Hola Mundo");
    ASSERT_STR_EQ("Hola Mundo", buf);
    p1_free(buf);
    ASSERT_NO_LEAKS();
}

/* --- 2. Gestor de Archivos Temporales (QoL 9) --- */

static char ruta_archivo_guardada[512] = {0};

TEST(archivos_temporales_creacion_y_lectura) {
    const char *path1 = p1_temp_file_create("Texto de prueba temporal\nSegunda linea\n");
    ASSERT_PTR_NOT_NULL(path1);
    ASSERT_FILE_EXISTS(path1);
    ASSERT_FILE_CONTAINS(path1, "prueba temporal");

    /* Archivo binario temporal */
    unsigned char datos[4] = {0xDE, 0xAD, 0xBE, 0xEF};
    const char *path2 = p1_temp_file_create_binary("binario", datos, sizeof(datos));
    ASSERT_PTR_NOT_NULL(path2);
    ASSERT_FILE_EXISTS(path2);

    /* Guardamos ruta para verificar borrado automático en el próximo test */
    strncpy(ruta_archivo_guardada, path1, sizeof(ruta_archivo_guardada) - 1);
}

TEST(archivos_temporales_limpieza_automatica) {
    /* El archivo creado en el test previo debe haber sido eliminado automáticamente */
    ASSERT_TRUE(strlen(ruta_archivo_guardada) > 0);
    ASSERT_FILE_NOT_EXISTS(ruta_archivo_guardada);
}

/* --- 3. Mocks de E/S Estándar (QoL 8) --- */

static void imprimir_saludos(void) {
    printf("Bienvenido al sistema de Programación 1.\n");
    fprintf(stderr, "ADVERTENCIA: memoria en rango seguro.\n");
}

static void eco_interactivo(void) {
    int valor = 0;
    if (scanf("%d", &valor) == 1) {
        printf("Numero recibido: %d\n", valor * 2);
    }
}

TEST(stdio_captura_contains_y_mock_completo) {
    ASSERT_STDOUT_CONTAINS(imprimir_saludos(), "Programación 1");
    ASSERT_STDERR_CONTAINS(imprimir_saludos(), "ADVERTENCIA");
    ASSERT_STDIO_EQ(eco_interactivo(), "21\n", "Numero recibido: 42\n");
}

/* --- 4. Pistas Pedagógicas Contextuales (QoL 4) --- */

TEST(pistas_pedagogicas_evaluacion_exitosa) {
    /* Verificamos que las macros _HINT evalúan correctamente */
    ASSERT_TRUE_HINT(1 == 1, "La igualdad fundamental debe ser verdadera");
    ASSERT_FALSE_HINT(0 != 0, "Cero nunca es distinto de cero");
    ASSERT_INT_EQ_HINT(42, 40 + 2, "La suma elemental fallo");
    ASSERT_INT_BETWEEN_HINT(15, 10, 20, "El valor debe estar dentro del rango [10, 20]");
    ASSERT_STR_EQ_HINT("aprobado", "aprobado", "Las cadenas deben coincidir");
    ASSERT_STR_CONTAINS_HINT("Estructuras de Datos", "Datos", "Debe contener la palabra clave");

    void *ptr = (void *)0x1234;
    ASSERT_PTR_NOT_NULL_HINT(ptr, "El puntero no debe ser nulo tras inicializar");

    int arr[3] = {1, 2, 3};
    ASSERT_ARRAY_INT_EQ_HINT(arr, arr, 3, "El arreglo debe coincidir consigo mismo");

    const char *tmp = p1_temp_file_create("contenido");
    ASSERT_FILE_EXISTS_HINT(tmp, "El archivo temporal debió crearse en disco");
    ASSERT_FILE_CONTAINS_HINT(tmp, "contenido", "El archivo temporal debe contener el texto inicial");
}

/* --- 5. Reporte Markdown (--md-report) (QoL 15) --- */

TEST(reporte_markdown_verificacion_archivo) {
    /* Generamos manualmente un reporte intermedio para comprobar formato */
    const char *report_file = "build/reporte_test.md";
    p1_write_markdown_report(report_file);
    ASSERT_FILE_EXISTS(report_file);
    ASSERT_FILE_CONTAINS(report_file, "# Reporte de Evaluación de Pruebas:");
    ASSERT_FILE_CONTAINS(report_file, "![Estado]");
    ASSERT_FILE_CONTAINS(report_file, "![Tests]");
    ASSERT_FILE_CONTAINS(report_file, "## Resumen Consolidado");
    ASSERT_FILE_CONTAINS(report_file, "## Desglose por Caso de Prueba");
}

int main(int argc, char **argv) {
    TEST_SUITE_BEGIN_ARGS("Suite de Validación de Nuevas Mejoras QoL (p1_test)", argc, argv);

    RUN_TEST(memoria_conteo_asignacion_y_liberacion);
    RUN_TEST(memoria_aislamiento_entre_testcases);
    RUN_TEST(archivos_temporales_creacion_y_lectura);
    RUN_TEST(archivos_temporales_limpieza_automatica);
    RUN_TEST(stdio_captura_contains_y_mock_completo);
    RUN_TEST(pistas_pedagogicas_evaluacion_exitosa);
    RUN_TEST(reporte_markdown_verificacion_archivo);

    return TEST_REPORT();
}
