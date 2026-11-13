#include "artemis.h"
#include "validaciones.h"

#include <math.h>

static double calcularDistanciaEquilibrio(const DatosTLI *datos) {
    return datos->distancia_tierra_luna_m /
           (1.0 + sqrt(MASA_LUNA_KG / MASA_TIERRA_KG));
}

static double calcularPotencialEspecifico(const DatosTLI *datos, double distancia_tierra_m) {
    double distancia_luna_m = datos->distancia_tierra_luna_m - distancia_tierra_m;
    return -(G_CONST * MASA_TIERRA_KG / distancia_tierra_m) -
           (G_CONST * MASA_LUNA_KG / distancia_luna_m);
}

static int calcularPuntoDeRetorno(const DatosTLI *datos, double energia_total_especifica,
                                  double inicio_m, double fin_m, double *retorno_m) {
    int iteracion;
    double izquierda = inicio_m;
    double derecha = fin_m;

    for (iteracion = 0; iteracion < 80; iteracion++) {
        double medio = izquierda + (derecha - izquierda) / 2.0;
        if (calcularPotencialEspecifico(datos, medio) <= energia_total_especifica) {
            izquierda = medio;
        } else {
            derecha = medio;
        }
    }

    *retorno_m = izquierda;
    return isfinite(*retorno_m);
}

static int simularTrayectoria(const DatosTLI *datos, ResultadoTLI *resultado,
                              double potencial_inicial_especifico,
                              double potencial_equilibrio_especifico) {
    double distancia_inicial_m = RADIO_TIERRA_M + datos->altura_orbita_m;
    double energia_total_especifica =
        resultado->energia_cinetica_especifica_j_kg + potencial_inicial_especifico;
    double paso_m = (resultado->distancia_equilibrio_terramoon_m - distancia_inicial_m) /
                    MAX_TURNOS_SIMULACION;
    int turno;

    resultado->cantidad_turnos = 1;
    resultado->turnos[0].numero = 0;
    resultado->turnos[0].distancia_desde_tierra_m = distancia_inicial_m;
    resultado->turnos[0].velocidad_m_s = datos->velocidad_tli_m_s;

    for (turno = 1; turno <= MAX_TURNOS_SIMULACION; turno++) {
        double distancia_m = distancia_inicial_m + paso_m * turno;
        double potencial_especifico = calcularPotencialEspecifico(datos, distancia_m);
        double energia_cinetica_especifica = energia_total_especifica - potencial_especifico;

        if (energia_cinetica_especifica < 0.0) {
            double distancia_retorno_m;
            double distancia_anterior_m = resultado->turnos[turno - 1].distancia_desde_tierra_m;

            if (!calcularPuntoDeRetorno(datos, energia_total_especifica,
                                        distancia_anterior_m, distancia_m,
                                        &distancia_retorno_m)) {
                return 0;
            }

            resultado->turnos[turno].numero = turno;
            resultado->turnos[turno].distancia_desde_tierra_m = distancia_retorno_m;
            resultado->turnos[turno].velocidad_m_s = 0.0;
            resultado->cantidad_turnos++;
            resultado->distancia_maxima_m = distancia_retorno_m;
            resultado->velocidad_equilibrio_m_s = 0.0;
            resultado->alcanza_equilibrio = 0;
            return 1;
        }

        resultado->turnos[turno].numero = turno;
        resultado->turnos[turno].distancia_desde_tierra_m = distancia_m;
        resultado->turnos[turno].velocidad_m_s = sqrt(2.0 * energia_cinetica_especifica);
        resultado->cantidad_turnos++;
    }

    resultado->distancia_maxima_m = resultado->distancia_equilibrio_terramoon_m;
    resultado->velocidad_equilibrio_m_s =
        sqrt(2.0 * (energia_total_especifica - potencial_equilibrio_especifico));
    resultado->alcanza_equilibrio = 1;
    return isfinite(resultado->velocidad_equilibrio_m_s);
}

void inicializarDatosPorDefecto(DatosTLI *datos) {
    if (datos == NULL) {
        return;
    }

    datos->velocidad_tli_m_s = 10900.0;
    datos->masa_nave_kg = 26000.0;
    datos->altura_orbita_m = 192000.0;
    datos->distancia_tierra_luna_m = DISTANCIA_TIERRA_LUNA_M;
}

void inicializarCasoExitoso(DatosTLI *datos) {
    inicializarDatosPorDefecto(datos);
    if (datos != NULL) {
        datos->velocidad_tli_m_s = 10950.0;
    }
}

static void imprimirComparacionPozosGravitatorios(void) {
    double velocidad_escape_tierra =
        sqrt((2.0 * G_CONST * MASA_TIERRA_KG) / RADIO_TIERRA_M);
    double velocidad_escape_luna =
        sqrt((2.0 * G_CONST * MASA_LUNA_KG) / RADIO_LUNA_M);

    printf("   Velocidad de escape desde la superficie terrestre: %.2f km/s.\n",
           velocidad_escape_tierra / 1000.0);
    printf("   Velocidad de escape desde la superficie lunar: %.2f km/s.\n",
           velocidad_escape_luna / 1000.0);
    printf("   La Luna tiene unas 81 veces menos masa que la Tierra, por lo que su\n");
    printf("   pozo gravitatorio es mucho menos profundo. La gravedad terrestre\n");
    printf("   puede aportar energia durante el regreso, aunque la captura y el\n");
    printf("   frenado tambien requieren maniobras y energia.\n");
}

void mostrarFundamentoFisico(void) {
    printf("\n=== Primera ley de Newton ===\n");
    printf("La primera ley establece que la velocidad permanece constante solo si\n");
    printf("la fuerza neta sobre el cuerpo es cero. Al apagarse los motores de Orion,\n");
    printf("la fuerza neta no es cero: actua la gravedad terrestre y, en menor medida,\n");
    printf("la lunar. Por eso cambia la velocidad; durante el alejamiento, la gravedad\n");
    printf("terrestre reduce la rapidez y curva la trayectoria hacia la Tierra.\n");
    printf("\n=== Profundidad de los pozos gravitatorios ===\n");
    imprimirComparacionPozosGravitatorios();
}

int validarDatosTLI(const DatosTLI *datos) {
    double radio_orbital_m;
    double distancia_equilibrio_m;
    double potencial_inicial;
    double potencial_equilibrio;
    double energia_cinetica_especifica;
    double energia_potencial_inicial_j;
    double energia_potencial_equilibrio_j;

    if (datos == NULL ||
        !isfinite(datos->velocidad_tli_m_s) ||
        !isfinite(datos->masa_nave_kg) ||
        !isfinite(datos->altura_orbita_m) ||
        !isfinite(datos->distancia_tierra_luna_m) ||
        datos->velocidad_tli_m_s <= 0.0 ||
        datos->velocidad_tli_m_s >= VELOCIDAD_LUZ_M_S ||
        datos->masa_nave_kg <= 0.0 ||
        datos->altura_orbita_m <= 0.0 ||
        datos->distancia_tierra_luna_m <= 0.0) {
        return 0;
    }

    radio_orbital_m = RADIO_TIERRA_M + datos->altura_orbita_m;
    if (!isfinite(radio_orbital_m) || datos->distancia_tierra_luna_m <= radio_orbital_m) {
        return 0;
    }

    distancia_equilibrio_m = calcularDistanciaEquilibrio(datos);
    if (!isfinite(distancia_equilibrio_m) ||
        distancia_equilibrio_m <= radio_orbital_m ||
        distancia_equilibrio_m >= datos->distancia_tierra_luna_m) {
        return 0;
    }

    potencial_inicial = calcularPotencialEspecifico(datos, radio_orbital_m);
    potencial_equilibrio = calcularPotencialEspecifico(datos, distancia_equilibrio_m);
    energia_cinetica_especifica = 0.5 * datos->velocidad_tli_m_s * datos->velocidad_tli_m_s;
    energia_potencial_inicial_j = datos->masa_nave_kg * potencial_inicial;
    energia_potencial_equilibrio_j = datos->masa_nave_kg * potencial_equilibrio;

    return isfinite(potencial_inicial) &&
           isfinite(potencial_equilibrio) &&
           isfinite(energia_cinetica_especifica) &&
           isfinite(energia_potencial_inicial_j) &&
           isfinite(energia_potencial_equilibrio_j) &&
           isfinite(datos->masa_nave_kg * energia_cinetica_especifica);
}

int ingresarDatosUsuario(DatosTLI *datos) {
    DatosTLI candidato;
    double *valores[4];
    const char *mensajes[4] = {
        "Velocidad TLI (m/s): ",
        "Masa de Orion (kg): ",
        "Altura de la orbita baja (m): ",
        "Distancia Tierra-Luna (m): "
    };
    int i;

    if (datos == NULL) {
        return 0;
    }

    inicializarDatosPorDefecto(&candidato);
    valores[0] = &candidato.velocidad_tli_m_s;
    valores[1] = &candidato.masa_nave_kg;
    valores[2] = &candidato.altura_orbita_m;
    valores[3] = &candidato.distancia_tierra_luna_m;

    printf("\nIngrese los cuatro datos. Si alguno es invalido, la carga se cancela.\n");
    for (i = 0; i < 4; i++) {
        if (!leerDoublePositivo(mensajes[i], valores[i])) {
            mostrarAdvertencia("Carga cancelada; se conservan los datos anteriores.");
            return 0;
        }
    }

    if (!validarDatosTLI(&candidato)) {
        mostrarAdvertencia("Los datos no son fisicamente consistentes o exceden el rango calculable.");
        return 0;
    }

    *datos = candidato;
    printf("\nDatos personalizados aceptados.\n");
    return 1;
}

int calcularResultadoTLI(const DatosTLI *datos, ResultadoTLI *resultado) {
    double radio_orbital_m;
    double distancia_equilibrio_m;
    double distancia_inicial_luna_m;
    double distancia_equilibrio_luna_m;
    double potencial_inicial_especifico;
    double potencial_equilibrio_especifico;

    if (resultado == NULL || !validarDatosTLI(datos)) {
        return 0;
    }

    radio_orbital_m = RADIO_TIERRA_M + datos->altura_orbita_m;
    distancia_equilibrio_m = calcularDistanciaEquilibrio(datos);
    distancia_inicial_luna_m = datos->distancia_tierra_luna_m - radio_orbital_m;
    distancia_equilibrio_luna_m = datos->distancia_tierra_luna_m - distancia_equilibrio_m;
    potencial_inicial_especifico = calcularPotencialEspecifico(datos, radio_orbital_m);
    potencial_equilibrio_especifico =
        calcularPotencialEspecifico(datos, distancia_equilibrio_m);

    resultado->energia_cinetica_especifica_j_kg =
        0.5 * datos->velocidad_tli_m_s * datos->velocidad_tli_m_s;
    resultado->energia_cinetica_j =
        datos->masa_nave_kg * resultado->energia_cinetica_especifica_j_kg;
    resultado->potencial_terrestre_inicial_j =
        -(G_CONST * MASA_TIERRA_KG * datos->masa_nave_kg) / radio_orbital_m;
    resultado->potencial_lunar_inicial_j =
        -(G_CONST * MASA_LUNA_KG * datos->masa_nave_kg) / distancia_inicial_luna_m;
    resultado->energia_potencial_inicial_j =
        resultado->potencial_terrestre_inicial_j + resultado->potencial_lunar_inicial_j;
    resultado->potencial_terrestre_equilibrio_j =
        -(G_CONST * MASA_TIERRA_KG * datos->masa_nave_kg) / distancia_equilibrio_m;
    resultado->potencial_lunar_equilibrio_j =
        -(G_CONST * MASA_LUNA_KG * datos->masa_nave_kg) / distancia_equilibrio_luna_m;
    resultado->energia_potencial_equilibrio_j =
        resultado->potencial_terrestre_equilibrio_j +
        resultado->potencial_lunar_equilibrio_j;
    resultado->energia_total_inicial_j =
        resultado->energia_cinetica_j + resultado->energia_potencial_inicial_j;
    resultado->distancia_equilibrio_terramoon_m = distancia_equilibrio_m;
    resultado->trabajo_ext_por_unidad_masa_j_kg =
        potencial_equilibrio_especifico - potencial_inicial_especifico;

    if (!isfinite(resultado->energia_total_inicial_j) ||
        !isfinite(resultado->energia_potencial_equilibrio_j) ||
        !isfinite(resultado->trabajo_ext_por_unidad_masa_j_kg)) {
        return 0;
    }

    return simularTrayectoria(datos, resultado, potencial_inicial_especifico,
                              potencial_equilibrio_especifico);
}

void imprimirResultadoTLI(const ResultadoTLI *resultado, const DatosTLI *datos) {
    int i;

    printf("\n=== REPORTE FINAL: INYECCION TRANSLUNAR ===\n");
    printf("Velocidad TLI: %.2f m/s | Masa de Orion: %.2f kg\n",
           datos->velocidad_tli_m_s, datos->masa_nave_kg);
    printf("Orbita inicial: %.2f km sobre la superficie terrestre\n",
           datos->altura_orbita_m / 1000.0);
    printf("Distancia Tierra-Luna: %.2f km\n",
           datos->distancia_tierra_luna_m / 1000.0);

    printf("\na) Inercia y gravedad\n");
    printf("   La primera ley exige fuerza neta cero para que la velocidad permanezca\n");
    printf("   constante. Con los motores apagados, la gravedad terrestre y lunar\n");
    printf("   siguen actuando, por lo que cambia el vector velocidad de Orion.\n");
    printf("   En el tramo de alejamiento, la componente de la gravedad terrestre\n");
    printf("   opuesta al movimiento reduce gradualmente la rapidez.\n");

    printf("\nb) Energia cinetica inicial\n");
    printf("   Ki = %.3e J (%.3e J/kg)\n",
           resultado->energia_cinetica_j,
           resultado->energia_cinetica_especifica_j_kg);
    printf("   Potencial inicial total (Tierra + Luna): %.3e J\n",
           resultado->energia_potencial_inicial_j);
    printf("   Energia mecanica inicial: %.3e J\n", resultado->energia_total_inicial_j);

    printf("\nc) Energia para llegar al punto de equilibrio\n");
    printf("   Distancia al punto de equilibrio: %.3f km desde el centro terrestre\n",
           resultado->distancia_equilibrio_terramoon_m / 1000.0);
    printf("   Potencial gravitatorio inicial (Tierra): %.3e J\n",
           resultado->potencial_terrestre_inicial_j);
    printf("   Potencial gravitatorio inicial (Luna): %.3e J\n",
           resultado->potencial_lunar_inicial_j);
    printf("   Potencial en el punto de equilibrio (Tierra + Luna): %.3e J\n",
           resultado->energia_potencial_equilibrio_j);
    printf("   Barrera energetica / Wext minimo por unidad de masa: %.3e J/kg\n",
           resultado->trabajo_ext_por_unidad_masa_j_kg);
    printf("   Criterio de llegada: Ki/m debe ser mayor o igual que la barrera;\n");
    printf("   en el limite, la rapidez en el punto de equilibrio es cero.\n");

    printf("\nSimulacion por turnos (modelo radial simplificado, %d tramos):\n",
           MAX_TURNOS_SIMULACION);
    printf("Turno | Distancia desde Tierra (km) | Rapidez (m/s)\n");
    for (i = 0; i < resultado->cantidad_turnos; i++) {
        printf("%5d | %27.3f | %12.2f\n",
               resultado->turnos[i].numero,
               resultado->turnos[i].distancia_desde_tierra_m / 1000.0,
               resultado->turnos[i].velocidad_m_s);
    }

    if (resultado->alcanza_equilibrio) {
        printf("Resultado: Orion alcanza el punto de equilibrio con %.2f m/s.\n",
               resultado->velocidad_equilibrio_m_s);
    } else {
        printf("Resultado: Orion no alcanza el punto de equilibrio; se detiene\n");
        printf("momentaneamente a %.3f km del centro terrestre antes de regresar.\n",
               resultado->distancia_maxima_m / 1000.0);
    }

    printf("\nd) Comparacion energetica ida y regreso\n");
    imprimirComparacionPozosGravitatorios();
    printf("   Es una comparacion idealizada; el balance real depende de la trayectoria.\n");
    printf("==================================================\n");
}

void ejecutarCalculoConDatos(const DatosTLI *datos) {
    ResultadoTLI resultado;

    if (!calcularResultadoTLI(datos, &resultado)) {
        mostrarAdvertencia("No se pudo calcular la simulacion con estos datos.");
        return;
    }

    imprimirResultadoTLI(&resultado, datos);
}
