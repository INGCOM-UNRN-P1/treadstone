#ifndef OPERACIONES_H
#define OPERACIONES_H

#include <stddef.h>

/**
 * @brief Determina si un número entero es par.
 * @param n Número a evaluar.
 * @return 1 si es par, 0 en caso contrario.
 */
int es_par(int n);

/**
 * @brief Calcula el factorial de un entero no negativo.
 * @param n Entero a calcular (0 <= n <= 12).
 * @return Factorial de n, o -1 si n < 0.
 */
int factorial(int n);

/**
 * @brief Calcula la media aritmética de un arreglo de números reales.
 * @param valores Arreglo de entrada.
 * @param n Cantidad de elementos.
 * @param resultado Puntero donde se almacenará el promedio.
 * @return 0 en caso de éxito, -1 si los argumentos son inválidos.
 */
int calcular_promedio(const double valores[], size_t n, double *resultado);

/**
 * @brief Construye un mensaje de saludo para el nombre provisto.
 * @param nombre Nombre del destinatario.
 * @param buffer Búfer de destino.
 * @param max_tam Tamaño máximo del búfer en bytes.
 * @return Puntero al búfer de destino, o NULL ante argumentos inválidos.
 */
char *obtener_saludo(const char *nombre, char *buffer, size_t max_tam);

/**
 * @brief Busca la primera aparición de un valor en un arreglo de enteros.
 * @param arreglo Arreglo a recorrer.
 * @param n Cantidad de elementos.
 * @param buscado Valor a localizar.
 * @return Puntero al elemento hallado dentro del arreglo, o NULL si no existe.
 */
const int *buscar_elemento(const int arreglo[], size_t n, int buscado);

#endif /* OPERACIONES_H */
