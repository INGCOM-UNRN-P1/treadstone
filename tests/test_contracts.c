/*
 * Pruebas del framework tda/contracts.h utilizando p1_test.h.
 * Compila con -Wall -Wextra -Werror -pedantic -std=c99.
 */
#include <stddef.h>

#include "p1_test.h"
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

TEST(probar_contratos_se_evaluan)
{
    const size_t antes = tda_contratos_evaluados();
    TDA_REQUIERE(1 == 1);
    TDA_GARANTIZA(2 > 1);
    /* el invariante corre sobre una lista vacía (NULL es válida) */
    const Nodo *lista_vacia = NULL;
    TDA_INVARIANTE(lista_vacia, lista_es_valida);
    ASSERT_TRUE(tda_contratos_evaluados() > antes);
}

TEST(probar_lista_sin_ciclos)
{
    Nodo c = {30, NULL};
    Nodo b = {20, &c};
    Nodo a = {10, &b};
    ASSERT_TRUE(lista_es_valida(&a));
    ASSERT_FALSE(tda_tiene_ciclo(&a, offsetof(Nodo, sig), 100));
    /* recorrido completo con tda_siguiente */
    const Nodo *segundo = (const Nodo *)tda_siguiente(&a, offsetof(Nodo, sig));
    ASSERT_PTR_NOT_NULL(segundo);
    ASSERT_INT_EQ(20, segundo->valor);
    ASSERT_PTR_NULL(tda_siguiente(&c, offsetof(Nodo, sig)));
}

TEST(probar_deteccion_de_ciclo)
{
    /* lista con ciclo: a -> b -> c -> b */
    Nodo c = {30, NULL};
    Nodo b = {20, &c};
    Nodo a = {10, &b};
    c.sig = &b;
    ASSERT_TRUE(tda_tiene_ciclo(&a, offsetof(Nodo, sig), 100));
}

TEST(probar_nodos_null)
{
    ASSERT_PTR_NULL(tda_siguiente(NULL, 0));
    ASSERT_FALSE(tda_tiene_ciclo(NULL, offsetof(Nodo, sig), 10));
}

int main(int argc, char **argv)
{
    TEST_SUITE_BEGIN_ARGS("Suite de Contratos TDA", argc, argv);

    RUN_TEST(probar_contratos_se_evaluan);
    RUN_TEST(probar_lista_sin_ciclos);
    RUN_TEST(probar_deteccion_de_ciclo);
    RUN_TEST(probar_nodos_null);

    return TEST_REPORT();
}
