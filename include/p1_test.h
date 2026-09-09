/**
 * @file p1_test.h
 * @brief Micro-framework avanzado de pruebas unitarias para Programación 1 (UNRN).
 * @version 2.0.0
 * 
 * Librería header-only en C11 puro con extensiones POSIX condicionales.
 * 
 * CARACTERÍSTICAS:
 * 1. Aserciones incondicionales: Inmunes a -DNDEBUG y -DDEBUG.
 * 2. Control de flujo no local: setjmp/longjmp (sigsetjmp/siglongjmp en POSIX).
 * 3. Filtrado por CLI (-k, --filter) y flags (--fail-fast, -q/--quiet, --tap, -t/--timeout).
 * 4. Watchdog de timeout por test con alarm/SIGALRM.
 * 5. Rescate de señales fatales (SIGSEGV, SIGFPE, SIGBUS, SIGILL) sin abortar la suite.
 * 6. Medición de tiempo de ejecución en milisegundos por test.
 * 7. Autodetección de terminal TTY para colores ANSI (compatible con NO_COLOR y TERM=dumb).
 * 8. Formato estándar TAP v13 (--tap) para integración con autograders.
 * 9. Subcasos descriptivos (SUBCASE).
 * 10. Tests salteados/pendientes (SKIP_TEST, TEST_SKIP).
 * 11. Comparador de arreglos enteros (ASSERT_ARRAY_INT_EQ).
 * 12. Tolerancia relativa para dobles (ASSERT_DOUBLE_NEAR_REL).
 * 13. Aserción de memoria binaria con volcado hexadecimal (ASSERT_MEM_EQ).
 * 14. Aserción de rango acotado (ASSERT_INT_BETWEEN).
 * 15. Aserción de cadenas insensible a mayúsculas (ASSERT_STR_CASE_EQ).
 * 16. Hooks de ciclo de vida (BEFORE_EACH, AFTER_EACH).
 * 17. Captura y aserción de salida estándar (ASSERT_STDOUT_EQ).
 * 18. Tests con semilla pseudoaleatoria fija (TEST_SEEDED, RUN_TEST_SEEDED).
 */

#ifndef P1_TEST_H
#define P1_TEST_H

#if !defined(_POSIX_C_SOURCE) && !defined(_XOPEN_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdarg.h>
#include <time.h>
#include <stdint.h>
#include <ctype.h>

#if defined(__unix__) || defined(__APPLE__)
#include <unistd.h>
#include <signal.h>
#define _P1_HAS_POSIX 1
extern int isatty(int);
extern unsigned int alarm(unsigned int);
extern int dup(int);
extern int dup2(int, int);
extern int close(int);
extern int fileno(FILE *);
#else
#define _P1_HAS_POSIX 0
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* --- Colores de consola ANSI --------------------------------------------- */
#define _P1_CLR_RESET   "\033[0m"
#define _P1_CLR_RED     "\033[1;31m"
#define _P1_CLR_GREEN   "\033[1;32m"
#define _P1_CLR_YELLOW  "\033[1;33m"
#define _P1_CLR_CYAN    "\033[1;36m"
#define _P1_CLR_MAGENTA "\033[1;35m"
#define _P1_CLR_BOLD    "\033[1m"

/* --- Tipos de Hook ------------------------------------------------------- */
typedef void (*p1_hook_fn_t)(void);

/* --- Estructura de estado global del ejecutor ----------------------------- */
typedef struct {
    const char *suite_name;         /**< Nombre descriptivo de la suite */
    const char *current_test_name;  /**< Nombre del test en ejecución */
    const char *current_subcase;    /**< Subcaso o sección descriptiva actual */
    int tests_run;                  /**< Cantidad total de tests ejecutados */
    int tests_passed;               /**< Cantidad de tests aprobados */
    int tests_failed;               /**< Cantidad de tests desaprobados */
    int tests_skipped;              /**< Cantidad de tests salteados */
    int asserts_total;              /**< Total de aserciones evaluadas */
    int asserts_failed;             /**< Total de aserciones que fallaron */
    int current_test_failed;        /**< 1 si el test actual falló */
    int current_test_skipped;       /**< 1 si el test actual fue salteado */
    int in_test_scope;              /**< 1 si se está dentro de un RUN_TEST */
    double current_elapsed_ms;      /**< Tiempo de ejecución del test actual */

    /* Configuración por CLI */
    const char *filter;             /**< Filtro de nombre de test */
    int fail_fast;                  /**< Detener suite en primer fallo */
    int quiet_mode;                 /**< Salida compacta (. y F) */
    int tap_mode;                   /**< Salida en formato TAP v13 */
    int no_color;                   /**< Desactivar colores ANSI */
    int timeout_seconds;            /**< Timeout en segundos por test */
    int tap_test_index;             /**< Contador de pruebas para TAP */

    /* Hooks */
    p1_hook_fn_t before_each;       /**< Setup antes de cada test */
    p1_hook_fn_t after_each;        /**< Teardown después de cada test */
    p1_hook_fn_t cleanup_hook;      /**< Hook de limpieza automática (descriptores, mocks) */

    /* Control de flujo no local estándar C11 */
    jmp_buf jump_env;
    int has_jump_env;
} p1_test_state_t;

/* Instancia estática única por unidad de compilación */
static p1_test_state_t _p1_global_state;

/* --- Gestión de colores y autodetección de terminal ----------------------- */

static inline int _p1_should_use_color(void) {
    if (_p1_global_state.no_color || _p1_global_state.tap_mode) return 0;
    if (getenv("NO_COLOR") != NULL) return 0;
    const char *term = getenv("TERM");
    if (term != NULL && strcmp(term, "dumb") == 0) return 0;
#if _P1_HAS_POSIX
    if (!isatty(fileno(stdout))) return 0;
#endif
    return 1;
}

static inline const char *_p1_clr(const char *ansi_code) {
    return _p1_should_use_color() ? ansi_code : "";
}

/* --- Funciones matemáticas y de comparación auxiliares ------------------- */

static inline double _p1_abs_double(double x) {
    return (x < 0.0) ? -x : x;
}

static inline int _p1_strcasecmp(const char *s1, const char *s2) {
    if (s1 == s2) return 0;
    if (s1 == NULL) return -1;
    if (s2 == NULL) return 1;
    while (*s1 && *s2) {
        int c1 = tolower((unsigned char)*s1);
        int c2 = tolower((unsigned char)*s2);
        if (c1 != c2) return c1 - c2;
        s1++;
        s2++;
    }
    return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
}

/* --- Formateo de diagnósticos de fallo ----------------------------------- */

static inline void _p1_print_fail_header(const char *file, int line, const char *assert_expr) {
    if (_p1_global_state.quiet_mode) return;
    fprintf(stderr, "\n  %s[FALLO]%s %s:%d: %s%s%s\n",
            _p1_clr(_P1_CLR_RED), _p1_clr(_P1_CLR_RESET), file, line,
            _p1_clr(_P1_CLR_BOLD), assert_expr, _p1_clr(_P1_CLR_RESET));
    if (_p1_global_state.current_test_name != NULL) {
        fprintf(stderr, "    %sen test:%s %s\n",
                _p1_clr(_P1_CLR_CYAN), _p1_clr(_P1_CLR_RESET), _p1_global_state.current_test_name);
    }
    if (_p1_global_state.current_subcase != NULL) {
        fprintf(stderr, "    %sen subcaso:%s %s\n",
                _p1_clr(_P1_CLR_YELLOW), _p1_clr(_P1_CLR_RESET), _p1_global_state.current_subcase);
    }
}

static inline void _p1_print_user_msg(const char *fmt, va_list args) {
    if (_p1_global_state.quiet_mode) return;
    if (fmt != NULL && fmt[0] != '\0') {
        fprintf(stderr, "    %smensaje:%s ", _p1_clr(_P1_CLR_YELLOW), _p1_clr(_P1_CLR_RESET));
        vfprintf(stderr, fmt, args);
        fprintf(stderr, "\n");
    }
}

static inline void _p1_trigger_failure(void) {
    _p1_global_state.asserts_failed++;
    _p1_global_state.current_test_failed = 1;
    if (_p1_global_state.in_test_scope) {
        longjmp(_p1_global_state.jump_env, 1);
    }
}

/* --- Manejadores de fallos tipados --------------------------------------- */

static inline void _p1_fail_bool(const char *file, int line, const char *expr,
                                 int expected_bool, const char *fmt, ...) {
    _p1_print_fail_header(file, line, expr);
    if (!_p1_global_state.quiet_mode) {
        fprintf(stderr, "    esperado: %s\n", expected_bool ? "verdadero (distinto de 0)" : "falso (0)");
        fprintf(stderr, "    obtenido: %s\n", expected_bool ? "falso (0)" : "verdadero");
    }
    if (fmt != NULL) {
        va_list args;
        va_start(args, fmt);
        _p1_print_user_msg(fmt, args);
        va_end(args);
    }
    _p1_trigger_failure();
}

static inline void _p1_fail_int(const char *file, int line, const char *expr,
                                long long expected, long long actual,
                                const char *op_name, const char *fmt, ...) {
    _p1_print_fail_header(file, line, expr);
    if (!_p1_global_state.quiet_mode) {
        fprintf(stderr, "    esperado: %s %lld\n", op_name, expected);
        fprintf(stderr, "    obtenido: %lld\n", actual);
    }
    if (fmt != NULL) {
        va_list args;
        va_start(args, fmt);
        _p1_print_user_msg(fmt, args);
        va_end(args);
    }
    _p1_trigger_failure();
}

static inline void _p1_fail_int_between(const char *file, int line, const char *expr,
                                        long long val, long long min_val, long long max_val,
                                        const char *fmt, ...) {
    _p1_print_fail_header(file, line, expr);
    if (!_p1_global_state.quiet_mode) {
        fprintf(stderr, "    esperado: valor acotado en [%lld, %lld]\n", min_val, max_val);
        fprintf(stderr, "    obtenido: %lld (fuera de rango)\n", val);
    }
    if (fmt != NULL) {
        va_list args;
        va_start(args, fmt);
        _p1_print_user_msg(fmt, args);
        va_end(args);
    }
    _p1_trigger_failure();
}

static inline void _p1_fail_uint(const char *file, int line, const char *expr,
                                 unsigned long long expected, unsigned long long actual,
                                 const char *op_name, const char *fmt, ...) {
    _p1_print_fail_header(file, line, expr);
    if (!_p1_global_state.quiet_mode) {
        fprintf(stderr, "    esperado: %s %llu\n", op_name, expected);
        fprintf(stderr, "    obtenido: %llu\n", actual);
    }
    if (fmt != NULL) {
        va_list args;
        va_start(args, fmt);
        _p1_print_user_msg(fmt, args);
        va_end(args);
    }
    _p1_trigger_failure();
}

static inline void _p1_fail_double(const char *file, int line, const char *expr,
                                   double expected, double actual, double epsilon,
                                   const char *fmt, ...) {
    _p1_print_fail_header(file, line, expr);
    if (!_p1_global_state.quiet_mode) {
        fprintf(stderr, "    esperado: %.7g (tolerancia +/- %.7g)\n", expected, epsilon);
        fprintf(stderr, "    obtenido: %.7g (diferencia: %.7g)\n", actual, _p1_abs_double(expected - actual));
    }
    if (fmt != NULL) {
        va_list args;
        va_start(args, fmt);
        _p1_print_user_msg(fmt, args);
        va_end(args);
    }
    _p1_trigger_failure();
}

static inline void _p1_fail_double_rel(const char *file, int line, const char *expr,
                                       double expected, double actual, double rel_tol,
                                       const char *fmt, ...) {
    _p1_print_fail_header(file, line, expr);
    double diff = _p1_abs_double(expected - actual);
    double max_mag = (_p1_abs_double(expected) > _p1_abs_double(actual)) ?
                     _p1_abs_double(expected) : _p1_abs_double(actual);
    double calc_rel = (max_mag > 0.0) ? (diff / max_mag) : 0.0;
    if (!_p1_global_state.quiet_mode) {
        fprintf(stderr, "    esperado: %.7g (tolerancia relativa: %.7g)\n", expected, rel_tol);
        fprintf(stderr, "    obtenido: %.7g (diferencia relativa: %.7g)\n", actual, calc_rel);
    }
    if (fmt != NULL) {
        va_list args;
        va_start(args, fmt);
        _p1_print_user_msg(fmt, args);
        va_end(args);
    }
    _p1_trigger_failure();
}

static inline void _p1_fail_str(const char *file, int line, const char *expr,
                                const char *expected, const char *actual,
                                const char *desc, const char *fmt, ...) {
    _p1_print_fail_header(file, line, expr);
    if (!_p1_global_state.quiet_mode) {
        fprintf(stderr, "    esperado: %s %s%s%s\n", desc,
                expected ? "\"" : "", expected ? expected : "NULL", expected ? "\"" : "");
        fprintf(stderr, "    obtenido: %s%s%s\n",
                actual ? "\"" : "", actual ? actual : "NULL", actual ? "\"" : "");
    }
    if (fmt != NULL) {
        va_list args;
        va_start(args, fmt);
        _p1_print_user_msg(fmt, args);
        va_end(args);
    }
    _p1_trigger_failure();
}

static inline void _p1_fail_ptr(const char *file, int line, const char *expr,
                                const void *expected, const void *actual,
                                const char *desc, const char *fmt, ...) {
    _p1_print_fail_header(file, line, expr);
    if (!_p1_global_state.quiet_mode) {
        if (expected == NULL && strcmp(desc, "distinto de") != 0) {
            fprintf(stderr, "    esperado: NULL\n");
        } else {
            fprintf(stderr, "    esperado: %s %p\n", desc, expected);
        }
        fprintf(stderr, "    obtenido: %p\n", actual);
    }
    if (fmt != NULL) {
        va_list args;
        va_start(args, fmt);
        _p1_print_user_msg(fmt, args);
        va_end(args);
    }
    _p1_trigger_failure();
}

static inline void _p1_fail_array_int(const char *file, int line, const char *expr,
                                      size_t index, long long expected, long long actual,
                                      const char *fmt, ...) {
    _p1_print_fail_header(file, line, expr);
    if (!_p1_global_state.quiet_mode) {
        fprintf(stderr, "    discrepancia en índice [%zu]:\n", index);
        fprintf(stderr, "      esperado: %lld\n", expected);
        fprintf(stderr, "      obtenido: %lld\n", actual);
    }
    if (fmt != NULL) {
        va_list args;
        va_start(args, fmt);
        _p1_print_user_msg(fmt, args);
        va_end(args);
    }
    _p1_trigger_failure();
}

static inline void _p1_fail_mem(const char *file, int line, const char *expr,
                                size_t diff_offset, unsigned char exp_byte, unsigned char act_byte,
                                const void *exp_buf, const void *act_buf, size_t total_size,
                                const char *fmt, ...) {
    _p1_print_fail_header(file, line, expr);
    if (!_p1_global_state.quiet_mode) {
        fprintf(stderr, "    discrepancia en byte offset 0x%zx (%zu de %zu bytes):\n",
                diff_offset, diff_offset, total_size);
        fprintf(stderr, "      esperado: 0x%02X\n", exp_byte);
        fprintf(stderr, "      obtenido: 0x%02X\n", act_byte);

        /* Volcado hexadecimal de contexto (hasta 8 bytes) */
        size_t start = (diff_offset >= 4) ? (diff_offset - 4) : 0;
        size_t end = (diff_offset + 4 < total_size) ? (diff_offset + 4) : total_size;
        fprintf(stderr, "    volcado esperado [%zu..%zu]: ", start, end - 1);
        for (size_t i = start; i < end; i++) {
            fprintf(stderr, "%02X ", ((const unsigned char *)exp_buf)[i]);
        }
        fprintf(stderr, "\n    volcado obtenido [%zu..%zu]: ", start, end - 1);
        for (size_t i = start; i < end; i++) {
            fprintf(stderr, "%02X ", ((const unsigned char *)act_buf)[i]);
        }
        fprintf(stderr, "\n");
    }
    if (fmt != NULL) {
        va_list args;
        va_start(args, fmt);
        _p1_print_user_msg(fmt, args);
        va_end(args);
    }
    _p1_trigger_failure();
}

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

/* --- Captura de señales fatales y watchdog de timeout --------------------- */
#if _P1_HAS_POSIX
static volatile sig_atomic_t _p1_caught_signal = 0;

static void _p1_signal_handler(int sig) {
    _p1_caught_signal = sig;
    const char *sig_name = "SEÑAL FATAL";
    if (sig == SIGSEGV) sig_name = "SIGSEGV (Violación de segmento / Puntero inválido)";
    else if (sig == SIGFPE) sig_name = "SIGFPE (Excepción aritmética / División por cero)";
    else if (sig == SIGALRM) sig_name = "SIGALRM (Timeout: Tiempo límite por test excedido)";
    else if (sig == SIGILL) sig_name = "SIGILL (Instrucción ilegal)";
#ifdef SIGBUS
    else if (sig == SIGBUS) sig_name = "SIGBUS (Error de bus)";
#endif

    if (!_p1_global_state.quiet_mode) {
        fprintf(stderr, "\n  %s[CRASH: %s]%s\n",
                _p1_clr(_P1_CLR_RED), sig_name, _p1_clr(_P1_CLR_RESET));
        if (_p1_global_state.current_test_name != NULL) {
            fprintf(stderr, "    %sen test:%s %s\n",
                    _p1_clr(_P1_CLR_CYAN), _p1_clr(_P1_CLR_RESET), _p1_global_state.current_test_name);
        }
        if (_p1_global_state.current_subcase != NULL) {
            fprintf(stderr, "    %sen subcaso:%s %s\n",
                    _p1_clr(_P1_CLR_YELLOW), _p1_clr(_P1_CLR_RESET), _p1_global_state.current_subcase);
        }
    }

    _p1_global_state.asserts_failed++;
    _p1_global_state.current_test_failed = 1;

    if (_p1_global_state.in_test_scope) {
        longjmp(_p1_global_state.jump_env, 1);
    } else {
        exit(1);
    }
}

static inline void _p1_setup_signals(void (**prev_segv)(int), void (**prev_fpe)(int), void (**prev_alrm)(int)) {
    *prev_segv = signal(SIGSEGV, _p1_signal_handler);
    *prev_fpe = signal(SIGFPE, _p1_signal_handler);
    *prev_alrm = signal(SIGALRM, _p1_signal_handler);
    if (_p1_global_state.timeout_seconds > 0) {
        alarm((unsigned int)_p1_global_state.timeout_seconds);
    }
}

static inline void _p1_restore_signals(void (*prev_segv)(int), void (*prev_fpe)(int), void (*prev_alrm)(int)) {
    if (_p1_global_state.timeout_seconds > 0) {
        alarm(0);
    }
    signal(SIGSEGV, prev_segv);
    signal(SIGFPE, prev_fpe);
    signal(SIGALRM, prev_alrm);
}

#define _P1_SETUP_SIGNALS(s, f, a) _p1_setup_signals(&(s), &(f), &(a))
#define _P1_RESTORE_SIGNALS(s, f, a) _p1_restore_signals(s, f, a)
#define _P1_SIG_VARS void (*_p1_prev_segv)(int) = NULL; void (*_p1_prev_fpe)(int) = NULL; void (*_p1_prev_alrm)(int) = NULL
#define _P1_SETJMP(env) setjmp(env)

#else

#define _P1_SETUP_SIGNALS(s, f, a) (void)0
#define _P1_RESTORE_SIGNALS(s, f, a) (void)0
#define _P1_SIG_VARS int _p1_sig_unused = 0
#define _P1_SETJMP(env) setjmp(env)

#endif

/* --- Captura de salida estándar (stdout) --------------------------------- */
#if _P1_HAS_POSIX
typedef struct {
    int saved_stdout;
    FILE *tmp_file;
} _p1_stdout_capture_t;

static _p1_stdout_capture_t _p1_capture_state = {-1, NULL};

static inline void _p1_capture_stdout_start(void) {
    fflush(stdout);
    _p1_capture_state.saved_stdout = dup(fileno(stdout));
    _p1_capture_state.tmp_file = tmpfile();
    if (_p1_capture_state.tmp_file != NULL && _p1_capture_state.saved_stdout >= 0) {
        dup2(fileno(_p1_capture_state.tmp_file), fileno(stdout));
    }
}

static inline void _p1_capture_stdout_finish(char *out_buf, size_t max_buf) {
    if (out_buf == NULL || max_buf == 0) return;
    out_buf[0] = '\0';
    if (_p1_capture_state.saved_stdout >= 0) {
        fflush(stdout);
        dup2(_p1_capture_state.saved_stdout, fileno(stdout));
        close(_p1_capture_state.saved_stdout);
        _p1_capture_state.saved_stdout = -1;
    }
    if (_p1_capture_state.tmp_file != NULL) {
        rewind(_p1_capture_state.tmp_file);
        size_t n = fread(out_buf, 1, max_buf - 1, _p1_capture_state.tmp_file);
        out_buf[n] = '\0';
        fclose(_p1_capture_state.tmp_file);
        _p1_capture_state.tmp_file = NULL;
    }
}
#endif

static inline void _p1_emergency_capture_cleanup(void) {
#if _P1_HAS_POSIX
    if (_p1_capture_state.saved_stdout >= 0) {
        fflush(stdout);
        dup2(_p1_capture_state.saved_stdout, fileno(stdout));
        close(_p1_capture_state.saved_stdout);
        _p1_capture_state.saved_stdout = -1;
    }
    if (_p1_capture_state.tmp_file != NULL) {
        fclose(_p1_capture_state.tmp_file);
        _p1_capture_state.tmp_file = NULL;
    }
#endif
}

/* --- Parser de argumentos CLI y configuración ---------------------------- */

static inline void _p1_parse_args(int argc, char **argv) {
    _p1_global_state.timeout_seconds = 5; /* Default: 5 segundos por test */
    if (argc <= 1 || argv == NULL) return;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--fail-fast") == 0) {
            _p1_global_state.fail_fast = 1;
        } else if (strcmp(argv[i], "-q") == 0 || strcmp(argv[i], "--quiet") == 0) {
            _p1_global_state.quiet_mode = 1;
        } else if (strcmp(argv[i], "--tap") == 0) {
            _p1_global_state.tap_mode = 1;
        } else if (strcmp(argv[i], "--no-color") == 0) {
            _p1_global_state.no_color = 1;
        } else if ((strcmp(argv[i], "-k") == 0 || strcmp(argv[i], "--filter") == 0) && i + 1 < argc) {
            _p1_global_state.filter = argv[++i];
        } else if ((strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--timeout") == 0) && i + 1 < argc) {
            _p1_global_state.timeout_seconds = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("Uso: %s [opciones] [filtro]\n", argv[0]);
            printf("Opciones:\n");
            printf("  -k, --filter <texto>   Ejecuta solo tests cuyo nombre contenga <texto>\n");
            printf("  -f, --fail-fast        Detiene la suite tras el primer test fallido\n");
            printf("  -q, --quiet            Modo silencioso/compacto (emite '.' y 'F')\n");
            printf("  -t, --timeout <seg>    Límite de tiempo por test (segundos, default: 5, 0=desactivado)\n");
            printf("      --tap              Salida en formato Test Anything Protocol v13\n");
            printf("      --no-color         Desactiva colores ANSI\n");
            printf("  -h, --help             Muestra este mensaje y sale\n");
            exit(0);
        } else if (argv[i][0] != '-') {
            _p1_global_state.filter = argv[i];
        }
    }
}

/* ========================================================================= */
/* --- MACROS DE INICIALIZACIÓN, SUITES Y CONTROL DE FLUJO ----------------- */
/* ========================================================================= */

/**
 * @def TEST(name)
 * @brief Declara una función de prueba unitaria.
 */
#define TEST(name) static void _p1_test_func_##name(void)

/**
 * @def BEFORE_EACH(fn)
 * @brief Registra una función de inicialización que corre antes de cada test.
 */
#define BEFORE_EACH(fn) do { _p1_global_state.before_each = (fn); } while (0)

/**
 * @def AFTER_EACH(fn)
 * @brief Registra una función de limpieza que corre después de cada test.
 */
#define AFTER_EACH(fn) do { _p1_global_state.after_each = (fn); } while (0)

/**
 * @def SUBCASE(desc)
 * @brief Establece una sección o subcaso lógico descriptivo dentro de un test.
 */
#define SUBCASE(desc) do { _p1_global_state.current_subcase = (desc); } while (0)

/**
 * @def TEST_SUITE_BEGIN_ARGS(suite_title, argc, argv)
 * @brief Inicializa la suite procesando argumentos de línea de comandos.
 */
#define TEST_SUITE_BEGIN_ARGS(suite_title, argc, argv) do { \
    _p1_global_state.suite_name = (suite_title); \
    _p1_global_state.tests_run = 0; \
    _p1_global_state.tests_passed = 0; \
    _p1_global_state.tests_failed = 0; \
    _p1_global_state.tests_skipped = 0; \
    _p1_global_state.asserts_total = 0; \
    _p1_global_state.asserts_failed = 0; \
    _p1_global_state.current_test_failed = 0; \
    _p1_global_state.current_test_skipped = 0; \
    _p1_global_state.in_test_scope = 0; \
    _p1_global_state.current_subcase = NULL; \
    _p1_global_state.tap_test_index = 0; \
    _p1_parse_args(argc, argv); \
    if (_p1_global_state.tap_mode) { \
        printf("TAP version 13\n"); \
    } else if (!_p1_global_state.quiet_mode) { \
        printf("\n%s=== INICIANDO SUITE: %s ===%s\n", \
               _p1_clr(_P1_CLR_BOLD), _p1_global_state.suite_name, _p1_clr(_P1_CLR_RESET)); \
        if (_p1_global_state.filter != NULL) { \
            printf("  [Filtro activo: '%s']\n", _p1_global_state.filter); \
        } \
        printf("\n"); \
    } \
} while (0)

/**
 * @def TEST_SUITE_BEGIN(suite_title)
 * @brief Inicializa la suite con configuración por defecto (sin flags CLI).
 */
#define TEST_SUITE_BEGIN(suite_title) TEST_SUITE_BEGIN_ARGS(suite_title, 0, NULL)

/**
 * @def SKIP_TEST(name, reason)
 * @brief Omite un test completo reportándolo como salteado.
 */
#define SKIP_TEST(name, reason) do { \
    if (_p1_global_state.filter == NULL || strstr(#name, _p1_global_state.filter) != NULL) { \
        _p1_global_state.tests_skipped++; \
        if (_p1_global_state.tap_mode) { \
            _p1_global_state.tap_test_index++; \
            printf("ok %d - %s # SKIP %s\n", _p1_global_state.tap_test_index, #name, reason); \
        } else if (_p1_global_state.quiet_mode) { \
            printf("%sS%s", _p1_clr(_P1_CLR_YELLOW), _p1_clr(_P1_CLR_RESET)); \
            fflush(stdout); \
        } else { \
            printf("  [SALTEADO] %s ... %s (%s)\n", #name, _p1_clr(_P1_CLR_YELLOW), reason); \
        } \
    } \
} while (0)

/**
 * @def TEST_SKIP(reason)
 * @brief Saltea la ejecución del test actual desde su propio cuerpo.
 */
#define TEST_SKIP(reason) do { \
    _p1_global_state.current_test_skipped = 1; \
    if (!_p1_global_state.quiet_mode && !_p1_global_state.tap_mode) { \
        printf("%s(salteado: %s)%s ", _p1_clr(_P1_CLR_YELLOW), reason, _p1_clr(_P1_CLR_RESET)); \
    } \
    if (_p1_global_state.in_test_scope) { \
        longjmp(_p1_global_state.jump_env, 2); \
    } \
} while (0)

/**
 * @def RUN_TEST(name)
 * @brief Ejecuta el caso de prueba con aislamiento, medición de tiempo y señales.
 */
#define RUN_TEST(name) do { \
    if (_p1_global_state.filter != NULL && strstr(#name, _p1_global_state.filter) == NULL) { \
        break; \
    } \
    if (_p1_global_state.fail_fast && _p1_global_state.tests_failed > 0) { \
        break; \
    } \
    _p1_global_state.tests_run++; \
    _p1_global_state.current_test_name = #name; \
    _p1_global_state.current_test_failed = 0; \
    _p1_global_state.current_test_skipped = 0; \
    _p1_global_state.current_subcase = NULL; \
    _p1_global_state.in_test_scope = 1; \
    if (!_p1_global_state.quiet_mode && !_p1_global_state.tap_mode) { \
        printf("  [CORRIENDO] %-35s ... ", #name); \
        fflush(stdout); \
    } \
    if (_p1_global_state.before_each != NULL) { \
        _p1_global_state.before_each(); \
    } \
    clock_t _p1_start_t = clock(); \
    int _p1_jump_res = 0; \
    _P1_SIG_VARS; \
    _P1_SETUP_SIGNALS(_p1_prev_segv, _p1_prev_fpe, _p1_prev_alrm); \
    int _p1_setjmp_val = _P1_SETJMP(_p1_global_state.jump_env); \
    if (_p1_setjmp_val == 0) { \
        _p1_test_func_##name(); \
    } else { \
        _p1_jump_res = _p1_setjmp_val; \
    } \
    _P1_RESTORE_SIGNALS(_p1_prev_segv, _p1_prev_fpe, _p1_prev_alrm); \
    if (_p1_global_state.cleanup_hook != NULL) { \
        _p1_global_state.cleanup_hook(); \
        _p1_global_state.cleanup_hook = NULL; \
    } \
    _p1_emergency_capture_cleanup(); \
    clock_t _p1_end_t = clock(); \
    _p1_global_state.current_elapsed_ms = ((double)(_p1_end_t - _p1_start_t) / (double)CLOCKS_PER_SEC) * 1000.0; \
    if (_p1_global_state.after_each != NULL) { \
        _p1_global_state.after_each(); \
    } \
    _p1_global_state.in_test_scope = 0; \
    _p1_global_state.tap_test_index++; \
    if (_p1_jump_res == 2 || _p1_global_state.current_test_skipped) { \
        _p1_global_state.tests_skipped++; \
        if (_p1_global_state.tap_mode) { \
            printf("ok %d - %s # SKIP\n", _p1_global_state.tap_test_index, #name); \
        } else if (_p1_global_state.quiet_mode) { \
            printf("%sS%s", _p1_clr(_P1_CLR_YELLOW), _p1_clr(_P1_CLR_RESET)); \
            fflush(stdout); \
        } else { \
            printf("%sSALTEADO%s (%.2f ms)\n", _p1_clr(_P1_CLR_YELLOW), _p1_clr(_P1_CLR_RESET), _p1_global_state.current_elapsed_ms); \
        } \
    } else if (_p1_global_state.current_test_failed) { \
        _p1_global_state.tests_failed++; \
        if (_p1_global_state.tap_mode) { \
            printf("not ok %d - %s\n", _p1_global_state.tap_test_index, #name); \
        } else if (_p1_global_state.quiet_mode) { \
            printf("%sF%s", _p1_clr(_P1_CLR_RED), _p1_clr(_P1_CLR_RESET)); \
            fflush(stdout); \
        } else { \
            printf("  [RESULTADO] %sFALLÓ%s (%.2f ms)\n", _p1_clr(_P1_CLR_RED), _p1_clr(_P1_CLR_RESET), _p1_global_state.current_elapsed_ms); \
        } \
    } else { \
        _p1_global_state.tests_passed++; \
        if (_p1_global_state.tap_mode) { \
            printf("ok %d - %s\n", _p1_global_state.tap_test_index, #name); \
        } else if (_p1_global_state.quiet_mode) { \
            printf("%s.%s", _p1_clr(_P1_CLR_GREEN), _p1_clr(_P1_CLR_RESET)); \
            fflush(stdout); \
        } else { \
            printf("%sPASÓ%s (%.2f ms)\n", _p1_clr(_P1_CLR_GREEN), _p1_clr(_P1_CLR_RESET), _p1_global_state.current_elapsed_ms); \
        } \
    } \
    _p1_global_state.current_test_name = NULL; \
    _p1_global_state.current_subcase = NULL; \
} while (0)

/**
 * @def RUN_TEST_SEEDED(name, seed)
 * @brief Ejecuta el test restableciendo una semilla determinística para rand().
 */
#define RUN_TEST_SEEDED(name, seed) do { \
    srand((unsigned int)(seed)); \
    RUN_TEST(name); \
} while (0)

/**
 * @def TEST_REPORT()
 * @brief Imprime el reporte consolidado de la suite y retorna código de salida.
 * @return 0 si todos los tests pasaron, 1 si al menos uno falló.
 */
static inline int TEST_REPORT(void) {
    if (_p1_global_state.quiet_mode) {
        printf("\n");
    }

    if (_p1_global_state.tap_mode) {
        printf("1..%d\n", _p1_global_state.tap_test_index);
        printf("# Suite '%s': pasados: %d, fallados: %d, salteados: %d\n",
               _p1_global_state.suite_name,
               _p1_global_state.tests_passed,
               _p1_global_state.tests_failed,
               _p1_global_state.tests_skipped);
        return (_p1_global_state.tests_failed == 0) ? 0 : 1;
    }

    printf("\n%s============================================================%s\n",
           _p1_clr(_P1_CLR_BOLD), _p1_clr(_P1_CLR_RESET));
    printf("%sResultados de la Suite: %s%s\n",
           _p1_clr(_P1_CLR_BOLD), _p1_global_state.suite_name, _p1_clr(_P1_CLR_RESET));
    printf("============================================================\n");
    printf("  Total de tests ejecutados : %d\n", _p1_global_state.tests_run);
    printf("  Tests aprobados           : %s%d%s\n",
           _p1_global_state.tests_passed > 0 ? _p1_clr(_P1_CLR_GREEN) : "",
           _p1_global_state.tests_passed, _p1_clr(_P1_CLR_RESET));
    printf("  Tests desaprobados        : %s%d%s\n",
           _p1_global_state.tests_failed > 0 ? _p1_clr(_P1_CLR_RED) : "",
           _p1_global_state.tests_failed, _p1_clr(_P1_CLR_RESET));
    printf("  Tests salteados           : %s%d%s\n",
           _p1_global_state.tests_skipped > 0 ? _p1_clr(_P1_CLR_YELLOW) : "",
           _p1_global_state.tests_skipped, _p1_clr(_P1_CLR_RESET));
    printf("  Aserciones evaluadas      : %d (falladas: %d)\n",
           _p1_global_state.asserts_total, _p1_global_state.asserts_failed);
    printf("============================================================\n");

    if (_p1_global_state.tests_failed == 0) {
        printf("%s>>> ESTADO: TODOS LOS TESTS PASARON EXITOSAMENTE <<<%s\n\n",
               _p1_clr(_P1_CLR_GREEN), _p1_clr(_P1_CLR_RESET));
        return 0;
    } else {
        printf("%s>>> ESTADO: SE DETECTARON FALLOS EN LAS PRUEBAS <<<%s\n\n",
               _p1_clr(_P1_CLR_RED), _p1_clr(_P1_CLR_RESET));
        return 1;
    }
}

/* ========================================================================= */
/* --- CATÁLOGO DE ASERCIONES TIPADAS -------------------------------------- */
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

#define ASSERT_INT_BETWEEN_MSG(val, min_val, max_val, ...) do { \
    _p1_global_state.asserts_total++; \
    long long _v = (long long)(val); \
    long long _min = (long long)(min_val); \
    long long _max = (long long)(max_val); \
    if (!(_v >= _min && _v <= _max)) { \
        _p1_fail_int_between(__FILE__, __LINE__, \
                             "ASSERT_INT_BETWEEN(" #val ", " #min_val ", " #max_val ")", \
                             _v, _min, _max, __VA_ARGS__); \
    } \
} while (0)

#define ASSERT_INT_BETWEEN(val, min_val, max_val) \
    ASSERT_INT_BETWEEN_MSG(val, min_val, max_val, NULL)

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

#define ASSERT_DOUBLE_NEAR_REL_MSG(expected, actual, rel_tol, ...) do { \
    _p1_global_state.asserts_total++; \
    double _dexp = (double)(expected); \
    double _dact = (double)(actual); \
    double _dtol = (double)(rel_tol); \
    double _diff = _p1_abs_double(_dexp - _dact); \
    double _max_mag = (_p1_abs_double(_dexp) > _p1_abs_double(_dact)) ? \
                      _p1_abs_double(_dexp) : _p1_abs_double(_dact); \
    if (_diff > (_max_mag * _dtol)) { \
        _p1_fail_double_rel(__FILE__, __LINE__, \
                            "ASSERT_DOUBLE_NEAR_REL(" #expected ", " #actual ", " #rel_tol ")", \
                            _dexp, _dact, _dtol, __VA_ARGS__); \
    } \
} while (0)

#define ASSERT_DOUBLE_NEAR_REL(expected, actual, rel_tol) \
    ASSERT_DOUBLE_NEAR_REL_MSG(expected, actual, rel_tol, NULL)

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

#define ASSERT_STR_CASE_EQ_MSG(expected, actual, ...) do { \
    _p1_global_state.asserts_total++; \
    const char *_sexp = (const char *)(expected); \
    const char *_sact = (const char *)(actual); \
    int _equal = (_p1_strcasecmp(_sexp, _sact) == 0); \
    if (!_equal) { \
        _p1_fail_str(__FILE__, __LINE__, "ASSERT_STR_CASE_EQ(" #expected ", " #actual ")", \
                     _sexp, _sact, "igual (sin mayúsculas) a", __VA_ARGS__); \
    } \
} while (0)

#define ASSERT_STR_CASE_EQ(expected, actual) ASSERT_STR_CASE_EQ_MSG(expected, actual, NULL)

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

/* --- Aserciones sobre Arreglos de Enteros --- */

#define ASSERT_ARRAY_INT_EQ_MSG(expected, actual, length, ...) do { \
    size_t _len = (size_t)(length); \
    const int *_aexp = (const int *)(expected); \
    const int *_aact = (const int *)(actual); \
    for (size_t _i = 0; _i < _len; _i++) { \
        _p1_global_state.asserts_total++; \
        if (_aexp[_i] != _aact[_i]) { \
            _p1_fail_array_int(__FILE__, __LINE__, \
                               "ASSERT_ARRAY_INT_EQ(" #expected ", " #actual ", " #length ")", \
                               _i, (long long)_aexp[_i], (long long)_aact[_i], __VA_ARGS__); \
            break; \
        } \
    } \
} while (0)

#define ASSERT_ARRAY_INT_EQ(expected, actual, length) \
    ASSERT_ARRAY_INT_EQ_MSG(expected, actual, length, NULL)

/* --- Aserciones sobre Memoria Binaria (memcmp con hexdump) --- */

#define ASSERT_MEM_EQ_MSG(expected, actual, size, ...) do { \
    size_t _sz = (size_t)(size); \
    const unsigned char *_mexp = (const unsigned char *)(expected); \
    const unsigned char *_mact = (const unsigned char *)(actual); \
    _p1_global_state.asserts_total++; \
    int _mcmp = memcmp(_mexp, _mact, _sz); \
    if (_mcmp != 0) { \
        size_t _diff_idx = 0; \
        while (_diff_idx < _sz && _mexp[_diff_idx] == _mact[_diff_idx]) { \
            _diff_idx++; \
        } \
        _p1_fail_mem(__FILE__, __LINE__, \
                     "ASSERT_MEM_EQ(" #expected ", " #actual ", " #size ")", \
                     _diff_idx, _mexp[_diff_idx], _mact[_diff_idx], \
                     _mexp, _mact, _sz, __VA_ARGS__); \
    } \
} while (0)

#define ASSERT_MEM_EQ(expected, actual, size) \
    ASSERT_MEM_EQ_MSG(expected, actual, size, NULL)

/* --- Aserción sobre Salida Estándar (captura stdout) --- */

#if _P1_HAS_POSIX
#define ASSERT_STDOUT_EQ(statement, expected) do { \
    char _p1_out_captured[4096] = {0}; \
    _p1_capture_stdout_start(); \
    do { statement; } while (0); \
    _p1_capture_stdout_finish(_p1_out_captured, sizeof(_p1_out_captured)); \
    ASSERT_STR_EQ(expected, _p1_out_captured); \
} while (0)
#endif

/* --- Aserción de Fallo Explícito --- */

#define ASSERT_FAIL(...) do { \
    _p1_global_state.asserts_total++; \
    _p1_fail_explicit(__FILE__, __LINE__, __VA_ARGS__); \
} while (0)

#ifdef __cplusplus
}
#endif

#endif /* P1_TEST_H */
