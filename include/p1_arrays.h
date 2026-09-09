/**
 * @file p1_arrays.h
 * @brief Aserciones avanzadas para arreglos en C11 para el framework p1_test.
 * @version 1.0.0
 * 
 * Proporciona comprobaciones tipadas sobre arreglos enteros, reales y de cadenas,
 * reportando el índice exacto de la primera discrepancia.
 */

#ifndef P1_ARRAYS_H
#define P1_ARRAYS_H

#include "p1_test.h"

#ifdef __cplusplus
extern "C" {
#endif

/* --- Funciones auxiliares internas para arreglos ------------------------- */

static inline void _p1_fail_array_null(const char *file, int line, const char *expr,
                                       const char *arg_name, const char *fmt, ...) {
    _p1_print_fail_header(file, line, expr);
    if (!_p1_global_state.quiet_mode) {
        fprintf(stderr, "    puntero nulo inesperado en argumento '%s' con longitud > 0\n", arg_name);
    }
    if (fmt != NULL) {
        va_list args;
        va_start(args, fmt);
        _p1_print_user_msg(fmt, args);
        va_end(args);
    }
    _p1_trigger_failure();
}

static inline void _p1_fail_array_double(const char *file, int line, const char *expr,
                                         size_t index, double expected, double actual,
                                         double diff, double eps, const char *fmt, ...) {
    _p1_print_fail_header(file, line, expr);
    if (!_p1_global_state.quiet_mode) {
        fprintf(stderr, "    discrepancia en índice [%zu]:\n", index);
        fprintf(stderr, "      esperado: %.7g (tolerancia +/- %.7g)\n", expected, eps);
        fprintf(stderr, "      obtenido: %.7g (diferencia: %.7g)\n", actual, diff);
    }
    if (fmt != NULL) {
        va_list args;
        va_start(args, fmt);
        _p1_print_user_msg(fmt, args);
        va_end(args);
    }
    _p1_trigger_failure();
}

static inline void _p1_fail_array_sorted(const char *file, int line, const char *expr,
                                         size_t index, long long val_i, long long val_next,
                                         int is_asc, const char *fmt, ...) {
    _p1_print_fail_header(file, line, expr);
    if (!_p1_global_state.quiet_mode) {
        fprintf(stderr, "    violación de orden %s en índice [%zu]:\n",
                is_asc ? "ascendente" : "descendente", index);
        fprintf(stderr, "      elemento actual [%zu] = %lld\n", index, val_i);
        fprintf(stderr, "      siguiente valor [%zu] = %lld\n", index + 1, val_next);
    }
    if (fmt != NULL) {
        va_list args;
        va_start(args, fmt);
        _p1_print_user_msg(fmt, args);
        va_end(args);
    }
    _p1_trigger_failure();
}

static inline void _p1_fail_str_array(const char *file, int line, const char *expr,
                                      size_t index, const char *expected, const char *actual,
                                      const char *fmt, ...) {
    _p1_print_fail_header(file, line, expr);
    if (!_p1_global_state.quiet_mode) {
        fprintf(stderr, "    discrepancia en elemento [%zu]:\n", index);
        fprintf(stderr, "      esperado: %s%s%s\n",
                expected ? "\"" : "", expected ? expected : "NULL", expected ? "\"" : "");
        fprintf(stderr, "      obtenido: %s%s%s\n",
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

static inline void _p1_fail_array_contains(const char *file, int line, const char *expr,
                                           long long target, int should_contain,
                                           const char *fmt, ...) {
    _p1_print_fail_header(file, line, expr);
    if (!_p1_global_state.quiet_mode) {
        if (should_contain) {
            fprintf(stderr, "    el valor %lld no fue encontrado en el arreglo\n", target);
        } else {
            fprintf(stderr, "    el valor %lld fue encontrado en el arreglo (se esperaba ausencia)\n", target);
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
/* --- MACROS DE ASERCIÓN SOBRE ARREGLOS ----------------------------------- */
/* ========================================================================= */

#ifndef ASSERT_ARRAY_INT_EQ
#define ASSERT_ARRAY_INT_EQ_MSG(expected, actual, length, ...) do { \
    size_t _len = (size_t)(length); \
    const int *_aexp = (const int *)(expected); \
    const int *_aact = (const int *)(actual); \
    if (_len == 0) { \
        _p1_global_state.asserts_total++; \
    } else if (_aexp == NULL || _aact == NULL) { \
        _p1_global_state.asserts_total++; \
        _p1_fail_array_null(__FILE__, __LINE__, \
                            "ASSERT_ARRAY_INT_EQ(" #expected ", " #actual ", " #length ")", \
                            _aexp == NULL ? #expected : #actual, __VA_ARGS__); \
    } else { \
        for (size_t _i = 0; _i < _len; _i++) { \
            _p1_global_state.asserts_total++; \
            if (_aexp[_i] != _aact[_i]) { \
                _p1_fail_array_int(__FILE__, __LINE__, \
                                   "ASSERT_ARRAY_INT_EQ(" #expected ", " #actual ", " #length ")", \
                                   _i, (long long)_aexp[_i], (long long)_aact[_i], __VA_ARGS__); \
                break; \
            } \
        } \
    } \
} while (0)

#define ASSERT_ARRAY_INT_EQ(expected, actual, length) \
    ASSERT_ARRAY_INT_EQ_MSG(expected, actual, length, NULL)
#endif

/* --- Arreglos de Enteros: Orden Ascendente --- */

#define ASSERT_ARRAY_INT_SORTED_ASC_MSG(arr, length, ...) do { \
    size_t _len = (size_t)(length); \
    const int *_a = (const int *)(arr); \
    if (_len <= 1) { \
        _p1_global_state.asserts_total++; \
    } else if (_a == NULL) { \
        _p1_global_state.asserts_total++; \
        _p1_fail_array_null(__FILE__, __LINE__, \
                            "ASSERT_ARRAY_INT_SORTED_ASC(" #arr ", " #length ")", \
                            #arr, __VA_ARGS__); \
    } else { \
        for (size_t _i = 0; _i < _len - 1; _i++) { \
            _p1_global_state.asserts_total++; \
            if (_a[_i] > _a[_i + 1]) { \
                _p1_fail_array_sorted(__FILE__, __LINE__, \
                                      "ASSERT_ARRAY_INT_SORTED_ASC(" #arr ", " #length ")", \
                                      _i, (long long)_a[_i], (long long)_a[_i + 1], 1, __VA_ARGS__); \
                break; \
            } \
        } \
    } \
} while (0)

#define ASSERT_ARRAY_INT_SORTED_ASC(arr, length) \
    ASSERT_ARRAY_INT_SORTED_ASC_MSG(arr, length, NULL)

/* --- Arreglos de Enteros: Orden Descendente --- */

#define ASSERT_ARRAY_INT_SORTED_DESC_MSG(arr, length, ...) do { \
    size_t _len = (size_t)(length); \
    const int *_a = (const int *)(arr); \
    if (_len <= 1) { \
        _p1_global_state.asserts_total++; \
    } else if (_a == NULL) { \
        _p1_global_state.asserts_total++; \
        _p1_fail_array_null(__FILE__, __LINE__, \
                            "ASSERT_ARRAY_INT_SORTED_DESC(" #arr ", " #length ")", \
                            #arr, __VA_ARGS__); \
    } else { \
        for (size_t _i = 0; _i < _len - 1; _i++) { \
            _p1_global_state.asserts_total++; \
            if (_a[_i] < _a[_i + 1]) { \
                _p1_fail_array_sorted(__FILE__, __LINE__, \
                                      "ASSERT_ARRAY_INT_SORTED_DESC(" #arr ", " #length ")", \
                                      _i, (long long)_a[_i], (long long)_a[_i + 1], 0, __VA_ARGS__); \
                break; \
            } \
        } \
    } \
} while (0)

#define ASSERT_ARRAY_INT_SORTED_DESC(arr, length) \
    ASSERT_ARRAY_INT_SORTED_DESC_MSG(arr, length, NULL)

/* --- Arreglos de Dobles con Tolerancia --- */

#define ASSERT_ARRAY_DOUBLE_EQ_MSG(expected, actual, length, epsilon, ...) do { \
    size_t _len = (size_t)(length); \
    const double *_dexp = (const double *)(expected); \
    const double *_dact = (const double *)(actual); \
    double _eps = (double)(epsilon); \
    if (_len == 0) { \
        _p1_global_state.asserts_total++; \
    } else if (_dexp == NULL || _dact == NULL) { \
        _p1_global_state.asserts_total++; \
        _p1_fail_array_null(__FILE__, __LINE__, \
                            "ASSERT_ARRAY_DOUBLE_EQ(" #expected ", " #actual ", " #length ", " #epsilon ")", \
                            _dexp == NULL ? #expected : #actual, __VA_ARGS__); \
    } else { \
        for (size_t _i = 0; _i < _len; _i++) { \
            _p1_global_state.asserts_total++; \
            double _diff = _p1_abs_double(_dexp[_i] - _dact[_i]); \
            if (_diff > _eps) { \
                _p1_fail_array_double(__FILE__, __LINE__, \
                                      "ASSERT_ARRAY_DOUBLE_EQ(" #expected ", " #actual ", " #length ", " #epsilon ")", \
                                      _i, _dexp[_i], _dact[_i], _diff, _eps, __VA_ARGS__); \
                break; \
            } \
        } \
    } \
} while (0)

#define ASSERT_ARRAY_DOUBLE_EQ(expected, actual, length, epsilon) \
    ASSERT_ARRAY_DOUBLE_EQ_MSG(expected, actual, length, epsilon, NULL)

/* --- Arreglos de Cadenas Terminados en NULL (argv-style) --- */

#define ASSERT_STR_ARRAY_EQ_MSG(expected, actual, ...) do { \
    const char * const *_sa_exp = (const char * const *)(expected); \
    const char * const *_sa_act = (const char * const *)(actual); \
    size_t _i = 0; \
    while (1) { \
        _p1_global_state.asserts_total++; \
        const char *_e = (_sa_exp != NULL) ? _sa_exp[_i] : NULL; \
        const char *_a = (_sa_act != NULL) ? _sa_act[_i] : NULL; \
        if (_e == NULL && _a == NULL) { \
            break; \
        } \
        if (_e == NULL || _a == NULL || strcmp(_e, _a) != 0) { \
            _p1_fail_str_array(__FILE__, __LINE__, \
                               "ASSERT_STR_ARRAY_EQ(" #expected ", " #actual ")", \
                               _i, _e, _a, __VA_ARGS__); \
            break; \
        } \
        _i++; \
    } \
} while (0)

#define ASSERT_STR_ARRAY_EQ(expected, actual) \
    ASSERT_STR_ARRAY_EQ_MSG(expected, actual, NULL)

/* --- Pertenencia en Arreglos de Enteros --- */

#define ASSERT_ARRAY_INT_CONTAINS_MSG(arr, length, target, ...) do { \
    _p1_global_state.asserts_total++; \
    size_t _len = (size_t)(length); \
    const int *_a = (const int *)(arr); \
    int _tgt = (int)(target); \
    int _found = 0; \
    if (_a != NULL) { \
        for (size_t _i = 0; _i < _len; _i++) { \
            if (_a[_i] == _tgt) { \
                _found = 1; \
                break; \
            } \
        } \
    } \
    if (!_found) { \
        _p1_fail_array_contains(__FILE__, __LINE__, \
                                "ASSERT_ARRAY_INT_CONTAINS(" #arr ", " #length ", " #target ")", \
                                (long long)_tgt, 1, __VA_ARGS__); \
    } \
} while (0)

#define ASSERT_ARRAY_INT_CONTAINS(arr, length, target) \
    ASSERT_ARRAY_INT_CONTAINS_MSG(arr, length, target, NULL)

#define ASSERT_ARRAY_INT_NOT_CONTAINS_MSG(arr, length, target, ...) do { \
    _p1_global_state.asserts_total++; \
    size_t _len = (size_t)(length); \
    const int *_a = (const int *)(arr); \
    int _tgt = (int)(target); \
    int _found = 0; \
    if (_a != NULL) { \
        for (size_t _i = 0; _i < _len; _i++) { \
            if (_a[_i] == _tgt) { \
                _found = 1; \
                break; \
            } \
        } \
    } \
    if (_found) { \
        _p1_fail_array_contains(__FILE__, __LINE__, \
                                "ASSERT_ARRAY_INT_NOT_CONTAINS(" #arr ", " #length ", " #target ")", \
                                (long long)_tgt, 0, __VA_ARGS__); \
    } \
} while (0)

#define ASSERT_ARRAY_INT_NOT_CONTAINS(arr, length, target) \
    ASSERT_ARRAY_INT_NOT_CONTAINS_MSG(arr, length, target, NULL)

#ifdef __cplusplus
}
#endif

#endif /* P1_ARRAYS_H */
