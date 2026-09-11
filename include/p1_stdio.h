/**
 * @file p1_stdio.h
 * @brief Mocks y captura de flujos estándar (stdin, stdout, stderr) para p1_test.
 * @version 1.0.0
 * 
 * Permite simular entradas por teclado (stdin) para probar scanf/fgets de forma
 * no interactiva, capturar salidas de consola (stdout y stderr) y verificar
 * que los flujos coincidan con lo esperado.
 */

#ifndef P1_STDIO_H
#define P1_STDIO_H

#include "p1_test.h"

#ifdef __cplusplus
extern "C" {
#endif

/* --- Estructura interna para mocking y captura de stdio ------------------ */

typedef struct {
    int saved_stdin_fd;
    FILE *mock_stdin_file;

    int saved_stdout_fd;
    FILE *capture_stdout_file;

    int saved_stderr_fd;
    FILE *capture_stderr_file;
} _p1_stdio_state_t;

static _p1_stdio_state_t _p1_stdio_ctx = {-1, NULL, -1, NULL, -1, NULL};

static inline void p1_mock_stdin_restore(void);

static inline void _p1_stdio_emergency_cleanup(void) {
    p1_mock_stdin_restore();
    if (_p1_stdio_ctx.saved_stdout_fd >= 0) {
        fflush(stdout);
        dup2(_p1_stdio_ctx.saved_stdout_fd, fileno(stdout));
        close(_p1_stdio_ctx.saved_stdout_fd);
        _p1_stdio_ctx.saved_stdout_fd = -1;
    }
    if (_p1_stdio_ctx.capture_stdout_file != NULL) {
        fclose(_p1_stdio_ctx.capture_stdout_file);
        _p1_stdio_ctx.capture_stdout_file = NULL;
    }
    if (_p1_stdio_ctx.saved_stderr_fd >= 0) {
        fflush(stderr);
        dup2(_p1_stdio_ctx.saved_stderr_fd, fileno(stderr));
        close(_p1_stdio_ctx.saved_stderr_fd);
        _p1_stdio_ctx.saved_stderr_fd = -1;
    }
    if (_p1_stdio_ctx.capture_stderr_file != NULL) {
        fclose(_p1_stdio_ctx.capture_stderr_file);
        _p1_stdio_ctx.capture_stderr_file = NULL;
    }
    _p1_global_state.cleanup_hook = NULL;
}

/* --- Gestión de Mock de stdin --- */

static inline void p1_mock_stdin_feed(const char *input_str) {
    if (_p1_stdio_ctx.saved_stdin_fd >= 0) {
        /* Ya estaba activo un mock, restaurar primero */
        dup2(_p1_stdio_ctx.saved_stdin_fd, fileno(stdin));
        close(_p1_stdio_ctx.saved_stdin_fd);
        _p1_stdio_ctx.saved_stdin_fd = -1;
        if (_p1_stdio_ctx.mock_stdin_file) {
            fclose(_p1_stdio_ctx.mock_stdin_file);
            _p1_stdio_ctx.mock_stdin_file = NULL;
        }
    }

    _p1_stdio_ctx.saved_stdin_fd = dup(fileno(stdin));
    _p1_stdio_ctx.mock_stdin_file = tmpfile();
    if (_p1_stdio_ctx.mock_stdin_file != NULL && input_str != NULL) {
        fputs(input_str, _p1_stdio_ctx.mock_stdin_file);
        rewind(_p1_stdio_ctx.mock_stdin_file);
        dup2(fileno(_p1_stdio_ctx.mock_stdin_file), fileno(stdin));
    }
    _p1_global_state.cleanup_hook = _p1_stdio_emergency_cleanup;
    p1_register_cleanup_hook(_p1_stdio_emergency_cleanup);
}

static inline void p1_mock_stdin_restore(void) {
    if (_p1_stdio_ctx.saved_stdin_fd >= 0) {
        dup2(_p1_stdio_ctx.saved_stdin_fd, fileno(stdin));
        close(_p1_stdio_ctx.saved_stdin_fd);
        _p1_stdio_ctx.saved_stdin_fd = -1;
    }
    if (_p1_stdio_ctx.mock_stdin_file != NULL) {
        fclose(_p1_stdio_ctx.mock_stdin_file);
        _p1_stdio_ctx.mock_stdin_file = NULL;
    }
    if (_p1_stdio_ctx.saved_stdin_fd < 0 && _p1_stdio_ctx.saved_stdout_fd < 0 && _p1_stdio_ctx.saved_stderr_fd < 0) {
        _p1_global_state.cleanup_hook = NULL;
    }
}

/* --- Gestión de Captura de stdout --- */

static inline void p1_capture_stdout_begin(void) {
    fflush(stdout);
    if (_p1_stdio_ctx.saved_stdout_fd >= 0) {
        dup2(_p1_stdio_ctx.saved_stdout_fd, fileno(stdout));
        close(_p1_stdio_ctx.saved_stdout_fd);
        _p1_stdio_ctx.saved_stdout_fd = -1;
        if (_p1_stdio_ctx.capture_stdout_file) {
            fclose(_p1_stdio_ctx.capture_stdout_file);
            _p1_stdio_ctx.capture_stdout_file = NULL;
        }
    }
    _p1_stdio_ctx.saved_stdout_fd = dup(fileno(stdout));
    _p1_stdio_ctx.capture_stdout_file = tmpfile();
    if (_p1_stdio_ctx.capture_stdout_file != NULL && _p1_stdio_ctx.saved_stdout_fd >= 0) {
        dup2(fileno(_p1_stdio_ctx.capture_stdout_file), fileno(stdout));
    }
    _p1_global_state.cleanup_hook = _p1_stdio_emergency_cleanup;
    p1_register_cleanup_hook(_p1_stdio_emergency_cleanup);
}

static inline void p1_capture_stdout_end(char *buf, size_t max_len) {
    if (buf == NULL || max_len == 0) return;
    buf[0] = '\0';
    if (_p1_stdio_ctx.saved_stdout_fd >= 0) {
        fflush(stdout);
        dup2(_p1_stdio_ctx.saved_stdout_fd, fileno(stdout));
        close(_p1_stdio_ctx.saved_stdout_fd);
        _p1_stdio_ctx.saved_stdout_fd = -1;
    }
    if (_p1_stdio_ctx.capture_stdout_file != NULL) {
        rewind(_p1_stdio_ctx.capture_stdout_file);
        size_t n = fread(buf, 1, max_len - 1, _p1_stdio_ctx.capture_stdout_file);
        buf[n] = '\0';
        fclose(_p1_stdio_ctx.capture_stdout_file);
        _p1_stdio_ctx.capture_stdout_file = NULL;
    }
    if (_p1_stdio_ctx.saved_stdin_fd < 0 && _p1_stdio_ctx.saved_stdout_fd < 0 && _p1_stdio_ctx.saved_stderr_fd < 0) {
        _p1_global_state.cleanup_hook = NULL;
    }
}

/* --- Gestión de Captura de stderr --- */

static inline void p1_capture_stderr_begin(void) {
    fflush(stderr);
    if (_p1_stdio_ctx.saved_stderr_fd >= 0) {
        dup2(_p1_stdio_ctx.saved_stderr_fd, fileno(stderr));
        close(_p1_stdio_ctx.saved_stderr_fd);
        _p1_stdio_ctx.saved_stderr_fd = -1;
        if (_p1_stdio_ctx.capture_stderr_file) {
            fclose(_p1_stdio_ctx.capture_stderr_file);
            _p1_stdio_ctx.capture_stderr_file = NULL;
        }
    }
    _p1_stdio_ctx.saved_stderr_fd = dup(fileno(stderr));
    _p1_stdio_ctx.capture_stderr_file = tmpfile();
    if (_p1_stdio_ctx.capture_stderr_file != NULL && _p1_stdio_ctx.saved_stderr_fd >= 0) {
        dup2(fileno(_p1_stdio_ctx.capture_stderr_file), fileno(stderr));
    }
    _p1_global_state.cleanup_hook = _p1_stdio_emergency_cleanup;
    p1_register_cleanup_hook(_p1_stdio_emergency_cleanup);
}

static inline void p1_capture_stderr_end(char *buf, size_t max_len) {
    if (buf == NULL || max_len == 0) return;
    buf[0] = '\0';
    if (_p1_stdio_ctx.saved_stderr_fd >= 0) {
        fflush(stderr);
        dup2(_p1_stdio_ctx.saved_stderr_fd, fileno(stderr));
        close(_p1_stdio_ctx.saved_stderr_fd);
        _p1_stdio_ctx.saved_stderr_fd = -1;
    }
    if (_p1_stdio_ctx.capture_stderr_file != NULL) {
        rewind(_p1_stdio_ctx.capture_stderr_file);
        size_t n = fread(buf, 1, max_len - 1, _p1_stdio_ctx.capture_stderr_file);
        buf[n] = '\0';
        fclose(_p1_stdio_ctx.capture_stderr_file);
        _p1_stdio_ctx.capture_stderr_file = NULL;
    }
    if (_p1_stdio_ctx.saved_stdin_fd < 0 && _p1_stdio_ctx.saved_stdout_fd < 0 && _p1_stdio_ctx.saved_stderr_fd < 0) {
        _p1_global_state.cleanup_hook = NULL;
    }
}

/* --- Funciones de fallo para stdio --- */

static inline void _p1_fail_stdin_consumed(const char *file, int line, const char *expr,
                                           const char *fmt, ...) {
    _p1_print_fail_header(file, line, expr);
    if (!_p1_global_state.quiet_mode) {
        fprintf(stderr, "    quedaron caracteres sin leer en stdin (se esperaba buffer vacío / EOF)\n");
    }
    if (fmt != NULL) {
        va_list args;
        va_start(args, fmt);
        _p1_print_user_msg(fmt, args);
        va_end(args);
    }
    _p1_trigger_failure();
}

/* ========================================================================= */
/* --- MACROS DE ASERCIÓN Y MOCK DE STDIO ----------------------------------- */
/* ========================================================================= */

/**
 * @def ASSERT_STDOUT_EQ(statement, expected)
 * @brief Ejecuta statement capturando stdout y comprueba que coincida con expected.
 */
#ifndef ASSERT_STDOUT_EQ
#define ASSERT_STDOUT_EQ_MSG(statement, expected, ...) do { \
    char _p1_out_captured[4096] = {0}; \
    p1_capture_stdout_begin(); \
    do { statement; } while (0); \
    p1_capture_stdout_end(_p1_out_captured, sizeof(_p1_out_captured)); \
    ASSERT_STR_EQ_MSG(expected, _p1_out_captured, __VA_ARGS__); \
} while (0)

#define ASSERT_STDOUT_EQ(statement, expected) \
    ASSERT_STDOUT_EQ_MSG(statement, expected, NULL)
#endif

/**
 * @def ASSERT_STDERR_EQ(statement, expected)
 * @brief Ejecuta statement capturando stderr y comprueba que coincida con expected.
 */
#define ASSERT_STDERR_EQ_MSG(statement, expected, ...) do { \
    char _p1_err_captured[4096] = {0}; \
    p1_capture_stderr_begin(); \
    do { statement; } while (0); \
    p1_capture_stderr_end(_p1_err_captured, sizeof(_p1_err_captured)); \
    ASSERT_STR_EQ_MSG(expected, _p1_err_captured, __VA_ARGS__); \
} while (0)

#define ASSERT_STDERR_EQ(statement, expected) \
    ASSERT_STDERR_EQ_MSG(statement, expected, NULL)

/**
 * @def ASSERT_STDIO_EQ(statement, stdin_input, expected_stdout)
 * @brief Simula la entrada por stdin, ejecuta statement, captura stdout y compara el resultado.
 */
#define ASSERT_STDIO_EQ_MSG(statement, stdin_input, expected_stdout, ...) do { \
    char _p1_stdio_out[4096] = {0}; \
    p1_mock_stdin_feed(stdin_input); \
    p1_capture_stdout_begin(); \
    do { statement; } while (0); \
    p1_capture_stdout_end(_p1_stdio_out, sizeof(_p1_stdio_out)); \
    p1_mock_stdin_restore(); \
    ASSERT_STR_EQ_MSG(expected_stdout, _p1_stdio_out, __VA_ARGS__); \
} while (0)

#define ASSERT_STDIO_EQ(statement, stdin_input, expected_stdout) \
    ASSERT_STDIO_EQ_MSG(statement, stdin_input, expected_stdout, NULL)

/**
 * @def ASSERT_STDIN_CONSUMED()
 * @brief Valida que el mock de stdin haya alcanzado el final de archivo (EOF).
 */
#define ASSERT_STDIN_CONSUMED_MSG(...) do { \
    _p1_global_state.asserts_total++; \
    int _ch = fgetc(stdin); \
    if (_ch != EOF) { \
        ungetc(_ch, stdin); \
        _p1_fail_stdin_consumed(__FILE__, __LINE__, "ASSERT_STDIN_CONSUMED()", __VA_ARGS__); \
    } \
} while (0)

#define ASSERT_STDIN_CONSUMED() ASSERT_STDIN_CONSUMED_MSG(NULL)

/**
 * @def ASSERT_STDOUT_CONTAINS(statement, needle)
 * @brief Ejecuta statement capturando stdout y verifica que contenga needle.
 */
#define ASSERT_STDOUT_CONTAINS_MSG(statement, needle, ...) do { \
    char _p1_out_captured[4096] = {0}; \
    p1_capture_stdout_begin(); \
    do { statement; } while (0); \
    p1_capture_stdout_end(_p1_out_captured, sizeof(_p1_out_captured)); \
    ASSERT_STR_CONTAINS_MSG(_p1_out_captured, needle, __VA_ARGS__); \
} while (0)

#define ASSERT_STDOUT_CONTAINS(statement, needle) \
    ASSERT_STDOUT_CONTAINS_MSG(statement, needle, NULL)

/**
 * @def ASSERT_STDERR_CONTAINS(statement, needle)
 * @brief Ejecuta statement capturando stderr y verifica que contenga needle.
 */
#define ASSERT_STDERR_CONTAINS_MSG(statement, needle, ...) do { \
    char _p1_err_captured[4096] = {0}; \
    p1_capture_stderr_begin(); \
    do { statement; } while (0); \
    p1_capture_stderr_end(_p1_err_captured, sizeof(_p1_err_captured)); \
    ASSERT_STR_CONTAINS_MSG(_p1_err_captured, needle, __VA_ARGS__); \
} while (0)

#define ASSERT_STDERR_CONTAINS(statement, needle) \
    ASSERT_STDERR_CONTAINS_MSG(statement, needle, NULL)

/* --- Macros de E/S con Pistas Pedagógicas (QoL 4) ------------------------- */

#define ASSERT_STDOUT_EQ_HINT(statement, expected, hint) do { \
    _p1_global_state.current_hint = (hint); \
    ASSERT_STDOUT_EQ(statement, expected); \
    _p1_global_state.current_hint = NULL; \
} while (0)

#define ASSERT_STDERR_EQ_HINT(statement, expected, hint) do { \
    _p1_global_state.current_hint = (hint); \
    ASSERT_STDERR_EQ(statement, expected); \
    _p1_global_state.current_hint = NULL; \
} while (0)

#define ASSERT_STDIO_EQ_HINT(statement, stdin_input, expected_stdout, hint) do { \
    _p1_global_state.current_hint = (hint); \
    ASSERT_STDIO_EQ(statement, stdin_input, expected_stdout); \
    _p1_global_state.current_hint = NULL; \
} while (0)

#define ASSERT_STDOUT_CONTAINS_HINT(statement, needle, hint) do { \
    _p1_global_state.current_hint = (hint); \
    ASSERT_STDOUT_CONTAINS(statement, needle); \
    _p1_global_state.current_hint = NULL; \
} while (0)

#define ASSERT_STDERR_CONTAINS_HINT(statement, needle, hint) do { \
    _p1_global_state.current_hint = (hint); \
    ASSERT_STDERR_CONTAINS(statement, needle); \
    _p1_global_state.current_hint = NULL; \
} while (0)

#define ASSERT_STDIN_CONSUMED_HINT(hint) do { \
    _p1_global_state.current_hint = (hint); \
    ASSERT_STDIN_CONSUMED(); \
    _p1_global_state.current_hint = NULL; \
} while (0)

#ifdef __cplusplus
}
#endif

#endif /* P1_STDIO_H */
