/**
 * @file p1_test.h
 * @brief Micro-framework de pruebas unitarias para Programación 1 (UNRN).
 * @version 1.0.0
 * 
 * Librería header-only en C99 puro, sin dependencias externas fuera de la
 * biblioteca estándar de C.
 * 
 * CARACTERÍSTICAS PRINCIPALES:
 * - Aserciones incondicionales: NO desaparecen con -DNDEBUG ni con -DDEBUG.
 * - Control de flujo con setjmp/longjmp: un assert fallido aborta solo el test
 *   actual (incluso dentro de funciones auxiliares) y la suite continúa.
 * - Aserciones tipadas con mensajes didácticos de esperado vs. obtenido:
 *   booleanos, enteros, unsigned, dobles con tolerancia, strings y punteros.
 * - Reporte final con conteo de tests y aserciones, con soporte opcional de colores
 *   ANSI y código de retorno apto para Makefiles y CI/CD (0 = éxito, 1 = fallo).
 */

#ifndef P1_TEST_H
#define P1_TEST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/* --- Configuración de colores de consola ANSI ---------------------------- */
#if !defined(P1_TEST_NO_COLOR)
  #define _P1_CLR_RESET   "\033[0m"
  #define _P1_CLR_RED     "\033[1;31m"
  #define _P1_CLR_GREEN   "\033[1;32m"
  #define _P1_CLR_YELLOW  "\033[1;33m"
  #define _P1_CLR_CYAN    "\033[1;36m"
  #define _P1_CLR_BOLD    "\033[1m"
#else
  #define _P1_CLR_RESET   ""
  #define _P1_CLR_RED     ""
  #define _P1_CLR_GREEN   ""
  #define _P1_CLR_YELLOW  ""
  #define _P1_CLR_CYAN    ""
  #define _P1_CLR_BOLD    ""
#endif

/* --- Estructura de estado del ejecutor de pruebas ------------------------ */
typedef struct {
    const char *suite_name;         /**< Nombre descriptivo de la suite */
    const char *current_test_name;  /**< Nombre del test en ejecución */
    int tests_run;                  /**< Cantidad total de tests ejecutados */
    int tests_passed;               /**< Cantidad de tests aprobados */
    int tests_failed;               /**< Cantidad de tests desaprobados */
    int asserts_total;              /**< Total de aserciones evaluadas */
    int asserts_failed;             /**< Total de aserciones que fallaron */
    int current_test_failed;        /**< Bandera: 1 si el test actual falló */
    jmp_buf jump_env;               /**< Punto de rescate para abortar el test */
    int in_test_scope;              /**< 1 si se está dentro de un RUN_TEST */
} p1_test_state_t;

/* Instancia estática única por unidad de compilación (inicializada a 0 por defecto) */
static p1_test_state_t _p1_global_state;

/* --- Funciones auxiliares internas --------------------------------------- */

/**
 * @brief Valor absoluto para dobles sin requerir vincular -lm.
 */
static inline double _p1_abs_double(double x) {
    return (x < 0.0) ? -x : x;
}

/**
 * @brief Imprime encabezado de fallo con archivo, línea y nombre del test.
 */
static inline void _p1_print_fail_header(const char *file, int line, const char *assert_expr) {
    fprintf(stderr, "\n  %s[FALLO]%s %s:%d: %s%s%s\n",
            _P1_CLR_RED, _P1_CLR_RESET, file, line,
            _P1_CLR_BOLD, assert_expr, _P1_CLR_RESET);
    if (_p1_global_state.current_test_name != NULL) {
        fprintf(stderr, "    %sen test:%s %s\n",
                _P1_CLR_CYAN, _P1_CLR_RESET, _p1_global_state.current_test_name);
    }
}

/**
 * @brief Imprime mensaje opcional formateado por el usuario.
 */
static inline void _p1_print_user_msg(const char *fmt, va_list args) {
    if (fmt != NULL && fmt[0] != '\0') {
        fprintf(stderr, "    %smensaje:%s ", _P1_CLR_YELLOW, _P1_CLR_RESET);
        vfprintf(stderr, fmt, args);
        fprintf(stderr, "\n");
    }
}

/**
 * @brief Registra el fallo de una aserción y aborta el test en curso.
 */
static inline void _p1_trigger_failure(void) {
    _p1_global_state.asserts_failed++;
    _p1_global_state.current_test_failed = 1;
    if (_p1_global_state.in_test_scope) {
        longjmp(_p1_global_state.jump_env, 1);
    }
}

/**
 * @brief Reporta fallo de aserción booleana.
 */
static inline void _p1_fail_bool(const char *file, int line, const char *expr,
                                 int expected_bool, const char *fmt, ...) {
    _p1_print_fail_header(file, line, expr);
    fprintf(stderr, "    esperado: %s\n", expected_bool ? "verdadero (distinto de 0)" : "falso (0)");
    fprintf(stderr, "    obtenido: %s\n", expected_bool ? "falso (0)" : "verdadero");

    if (fmt != NULL) {
        va_list args;
        va_start(args, fmt);
        _p1_print_user_msg(fmt, args);
        va_end(args);
    }
    _p1_trigger_failure();
}

/**
 * @brief Reporta fallo de comparación de enteros con signo.
 */
static inline void _p1_fail_int(const char *file, int line, const char *expr,
                                long long expected, long long actual,
                                const char *op_name, const char *fmt, ...) {
    _p1_print_fail_header(file, line, expr);
    fprintf(stderr, "    esperado: %s %lld\n", op_name, expected);
    fprintf(stderr, "    obtenido: %lld\n", actual);

    if (fmt != NULL) {
        va_list args;
        va_start(args, fmt);
        _p1_print_user_msg(fmt, args);
        va_end(args);
    }
    _p1_trigger_failure();
}

/**
 * @brief Reporta fallo de comparación de enteros sin signo.
 */
static inline void _p1_fail_uint(const char *file, int line, const char *expr,
                                 unsigned long long expected, unsigned long long actual,
                                 const char *op_name, const char *fmt, ...) {
    _p1_print_fail_header(file, line, expr);
    fprintf(stderr, "    esperado: %s %llu\n", op_name, expected);
    fprintf(stderr, "    obtenido: %llu\n", actual);

    if (fmt != NULL) {
        va_list args;
        va_start(args, fmt);
        _p1_print_user_msg(fmt, args);
        va_end(args);
    }
    _p1_trigger_failure();
}

/**
 * @brief Reporta fallo de comparación de números de coma flotante (double).
 */
static inline void _p1_fail_double(const char *file, int line, const char *expr,
                                   double expected, double actual, double epsilon,
                                   const char *fmt, ...) {
    _p1_print_fail_header(file, line, expr);
    fprintf(stderr, "    esperado: %.7g (tolerancia +/- %.7g)\n", expected, epsilon);
    fprintf(stderr, "    obtenido: %.7g (diferencia: %.7g)\n", actual, _p1_abs_double(expected - actual));

    if (fmt != NULL) {
        va_list args;
        va_start(args, fmt);
        _p1_print_user_msg(fmt, args);
        va_end(args);
    }
    _p1_trigger_failure();
}

/**
 * @brief Reporta fallo de comparación de cadenas de texto (strings).
 */
static inline void _p1_fail_str(const char *file, int line, const char *expr,
                                const char *expected, const char *actual,
                                const char *desc, const char *fmt, ...) {
    _p1_print_fail_header(file, line, expr);
    fprintf(stderr, "    esperado: %s %s%s%s\n", desc,
            expected ? "\"" : "", expected ? expected : "NULL", expected ? "\"" : "");
    fprintf(stderr, "    obtenido: %s%s%s\n",
            actual ? "\"" : "", actual ? actual : "NULL", actual ? "\"" : "");

    if (fmt != NULL) {
        va_list args;
        va_start(args, fmt);
        _p1_print_user_msg(fmt, args);
        va_end(args);
    }
    _p1_trigger_failure();
}

/**
 * @brief Reporta fallo de comparación de punteros.
 */
static inline void _p1_fail_ptr(const char *file, int line, const char *expr,
                                const void *expected, const void *actual,
                                const char *desc, const char *fmt, ...) {
    _p1_print_fail_header(file, line, expr);
    if (expected == NULL && strcmp(desc, "distinto de") != 0) {
        fprintf(stderr, "    esperado: NULL\n");
    } else {
        fprintf(stderr, "    esperado: %s %p\n", desc, expected);
    }
    fprintf(stderr, "    obtenido: %p\n", actual);

    if (fmt != NULL) {
        va_list args;
        va_start(args, fmt);
        _p1_print_user_msg(fmt, args);
        va_end(args);
    }
    _p1_trigger_failure();
}

/**
 * @brief Reporta un fallo explícito indicado por el usuario.
 */
static inline void _p1_fail_explicit(const char *file, int line, const char *fmt, ...) {
    _p1_print_fail_header(file, line, "ASSERT_FAIL()");
    if (fmt != NULL) {
        va_list args;
        va_start(args, fmt);
        _p1_print_user_msg(fmt, args);
        va_end(args);
    }
    _p1_trigger_failure();
}

/* ========================================================================= */
/* --- MACROS DE DEFINICIÓN Y EJECUCIÓN DE PRUEBAS ------------------------- */
/* ========================================================================= */

/**
 * @def TEST(name)
 * @brief Declara una función de prueba unitaria.
 * @param name Identificador del test en C.
 */
#define TEST(name) static void _p1_test_func_##name(void)

/**
 * @def TEST_SUITE_BEGIN(suite_title)
 * @brief Inicializa la suite de pruebas con un nombre descriptivo.
 */
#define TEST_SUITE_BEGIN(suite_title) do { \
    _p1_global_state.suite_name = (suite_title); \
    _p1_global_state.tests_run = 0; \
    _p1_global_state.tests_passed = 0; \
    _p1_global_state.tests_failed = 0; \
    _p1_global_state.asserts_total = 0; \
    _p1_global_state.asserts_failed = 0; \
    _p1_global_state.current_test_failed = 0; \
    _p1_global_state.in_test_scope = 0; \
    printf("\n%s=== INICIANDO SUITE: %s ===%s\n\n", \
           _P1_CLR_BOLD, _p1_global_state.suite_name, _P1_CLR_RESET); \
} while (0)

/**
 * @def RUN_TEST(name)
 * @brief Ejecuta el caso de prueba especificado.
 *
 * Captura fallos de aserción con setjmp/longjmp, reporta el resultado
 * individual y continúa ejecutando la suite sin detener el proceso.
 */
#define RUN_TEST(name) do { \
    _p1_global_state.tests_run++; \
    _p1_global_state.current_test_name = #name; \
    _p1_global_state.current_test_failed = 0; \
    _p1_global_state.in_test_scope = 1; \
    printf("  [CORRIENDO] %s ... ", #name); \
    fflush(stdout); \
    if (setjmp(_p1_global_state.jump_env) == 0) { \
        _p1_test_func_##name(); \
    } \
    _p1_global_state.in_test_scope = 0; \
    if (_p1_global_state.current_test_failed) { \
        _p1_global_state.tests_failed++; \
        printf("  [RESULTADO] %sFALLÓ%s\n", _P1_CLR_RED, _P1_CLR_RESET); \
    } else { \
        _p1_global_state.tests_passed++; \
        printf("%sPASÓ%s\n", _P1_CLR_GREEN, _P1_CLR_RESET); \
    } \
    _p1_global_state.current_test_name = NULL; \
} while (0)

/**
 * @def TEST_REPORT()
 * @brief Imprime el resumen consolidado de la suite y retorna el código de salida.
 * @return 0 si todos los tests pasaron, 1 si al menos un test falló.
 */
static inline int TEST_REPORT(void) {
    printf("\n%s============================================================%s\n",
           _P1_CLR_BOLD, _P1_CLR_RESET);
    printf("%sResultados de la Suite: %s%s\n",
           _P1_CLR_BOLD, _p1_global_state.suite_name, _P1_CLR_RESET);
    printf("============================================================\n");
    printf("  Total de tests ejecutados : %d\n", _p1_global_state.tests_run);
    printf("  Tests aprobados           : %s%d%s\n",
           _p1_global_state.tests_passed > 0 ? _P1_CLR_GREEN : "",
           _p1_global_state.tests_passed, _P1_CLR_RESET);
    printf("  Tests desaprobados        : %s%d%s\n",
           _p1_global_state.tests_failed > 0 ? _P1_CLR_RED : "",
           _p1_global_state.tests_failed, _P1_CLR_RESET);
    printf("  Aserciones evaluadas      : %d (falladas: %d)\n",
           _p1_global_state.asserts_total, _p1_global_state.asserts_failed);
    printf("============================================================\n");

    if (_p1_global_state.tests_failed == 0) {
        printf("%s>>> ESTADO: TODOS LOS TESTS PASARON EXITOSAMENTE <<<%s\n\n",
               _P1_CLR_GREEN, _P1_CLR_RESET);
        return 0;
    } else {
        printf("%s>>> ESTADO: SE DETECTARON FALLOS EN LAS PRUEBAS <<<%s\n\n",
               _P1_CLR_RED, _P1_CLR_RESET);
        return 1;
    }
}

/* ========================================================================= */
/* --- MACROS DE ASERCIÓN DIDÁCTICAS (SIEMPRE ACTIVAS) --------------------- */
/* ========================================================================= */

/* --- Aserciones Booleanas --- */

#define ASSERT_TRUE_MSG(cond, ...) do { \
    _p1_global_state.asserts_total++; \
    if (!(cond)) { \
        _p1_fail_bool(__FILE__, __LINE__, "ASSERT_TRUE(" #cond ")", 1, __VA_ARGS__); \
    } \
} while (0)

#define ASSERT_TRUE(cond) ASSERT_TRUE_MSG(cond, NULL)

#define ASSERT_FALSE_MSG(cond, ...) do { \
    _p1_global_state.asserts_total++; \
    if (cond) { \
        _p1_fail_bool(__FILE__, __LINE__, "ASSERT_FALSE(" #cond ")", 0, __VA_ARGS__); \
    } \
} while (0)

#define ASSERT_FALSE(cond) ASSERT_FALSE_MSG(cond, NULL)

/* --- Aserciones sobre Enteros con Signo --- */

#define ASSERT_INT_EQ_MSG(expected, actual, ...) do { \
    _p1_global_state.asserts_total++; \
    long long _exp = (long long)(expected); \
    long long _act = (long long)(actual); \
    if (_exp != _act) { \
        _p1_fail_int(__FILE__, __LINE__, "ASSERT_INT_EQ(" #expected ", " #actual ")", \
                     _exp, _act, "igual a", __VA_ARGS__); \
    } \
} while (0)

#define ASSERT_INT_EQ(expected, actual) ASSERT_INT_EQ_MSG(expected, actual, NULL)

#define ASSERT_INT_NE_MSG(expected, actual, ...) do { \
    _p1_global_state.asserts_total++; \
    long long _exp = (long long)(expected); \
    long long _act = (long long)(actual); \
    if (_exp == _act) { \
        _p1_fail_int(__FILE__, __LINE__, "ASSERT_INT_NE(" #expected ", " #actual ")", \
                     _exp, _act, "distinto de", __VA_ARGS__); \
    } \
} while (0)

#define ASSERT_INT_NE(expected, actual) ASSERT_INT_NE_MSG(expected, actual, NULL)

#define ASSERT_INT_LT_MSG(val, limit, ...) do { \
    _p1_global_state.asserts_total++; \
    long long _v = (long long)(val); \
    long long _lim = (long long)(limit); \
    if (!(_v < _lim)) { \
        _p1_fail_int(__FILE__, __LINE__, "ASSERT_INT_LT(" #val ", " #limit ")", \
                     _lim, _v, "menor que", __VA_ARGS__); \
    } \
} while (0)

#define ASSERT_INT_LT(val, limit) ASSERT_INT_LT_MSG(val, limit, NULL)

#define ASSERT_INT_LE_MSG(val, limit, ...) do { \
    _p1_global_state.asserts_total++; \
    long long _v = (long long)(val); \
    long long _lim = (long long)(limit); \
    if (!(_v <= _lim)) { \
        _p1_fail_int(__FILE__, __LINE__, "ASSERT_INT_LE(" #val ", " #limit ")", \
                     _lim, _v, "menor o igual que", __VA_ARGS__); \
    } \
} while (0)

#define ASSERT_INT_LE(val, limit) ASSERT_INT_LE_MSG(val, limit, NULL)

#define ASSERT_INT_GT_MSG(val, limit, ...) do { \
    _p1_global_state.asserts_total++; \
    long long _v = (long long)(val); \
    long long _lim = (long long)(limit); \
    if (!(_v > _lim)) { \
        _p1_fail_int(__FILE__, __LINE__, "ASSERT_INT_GT(" #val ", " #limit ")", \
                     _lim, _v, "mayor que", __VA_ARGS__); \
    } \
} while (0)

#define ASSERT_INT_GT(val, limit) ASSERT_INT_GT_MSG(val, limit, NULL)

#define ASSERT_INT_GE_MSG(val, limit, ...) do { \
    _p1_global_state.asserts_total++; \
    long long _v = (long long)(val); \
    long long _lim = (long long)(limit); \
    if (!(_v >= _lim)) { \
        _p1_fail_int(__FILE__, __LINE__, "ASSERT_INT_GE(" #val ", " #limit ")", \
                     _lim, _v, "mayor o igual que", __VA_ARGS__); \
    } \
} while (0)

#define ASSERT_INT_GE(val, limit) ASSERT_INT_GE_MSG(val, limit, NULL)

/* --- Aserciones sobre Enteros sin Signo (unsigned, size_t) --- */

#define ASSERT_UINT_EQ_MSG(expected, actual, ...) do { \
    _p1_global_state.asserts_total++; \
    unsigned long long _uexp = (unsigned long long)(expected); \
    unsigned long long _uact = (unsigned long long)(actual); \
    if (_uexp != _uact) { \
        _p1_fail_uint(__FILE__, __LINE__, "ASSERT_UINT_EQ(" #expected ", " #actual ")", \
                      _uexp, _uact, "igual a", __VA_ARGS__); \
    } \
} while (0)

#define ASSERT_UINT_EQ(expected, actual) ASSERT_UINT_EQ_MSG(expected, actual, NULL)

#define ASSERT_UINT_NE_MSG(expected, actual, ...) do { \
    _p1_global_state.asserts_total++; \
    unsigned long long _uexp = (unsigned long long)(expected); \
    unsigned long long _uact = (unsigned long long)(actual); \
    if (_uexp == _uact) { \
        _p1_fail_uint(__FILE__, __LINE__, "ASSERT_UINT_NE(" #expected ", " #actual ")", \
                      _uexp, _uact, "distinto de", __VA_ARGS__); \
    } \
} while (0)

#define ASSERT_UINT_NE(expected, actual) ASSERT_UINT_NE_MSG(expected, actual, NULL)

/* --- Aserciones sobre Coma Flotante (double, float) --- */

#define ASSERT_DOUBLE_EQ_MSG(expected, actual, epsilon, ...) do { \
    _p1_global_state.asserts_total++; \
    double _dexp = (double)(expected); \
    double _dact = (double)(actual); \
    double _deps = (double)(epsilon); \
    if (_p1_abs_double(_dexp - _dact) > _deps) { \
        _p1_fail_double(__FILE__, __LINE__, \
                        "ASSERT_DOUBLE_EQ(" #expected ", " #actual ", " #epsilon ")", \
                        _dexp, _dact, _deps, __VA_ARGS__); \
    } \
} while (0)

#define ASSERT_DOUBLE_EQ(expected, actual, epsilon) \
    ASSERT_DOUBLE_EQ_MSG(expected, actual, epsilon, NULL)

#define ASSERT_DOUBLE_NE_MSG(expected, actual, epsilon, ...) do { \
    _p1_global_state.asserts_total++; \
    double _dexp = (double)(expected); \
    double _dact = (double)(actual); \
    double _deps = (double)(epsilon); \
    if (_p1_abs_double(_dexp - _dact) <= _deps) { \
        _p1_fail_double(__FILE__, __LINE__, \
                        "ASSERT_DOUBLE_NE(" #expected ", " #actual ", " #epsilon ")", \
                        _dexp, _dact, _deps, __VA_ARGS__); \
    } \
} while (0)

#define ASSERT_DOUBLE_NE(expected, actual, epsilon) \
    ASSERT_DOUBLE_NE_MSG(expected, actual, epsilon, NULL)

/* --- Aserciones sobre Cadenas de Caracteres (strings) --- */

#define ASSERT_STR_EQ_MSG(expected, actual, ...) do { \
    _p1_global_state.asserts_total++; \
    const char *_sexp = (const char *)(expected); \
    const char *_sact = (const char *)(actual); \
    int _equal = 0; \
    if (_sexp == NULL && _sact == NULL) { \
        _equal = 1; \
    } else if (_sexp != NULL && _sact != NULL) { \
        _equal = (strcmp(_sexp, _sact) == 0); \
    } \
    if (!_equal) { \
        _p1_fail_str(__FILE__, __LINE__, "ASSERT_STR_EQ(" #expected ", " #actual ")", \
                     _sexp, _sact, "igual a", __VA_ARGS__); \
    } \
} while (0)

#define ASSERT_STR_EQ(expected, actual) ASSERT_STR_EQ_MSG(expected, actual, NULL)

#define ASSERT_STR_NE_MSG(expected, actual, ...) do { \
    _p1_global_state.asserts_total++; \
    const char *_sexp = (const char *)(expected); \
    const char *_sact = (const char *)(actual); \
    int _equal = 0; \
    if (_sexp == NULL && _sact == NULL) { \
        _equal = 1; \
    } else if (_sexp != NULL && _sact != NULL) { \
        _equal = (strcmp(_sexp, _sact) == 0); \
    } \
    if (_equal) { \
        _p1_fail_str(__FILE__, __LINE__, "ASSERT_STR_NE(" #expected ", " #actual ")", \
                     _sexp, _sact, "distinto de", __VA_ARGS__); \
    } \
} while (0)

#define ASSERT_STR_NE(expected, actual) ASSERT_STR_NE_MSG(expected, actual, NULL)

#define ASSERT_STR_CONTAINS_MSG(haystack, needle, ...) do { \
    _p1_global_state.asserts_total++; \
    const char *_hay = (const char *)(haystack); \
    const char *_nee = (const char *)(needle); \
    int _found = 0; \
    if (_hay != NULL && _nee != NULL) { \
        _found = (strstr(_hay, _nee) != NULL); \
    } \
    if (!_found) { \
        _p1_fail_str(__FILE__, __LINE__, "ASSERT_STR_CONTAINS(" #haystack ", " #needle ")", \
                     _nee, _hay, "que contenga a", __VA_ARGS__); \
    } \
} while (0)

#define ASSERT_STR_CONTAINS(haystack, needle) ASSERT_STR_CONTAINS_MSG(haystack, needle, NULL)

/* --- Aserciones sobre Punteros --- */

#define ASSERT_PTR_NULL_MSG(ptr, ...) do { \
    _p1_global_state.asserts_total++; \
    const void *_p = (const void *)(ptr); \
    if (_p != NULL) { \
        _p1_fail_ptr(__FILE__, __LINE__, "ASSERT_PTR_NULL(" #ptr ")", \
                     NULL, _p, "igual a", __VA_ARGS__); \
    } \
} while (0)

#define ASSERT_PTR_NULL(ptr) ASSERT_PTR_NULL_MSG(ptr, NULL)

#define ASSERT_PTR_NOT_NULL_MSG(ptr, ...) do { \
    _p1_global_state.asserts_total++; \
    const void *_p = (const void *)(ptr); \
    if (_p == NULL) { \
        _p1_fail_ptr(__FILE__, __LINE__, "ASSERT_PTR_NOT_NULL(" #ptr ")", \
                     NULL, _p, "distinto de", __VA_ARGS__); \
    } \
} while (0)

#define ASSERT_PTR_NOT_NULL(ptr) ASSERT_PTR_NOT_NULL_MSG(ptr, NULL)

#define ASSERT_PTR_EQ_MSG(expected, actual, ...) do { \
    _p1_global_state.asserts_total++; \
    const void *_pexp = (const void *)(expected); \
    const void *_pact = (const void *)(actual); \
    if (_pexp != _pact) { \
        _p1_fail_ptr(__FILE__, __LINE__, "ASSERT_PTR_EQ(" #expected ", " #actual ")", \
                     _pexp, _pact, "igual a", __VA_ARGS__); \
    } \
} while (0)

#define ASSERT_PTR_EQ(expected, actual) ASSERT_PTR_EQ_MSG(expected, actual, NULL)

#define ASSERT_PTR_NE_MSG(expected, actual, ...) do { \
    _p1_global_state.asserts_total++; \
    const void *_pexp = (const void *)(expected); \
    const void *_pact = (const void *)(actual); \
    if (_pexp == _pact) { \
        _p1_fail_ptr(__FILE__, __LINE__, "ASSERT_PTR_NE(" #expected ", " #actual ")", \
                     _pexp, _pact, "distinto de", __VA_ARGS__); \
    } \
} while (0)

#define ASSERT_PTR_NE(expected, actual) ASSERT_PTR_NE_MSG(expected, actual, NULL)

/* --- Aserción de Fallo Explícito --- */

#define ASSERT_FAIL(...) do { \
    _p1_global_state.asserts_total++; \
    _p1_fail_explicit(__FILE__, __LINE__, __VA_ARGS__); \
} while (0)

#ifdef __cplusplus
}
#endif

#endif /* P1_TEST_H */
