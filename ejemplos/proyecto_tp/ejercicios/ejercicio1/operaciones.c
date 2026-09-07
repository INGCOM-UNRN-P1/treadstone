#include "operaciones.h"
#include <stdio.h>
#include <string.h>

int es_par(int n) {
    return (n % 2) == 0;
}

int factorial(int n) {
    if (n < 0) {
        return -1;
    }
    int res = 1;
    for (int i = 2; i <= n; i++) {
        res *= i;
    }
    return res;
}

int calcular_promedio(const double valores[], size_t n, double *resultado) {
    if (valores == NULL || resultado == NULL || n == 0) {
        return -1;
    }
    double suma = 0.0;
    for (size_t i = 0; i < n; i++) {
        suma += valores[i];
    }
    *resultado = suma / (double)n;
    return 0;
}

char *obtener_saludo(const char *nombre, char *buffer, size_t max_tam) {
    if (nombre == NULL || buffer == NULL || max_tam == 0) {
        return NULL;
    }
    int esc = snprintf(buffer, max_tam, "Hola, %s!", nombre);
    if (esc < 0 || (size_t)esc >= max_tam) {
        return NULL;
    }
    return buffer;
}

const int *buscar_elemento(const int arreglo[], size_t n, int buscado) {
    if (arreglo == NULL) {
        return NULL;
    }
    for (size_t i = 0; i < n; i++) {
        if (arreglo[i] == buscado) {
            return &arreglo[i];
        }
    }
    return NULL;
}
