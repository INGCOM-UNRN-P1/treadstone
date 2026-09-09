/*
 * tda/contracts.h — Framework de invariantes en runtime para TDAs (C11).
 * ============================================================================
 * c-tda-contracts: contratos de precondición/postcondición/invariante con
 * reporte pedagógico, y helpers genéricos para validar estructuras enlazadas
 * (detección de ciclos) sin conocer el layout más allá del offset del campo
 * `siguiente`.
 *
 * Uso típico dentro de una primitiva del TDA:
 *
 *     Lista *lista_insertar(Lista *l, int valor) {
 *         TDA_REQUIERE(l != NULL);
 *         TDA_INVARIANTE(l, lista_es_valida);
 *         ... implementación ...
 *         TDA_GARANTIZA(l->tam > 0);
 *         TDA_INVARIANTE(l, lista_es_valida);
 *         return l;
 *     }
 *
 * Compilación:
 *   - Normal: los contratos se evalúan; una violación imprime archivo:línea y
 *     la condición rota, y aborta (comportamiento deseado en las pruebas).
 *   - Con -DNDEBUG: los contratos se compilan fuera (cero costo de runtime).
 *
 * Encapsulamiento: el struct del TDA debe declararse COMPLETO sólo en el .c.
 * El header público declara `typedef struct lista Lista;` (tipo incompleto):
 * si el código cliente intenta `l->tam`, obtiene un error de compilación
 * "dereference of incomplete type" — el encapsulamiento se defiende en tiempo
 * de compilación, no en runtime.
 */

#ifndef TDA_CONTRACTS_H
#define TDA_CONTRACTS_H

#include <stddef.h>

/* ---------------------------------------------------------------------------
 * Contador pedagógico por unidad de traducción: cuántos contratos corrieron.
 * Útil para que la suite verifique que los contratos SE evaluaron.
 * ------------------------------------------------------------------------- */
extern size_t tda_contratos_evaluados_;
size_t tda_contratos_evaluados_;

#ifdef NDEBUG
/* Versión producción: sin evaluación, sin costo. */
#define TDA_REQUIERE(cond)                 ((void)0)
#define TDA_GARANTIZA(cond)                ((void)0)
#define TDA_INVARIANTE(estructura, validador) ((void)0)
#define tda_contratos_evaluados()          ((size_t)0)

#else /* modo verificación */
#include <stdio.h>
#include <stdlib.h>

#define TDA_REQUIERE(cond)                                                    \
    do {                                                                      \
        if (!(cond)) {                                                        \
            fprintf(stderr,                                                   \
                    "❌ PRECONDICIÓN rota  %s:%d  TDA_REQUIERE(%s)\n",        \
                    __FILE__, __LINE__, #cond);                               \
            abort();                                                          \
        }                                                                     \
    } while (0)

#define TDA_GARANTIZA(cond)                                                   \
    do {                                                                      \
        if (!(cond)) {                                                        \
            fprintf(stderr,                                                   \
                    "❌ POSTCONDICIÓN rota  %s:%d  TDA_GARANTIZA(%s)\n",      \
                    __FILE__, __LINE__, #cond);                               \
            abort();                                                          \
        }                                                                     \
    } while (0)

#define TDA_INVARIANTE(estructura, validador)                                 \
    do {                                                                      \
        if (!(validador)(estructura)) {                                       \
            fprintf(stderr,                                                   \
                    "❌ INVARIANTE roto  %s:%d  %s(estructura)\n",            \
                    __FILE__, __LINE__, #validador);                          \
            abort();                                                          \
        }                                                                     \
        tda_contratos_evaluados_++;                                           \
    } while (0)

#define tda_contratos_evaluados() (tda_contratos_evaluados_)

#endif /* NDEBUG */

/* ---------------------------------------------------------------------------
 * Helpers genéricos para invariantes estructurales sobre nodos enlazados.
 * No requieren conocer el tipo: basta el offset del campo `siguiente`.
 * ------------------------------------------------------------------------- */

/* Devuelve el nodo siguiente siguiendo el puntero ubicado en `offset_sig`. */
static inline void *tda_siguiente(const void *nodo, size_t offset_sig)
{
    if (nodo == NULL) {
        return NULL;
    }
    void *sig;
    __builtin_memcpy(&sig, (const char *)nodo + offset_sig, sizeof sig);
    return sig;
}

/*
 * Recorre acotadamente la cadena empezando en `primero` y reporta si existe
 * un ciclo (un nodo revisitado). `max_nodos` acota el peor caso para no
 * colgar la batería de pruebas ante corrupción grave.
 *
 * Devuelve: 0 = sin ciclo, 1 = ciclo detectado.
 */
static inline int tda_tiene_ciclo(const void *primero, size_t offset_sig,
                                  size_t max_nodos)
{
    const void *vistos[256];
    size_t n_vistos = 0;
    const void *actual = primero;

    while (actual != NULL && n_vistos < max_nodos) {
        for (size_t i = 0; i < n_vistos; i++) {
            if (vistos[i] == actual) {
                return 1; /* nodo revisitado: ciclo */
            }
        }
        if (n_vistos < sizeof vistos / sizeof vistos[0]) {
            vistos[n_vistos++] = actual;
        }
        actual = tda_siguiente(actual, offset_sig);
    }
    return 0;
}

#endif /* TDA_CONTRACTS_H */
