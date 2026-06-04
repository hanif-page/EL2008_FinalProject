#include <stdio.h>
#include <stdlib.h>
#include "app_logic.h"
#include "serial_comm.h"

int main() {
    // Sesuaikan "COM3" dengan nomor port tempat Arduino kamu terdeteksi di Device Manager
    const char* arduinoPort = "COM6"; 

    printf("==================================================\n");
    printf("   APLIKASI KONSOL INVENTARIS BARANG (PC SIDE)   \n");
    printf("==================================================\n");

    // 1. Membuka koneksi serial ke Arduino
    initSerialPort(arduinoPort);

    // 2. Menjalankan perulangan menu utama program
    runApp();

    // 3. Menutup port jika program keluar dari loop runApp (jika menggunakan mekanisme return)
    closeSerialPort();

    return 0;
}