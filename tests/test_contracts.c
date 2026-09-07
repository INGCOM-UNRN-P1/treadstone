/*
 * Pruebas del framework tda/contracts.h.
 * Compila con -Wall -Wextra -Werror -pedantic -std=c99: el propio test es la
 * prueba de que los contratos son C99 estricto.
 */
#include <assert.h>
#include <stdio.h>
#include <stddef.h>

#include "tda/contracts.h"

/* --- TDA de juguete: lista enlazada simple ------------------------------- */
typedef struct nodo {
    int valor;
    struct nodo *sig;
} Nodo;

/* Invariante estructural de la lista: sin ciclos y acotada. */
static int lista_es_valida(const Nodo *primero)
{
    return !tda_tiene_ciclo(primero, offsetof(Nodo, sig), 1000);
}

static void probar_contratos_se_evaluan(void)
{
    const size_t antes = tda_contratos_evaluados();
    TDA_REQUIERE(1 == 1);
    TDA_GARANTIZA(2 > 1);
    /* el invariante corre sobre una lista vacía (NULL es válida) */
    const Nodo *lista_vacia = NULL;
    TDA_INVARIANTE(lista_vacia, lista_es_valida);
    assert(tda_contratos_evaluados() > antes);
    printf("test_contratos_se_evaluan: PASSED\n");
}

static void probar_lista_sin_ciclos(void)
{
    Nodo c = {30, NULL};
    Nodo b = {20, &c};
    Nodo a = {10, &b};
    assert(lista_es_valida(&a));
    assert(!tda_tiene_ciclo(&a, offsetof(Nodo, sig), 100));
    /* recorrido completo con tda_siguiente */
    assert(((Nodo *)tda_siguiente(&a, offsetof(Nodo, sig)))->valor == 20);
    assert(tda_siguiente(&c, offsetof(Nodo, sig)) == NULL);
    printf("test_lista_sin_ciclos: PASSED\n");
}

static void probar_deteccion_de_ciclo(void)
{
    /* lista con ciclo: a -> b -> c -> b */
    Nodo c = {30, NULL};
    Nodo b = {20, &c};
    Nodo a = {10, &b};
    c.sig = &b;
    assert(tda_tiene_ciclo(&a, offsetof(Nodo, sig), 100));
    printf("test_deteccion_de_ciclo: PASSED\n");
}

static void probar_nodos_null(void)
{
    assert(tda_siguiente(NULL, 0) == NULL);
    assert(!tda_tiene_ciclo(NULL, offsetof(Nodo, sig), 10));
    printf("test_nodos_null: PASSED\n");
}

int main(void)
{
    printf("Pruebas de tda/contracts...\n");
    probar_contratos_se_evaluan();
    probar_lista_sin_ciclos();
    probar_deteccion_de_ciclo();
    probar_nodos_null();
    printf("Todas las pruebas de contracts pasaron.\n");
    return 0;
}
