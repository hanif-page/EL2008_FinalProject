#include <stdio.h>
#include "app_logic.h"
#include "serial_comm.h"

int main()
{
    char portName[20];

    printf("=====================================\n");
    printf("   INVENTORY APP LOGIC TEST MODE\n");
    printf("=====================================\n");

    printf("Masukkan COM Port Arduino (contoh COM5): ");
    scanf("%19s", portName);

    initSerialPort(portName);

    printf("\n=====================================\n");
    printf("Koneksi berhasil.\n");
    printf("Silakan uji seluruh fitur App Logic.\n");
    printf("=====================================\n");

    runApp();

    closeSerialPort();

    printf("\nProgram selesai.\n");

    return 0;
}