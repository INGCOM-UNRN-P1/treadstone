/**
 * @file p1_files.h
 * @brief Aserciones avanzadas para archivos de texto y binarios en C11.
 * @version 1.0.0
 * 
 * Permite validar existencia, contenido de texto línea por línea y coincidencia
 * binaria byte a byte con volcado hexadecimal ante discrepancias.
 */

#ifndef P1_FILES_H
#define P1_FILES_H

#include "p1_test.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================= */
/* --- GESTOR DIDÁCTICO DE ARCHIVOS TEMPORALES (QoL 9) --------------------- */
/* ========================================================================= */

#define _P1_MAX_TEMP_FILES 64

typedef struct {
    char paths[_P1_MAX_TEMP_FILES][512];
    size_t count;
} _p1_temp_file_registry_t;

static _p1_temp_file_registry_t _p1_temp_files = { { {0} }, 0 };

static inline void p1_temp_files_cleanup(void) {
    for (size_t i = 0; i < _p1_temp_files.count; i++) {
        if (_p1_temp_files.paths[i][0] != '\0') {
            remove(_p1_temp_files.paths[i]);
            _p1_temp_files.paths[i][0] = '\0';
        }
    }
    _p1_temp_files.count = 0;
}

#if _P1_HAS_POSIX
extern int mkstemp(char *);
extern ssize_t write(int, const void *, size_t);
#endif

static inline const char *p1_temp_file_create_binary(const char *prefix, const void *data, size_t size) {
    if (_p1_temp_files.count >= _P1_MAX_TEMP_FILES) {
        return NULL;
    }
    size_t idx = _p1_temp_files.count;
    char *path = _p1_temp_files.paths[idx];
    const char *pfx = (prefix != NULL && prefix[0] != '\0') ? prefix : "p1_test";

#if _P1_HAS_POSIX
    snprintf(path, 512, "/tmp/%s_XXXXXX", pfx);
    int fd = mkstemp(path);
    if (fd < 0) {
        path[0] = '\0';
        return NULL;
    }
    if (data != NULL && size > 0) {
        ssize_t written = write(fd, data, size);
        (void)written;
    }
    close(fd);
#else
    snprintf(path, 512, "tmp_%s_%u_%zu.dat", pfx, (unsigned int)rand(), idx);
    FILE *fp = fopen(path, "wb");
    if (!fp) {
        path[0] = '\0';
        return NULL;
    }
    if (data != NULL && size > 0) {
        fwrite(data, 1, size, fp);
    }
    fclose(fp);
#endif

    _p1_temp_files.count++;
    p1_register_cleanup_hook(p1_temp_files_cleanup);
    return path;
}

static inline const char *p1_temp_file_create(const char *initial_content) {
    size_t len = (initial_content != NULL) ? strlen(initial_content) : 0;
    return p1_temp_file_create_binary("p1_temp", initial_content, len);
}

static inline const char *p1_temp_file_create_named(const char *prefix, const char *initial_content) {
    size_t len = (initial_content != NULL) ? strlen(initial_content) : 0;
    return p1_temp_file_create_binary(prefix, initial_content, len);
}

/* --- Funciones auxiliares internas para archivos ------------------------- */

static inline void _p1_fail_file_exists(const char *file, int line, const char *expr,
                                        const char *path, int should_exist, const char *fmt, ...) {
    _p1_print_fail_header(file, line, expr);
    if (!_p1_global_state.quiet_mode) {
        if (should_exist) {
            fprintf(stderr, "    el archivo '%s' no existe o no se pudo abrir para lectura\n", path);
        } else {
            fprintf(stderr, "    el archivo '%s' existe (se esperaba que no existiera)\n", path);
        }
    }
    if (fmt != NULL) {
        va_list args;
        va_start(args, fmt);
        _p1_print_user_msg(fmt, args);
        va_end(args);
    }
    _p1_trigger_failure();
}

static inline void _p1_fail_file_line(const char *file, int line, const char *expr,
                                      size_t line_num, const char *exp_line, const char *act_line,
                                      const char *fmt, ...) {
    _p1_print_fail_header(file, line, expr);
    if (!_p1_global_state.quiet_mode) {
        fprintf(stderr, "    discrepancia en línea %zu del archivo:\n", line_num);
        fprintf(stderr, "      esperado: \"%s\"\n", exp_line ? exp_line : "<EOF>");
        fprintf(stderr, "      obtenido: \"%s\"\n", act_line ? act_line : "<EOF>");
    }
    if (fmt != NULL) {
        va_list args;
        va_start(args, fmt);
        _p1_print_user_msg(fmt, args);
        va_end(args);
    }
    _p1_trigger_failure();
}

static inline void _p1_fail_file_contains(const char *file, int line, const char *expr,
                                          const char *path, const char *needle,
                                          const char *fmt, ...) {
    _p1_print_fail_header(file, line, expr);
    if (!_p1_global_state.quiet_mode) {
        fprintf(stderr, "    el texto \"%s\" no fue hallado en el archivo '%s'\n", needle, path);
    }
    if (fmt != NULL) {
        va_list args;
        va_start(args, fmt);
        _p1_print_user_msg(fmt, args);
        va_end(args);
    }
    _p1_trigger_failure();
}

static inline void _p1_fail_file_bin(const char *file, int line, const char *expr,
                                     size_t offset, int exp_byte, int act_byte,
                                     const char *fmt, ...) {
    _p1_print_fail_header(file, line, expr);
    if (!_p1_global_state.quiet_mode) {
        fprintf(stderr, "    discrepancia binaria en offset 0x%zx (%zu bytes):\n", offset, offset);
        if (exp_byte < 0) {
            fprintf(stderr, "      esperado: <Fin de Archivo>\n");
        } else {
            fprintf(stderr, "      esperado: 0x%02X\n", (unsigned char)exp_byte);
        }
        if (act_byte < 0) {
            fprintf(stderr, "      obtenido: <Fin de Archivo>\n");
        } else {
            fprintf(stderr, "      obtenido: 0x%02X\n", (unsigned char)act_byte);
        }
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
/* --- MACROS DE ASERCIÓN SOBRE ARCHIVOS ----------------------------------- */
/* ========================================================================= */

/* --- Existencia de Archivos --- */

#define ASSERT_FILE_EXISTS_MSG(filepath, ...) do { \
    _p1_global_state.asserts_total++; \
    const char *_fpath = (const char *)(filepath); \
    FILE *_fp = (_fpath != NULL) ? fopen(_fpath, "rb") : NULL; \
    if (_fp != NULL) { \
        fclose(_fp); \
    } else { \
        _p1_fail_file_exists(__FILE__, __LINE__, \
                             "ASSERT_FILE_EXISTS(" #filepath ")", \
                             _fpath, 1, __VA_ARGS__); \
    } \
} while (0)

#define ASSERT_FILE_EXISTS(filepath) ASSERT_FILE_EXISTS_MSG(filepath, NULL)

#define ASSERT_FILE_NOT_EXISTS_MSG(filepath, ...) do { \
    _p1_global_state.asserts_total++; \
    const char *_fpath = (const char *)(filepath); \
    FILE *_fp = (_fpath != NULL) ? fopen(_fpath, "rb") : NULL; \
    if (_fp != NULL) { \
        fclose(_fp); \
        _p1_fail_file_exists(__FILE__, __LINE__, \
                             "ASSERT_FILE_NOT_EXISTS(" #filepath ")", \
                             _fpath, 0, __VA_ARGS__); \
    } \
} while (0)

#define ASSERT_FILE_NOT_EXISTS(filepath) ASSERT_FILE_NOT_EXISTS_MSG(filepath, NULL)

/* --- Comparación de Archivos de Texto Línea por Línea --- */

#define ASSERT_FILE_EQ_MSG(expected_path, actual_path, ...) do { \
    _p1_global_state.asserts_total++; \
    const char *_exp_p = (const char *)(expected_path); \
    const char *_act_p = (const char *)(actual_path); \
    FILE *_f_exp = (_exp_p != NULL) ? fopen(_exp_p, "r") : NULL; \
    FILE *_f_act = (_act_p != NULL) ? fopen(_act_p, "r") : NULL; \
    if (_f_exp == NULL) { \
        if (_f_act) fclose(_f_act); \
        _p1_fail_file_exists(__FILE__, __LINE__, "ASSERT_FILE_EQ: apertura de esperado", _exp_p, 1, __VA_ARGS__); \
    } \
    if (_f_act == NULL) { \
        fclose(_f_exp); \
        _p1_fail_file_exists(__FILE__, __LINE__, "ASSERT_FILE_EQ: apertura de obtenido", _act_p, 1, __VA_ARGS__); \
    } \
    char _l_exp[1024]; \
    char _l_act[1024]; \
    size_t _lnum = 1; \
    int _diff_found = 0; \
    while (1) { \
        char *_r_exp = fgets(_l_exp, sizeof(_l_exp), _f_exp); \
        char *_r_act = fgets(_l_act, sizeof(_l_act), _f_act); \
        if (_r_exp == NULL && _r_act == NULL) { \
            break; \
        } \
        if (_r_exp == NULL || _r_act == NULL || strcmp(_l_exp, _l_act) != 0) { \
            _diff_found = 1; \
            fclose(_f_exp); \
            fclose(_f_act); \
            _p1_fail_file_line(__FILE__, __LINE__, \
                              "ASSERT_FILE_EQ(" #expected_path ", " #actual_path ")", \
                              _lnum, _r_exp ? _l_exp : NULL, _r_act ? _l_act : NULL, __VA_ARGS__); \
            break; \
        } \
        _lnum++; \
    } \
    if (!_diff_found) { \
        fclose(_f_exp); \
        fclose(_f_act); \
    } \
} while (0)

#define ASSERT_FILE_EQ(expected_path, actual_path) \
    ASSERT_FILE_EQ_MSG(expected_path, actual_path, NULL)

/* --- Búsqueda de Subcadena en Archivo de Texto --- */

#define ASSERT_FILE_CONTAINS_MSG(filepath, needle, ...) do { \
    _p1_global_state.asserts_total++; \
    const char *_fpath = (const char *)(filepath); \
    const char *_nee = (const char *)(needle); \
    FILE *_fp = (_fpath != NULL) ? fopen(_fpath, "r") : NULL; \
    if (_fp == NULL) { \
        _p1_fail_file_exists(__FILE__, __LINE__, "ASSERT_FILE_CONTAINS: apertura", _fpath, 1, __VA_ARGS__); \
    } else { \
        char _line_buf[1024]; \
        int _found = 0; \
        while (fgets(_line_buf, sizeof(_line_buf), _fp) != NULL) { \
            if (strstr(_line_buf, _nee) != NULL) { \
                _found = 1; \
                break; \
            } \
        } \
        fclose(_fp); \
        if (!_found) { \
            _p1_fail_file_contains(__FILE__, __LINE__, \
                                  "ASSERT_FILE_CONTAINS(" #filepath ", " #needle ")", \
                                  _fpath, _nee, __VA_ARGS__); \
        } \
    } \
} while (0)

#define ASSERT_FILE_CONTAINS(filepath, needle) \
    ASSERT_FILE_CONTAINS_MSG(filepath, needle, NULL)

/* --- Comparación Binaria Byte a Byte --- */

#define ASSERT_FILE_BINARY_EQ_MSG(expected_path, actual_path, ...) do { \
    _p1_global_state.asserts_total++; \
    const char *_exp_p = (const char *)(expected_path); \
    const char *_act_p = (const char *)(actual_path); \
    FILE *_f_exp = (_exp_p != NULL) ? fopen(_exp_p, "rb") : NULL; \
    FILE *_f_act = (_act_p != NULL) ? fopen(_act_p, "rb") : NULL; \
    if (_f_exp == NULL) { \
        if (_f_act) fclose(_f_act); \
        _p1_fail_file_exists(__FILE__, __LINE__, "ASSERT_FILE_BINARY_EQ: apertura esperado", _exp_p, 1, __VA_ARGS__); \
    } \
    if (_f_act == NULL) { \
        fclose(_f_exp); \
        _p1_fail_file_exists(__FILE__, __LINE__, "ASSERT_FILE_BINARY_EQ: apertura obtenido", _act_p, 1, __VA_ARGS__); \
    } \
    size_t _offset = 0; \
    int _diff_found = 0; \
    while (1) { \
        int _b_exp = fgetc(_f_exp); \
        int _b_act = fgetc(_f_act); \
        if (_b_exp == EOF && _b_act == EOF) { \
            break; \
        } \
        if (_b_exp != _b_act) { \
            _diff_found = 1; \
            fclose(_f_exp); \
            fclose(_f_act); \
            _p1_fail_file_bin(__FILE__, __LINE__, \
                             "ASSERT_FILE_BINARY_EQ(" #expected_path ", " #actual_path ")", \
                             _offset, _b_exp, _b_act, __VA_ARGS__); \
            break; \
        } \
        _offset++; \
    } \
    if (!_diff_found) { \
        fclose(_f_exp); \
        fclose(_f_act); \
    } \
} while (0)

#define ASSERT_FILE_BINARY_EQ(expected_path, actual_path) \
    ASSERT_FILE_BINARY_EQ_MSG(expected_path, actual_path, NULL)

/* --- Aserciones sobre Archivos con Pistas Pedagógicas (QoL 4) ------------ */

#define ASSERT_FILE_EXISTS_HINT(filepath, hint) do { \
    _p1_global_state.current_hint = (hint); \
    ASSERT_FILE_EXISTS(filepath); \
    _p1_global_state.current_hint = NULL; \
} while (0)

#define ASSERT_FILE_NOT_EXISTS_HINT(filepath, hint) do { \
    _p1_global_state.current_hint = (hint); \
    ASSERT_FILE_NOT_EXISTS(filepath); \
    _p1_global_state.current_hint = NULL; \
} while (0)

#define ASSERT_FILE_EQ_HINT(expected_path, actual_path, hint) do { \
    _p1_global_state.current_hint = (hint); \
    ASSERT_FILE_EQ(expected_path, actual_path); \
    _p1_global_state.current_hint = NULL; \
} while (0)

#define ASSERT_FILE_CONTAINS_HINT(filepath, needle, hint) do { \
    _p1_global_state.current_hint = (hint); \
    ASSERT_FILE_CONTAINS(filepath, needle); \
    _p1_global_state.current_hint = NULL; \
} while (0)

#define ASSERT_FILE_BINARY_EQ_HINT(expected_path, actual_path, hint) do { \
    _p1_global_state.current_hint = (hint); \
    ASSERT_FILE_BINARY_EQ(expected_path, actual_path); \
    _p1_global_state.current_hint = NULL; \
} while (0)

#ifdef __cplusplus
}
#endif

#endif /* P1_FILES_H */
