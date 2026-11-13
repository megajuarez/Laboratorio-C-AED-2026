#include "validaciones.h"

#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>

int validarEntero(const char *entrada, int *valor) {
    char *fin = NULL;
    long numero;

    if (entrada == NULL || valor == NULL) {
        return 0;
    }

    errno = 0;
    numero = strtol(entrada, &fin, 10);
    if (fin == entrada || errno == ERANGE) {
        return 0;
    }

    while (*fin == ' ' || *fin == '\n' || *fin == '\r' || *fin == '\t') {
        fin++;
    }

    if (*fin != '\0' || numero < INT_MIN || numero > INT_MAX) {
        return 0;
    }

    *valor = (int)numero;
    return 1;
}

int validarDouble(const char *entrada, double *valor) {
    char *fin = NULL;
    double numero;

    if (entrada == NULL || valor == NULL) {
        return 0;
    }

    errno = 0;
    numero = strtod(entrada, &fin);
    if (fin == entrada || errno == ERANGE) {
        return 0;
    }

    while (*fin == ' ' || *fin == '\n' || *fin == '\r' || *fin == '\t') {
        fin++;
    }

    if (*fin != '\0' || !isfinite(numero) || numero <= 0.0) {
        return 0;
    }

    *valor = numero;
    return 1;
}

int leerEnteroRango(const char *mensaje, int min, int max) {
    char entrada[64];
    int numero;

    while (1) {
        printf("%s", mensaje);
        if (fgets(entrada, sizeof(entrada), stdin) == NULL) {
            printf("\nFin de entrada detectado. Cerrando simulador.\n");
            return 0;
        }

        if (!validarEntero(entrada, &numero)) {
            mostrarAdvertencia("Debe ingresar un numero entero valido.");
            continue;
        }

        if (numero < min || numero > max) {
            printf("El valor debe estar entre %d y %d.\n", min, max);
            continue;
        }

        return numero;
    }
}

int leerDoublePositivo(const char *mensaje, double *valor) {
    char entrada[128];

    while (1) {
        printf("%s", mensaje);
        if (fgets(entrada, sizeof(entrada), stdin) == NULL) {
            return 0;
        }

        if (!validarDouble(entrada, valor)) {
            mostrarAdvertencia("Debe ingresar un numero positivo y finito.");
            continue;
        }

        return 1;
    }
}

void mostrarAdvertencia(const char *mensaje) {
    printf("\n[ADVERTENCIA] %s\n", mensaje);
}
