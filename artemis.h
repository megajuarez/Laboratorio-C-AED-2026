#ifndef ARTEMIS_H
#define ARTEMIS_H

#include <stdio.h>

#define G_CONST 6.674e-11
#define MASA_TIERRA_KG 5.972e24
#define MASA_LUNA_KG 7.347e22
#define RADIO_TIERRA_M 6.371e6
#define RADIO_LUNA_M 1.7374e6
#define DISTANCIA_TIERRA_LUNA_M 3.84e8
#define VELOCIDAD_LUZ_M_S 299792458.0
#define MAX_TURNOS_SIMULACION 10

typedef struct {
    double velocidad_tli_m_s;
    double masa_nave_kg;
    double altura_orbita_m;
    double distancia_tierra_luna_m;
} DatosTLI;

typedef struct {
    int numero;
    double distancia_desde_tierra_m;
    double velocidad_m_s;
} TurnoSimulacion;

typedef struct {
    double energia_cinetica_j;
    double energia_cinetica_especifica_j_kg;
    double energia_potencial_inicial_j;
    double energia_potencial_equilibrio_j;
    double potencial_terrestre_inicial_j;
    double potencial_lunar_inicial_j;
    double potencial_terrestre_equilibrio_j;
    double potencial_lunar_equilibrio_j;
    double energia_total_inicial_j;
    double distancia_equilibrio_terramoon_m;
    double trabajo_ext_por_unidad_masa_j_kg;
    double distancia_maxima_m;
    double velocidad_equilibrio_m_s;
    int alcanza_equilibrio;
    int cantidad_turnos;
    TurnoSimulacion turnos[MAX_TURNOS_SIMULACION + 1];
} ResultadoTLI;

void inicializarDatosPorDefecto(DatosTLI *datos);
void inicializarCasoExitoso(DatosTLI *datos);
void mostrarFundamentoFisico(void);
int validarDatosTLI(const DatosTLI *datos);
int ingresarDatosUsuario(DatosTLI *datos);
int calcularResultadoTLI(const DatosTLI *datos, ResultadoTLI *resultado);
void imprimirResultadoTLI(const ResultadoTLI *resultado, const DatosTLI *datos);
void ejecutarCalculoConDatos(const DatosTLI *datos);

#endif
