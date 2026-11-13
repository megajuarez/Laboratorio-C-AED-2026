#ifndef VALIDACIONES_H
#define VALIDACIONES_H

#include <stdio.h>
#include <stdlib.h>

int validarEntero(const char *entrada, int *valor);
int validarDouble(const char *entrada, double *valor);
int leerEnteroRango(const char *mensaje, int min, int max);
int leerDoublePositivo(const char *mensaje, double *valor);
void mostrarAdvertencia(const char *mensaje);

#endif
