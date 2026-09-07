#include <stdio.h>
#include "operaciones.h"

int main(void) {
    printf("=== Demostración del Ejercicio 1 ===\n");

    int num = 4;
    printf("¿El número %d es par? %s\n", num, es_par(num) ? "Sí" : "No");

    int fact = factorial(5);
    printf("Factorial de 5: %d\n", fact);

    double datos[] = {10.0, 20.0, 30.0};
    double prom = 0.0;
    if (calcular_promedio(datos, 3, &prom) == 0) {
        printf("Promedio de {10, 20, 30}: %.2f\n", prom);
    }

    char buf[64];
    if (obtener_saludo("Estudiante", buf, sizeof(buf)) != NULL) {
        printf("Saludo generado: %s\n", buf);
    }

    return 0;
}
