#include <stdio.h>
#include <stdlib.h>
#include "app_logic.h"
#include "serial_comm.h"

int main(int argc, char *argv[]) {
    char portName[64];
    int connectionStatus = 0; // Variabel penampung status

    printf("==========================================\n");
    printf("   SISTEM INVENTARISASI LABORATORIUM PC   \n");
    printf("==========================================\n\n");

    if (argc > 1) {
        snprintf(portName, sizeof(portName), "%s", argv[1]);
        printf("[INFO] Menggunakan port dari argumen: %s\n", portName);
    } else {
        printf("Masukkan Port Arduino (contoh: /dev/cu.usbmodem11301): ");
        scanf("%63s", portName);
        
        int c;
        while ((c = getchar()) != '\n' && c != EOF) {}
    }

    // Inisialisasi koneksi Serial dengan Pass-by-Reference
    initSerialPort(portName, &connectionStatus);

    // Cek apakah Arduino berhasil terhubung
    if (connectionStatus == 0) {
        printf("\n[FATAL] Program dihentikan karena perangkat keras tidak terdeteksi.\n");
        printf("Pastikan Arduino terhubung dan port yang dimasukkan benar.\n\n");
        return 1; // Keluar dari program
    }

    // Jalankan logika aplikasi jika koneksi sukses
    runApp();

    // Putus koneksi secara aman
    closeSerialPort();

    return 0;
}