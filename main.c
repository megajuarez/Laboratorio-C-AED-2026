#include "artemis.h"
#include "validaciones.h"

static void mostrarMenu(void) {
    printf("\n========================================\n");
    printf("   SIMULADOR ARTEMIS - ESCENARIO 3\n");
    printf("========================================\n");
    printf("1. Simular con valores predeterminados\n");
    printf("2. Ingresar valores personalizados\n");
    printf("3. Caso exitoso\n");
    printf("4. Fundamento fisico\n");
    printf("5. Salir\n");
    printf("========================================\n");
}

int main(void) {
    DatosTLI datos;
    int opcion;

    inicializarDatosPorDefecto(&datos);

    do {
        mostrarMenu();
        opcion = leerEnteroRango("Seleccione una opcion: ", 1, 5);
        if (opcion == 0) {
            break;
        }

        switch (opcion) {
            case 1:
                inicializarDatosPorDefecto(&datos);
                ejecutarCalculoConDatos(&datos);
                break;
            case 2:
                if (ingresarDatosUsuario(&datos)) {
                    ejecutarCalculoConDatos(&datos);
                }
                break;
            case 3:
                inicializarCasoExitoso(&datos);
                ejecutarCalculoConDatos(&datos);
                break;
            case 4:
                mostrarFundamentoFisico();
                break;
            case 5:
                printf("\nSaliendo del simulador Artemis...\n");
                break;
            default:
                break;
        }
    } while (opcion != 5);

    return 0;
}
