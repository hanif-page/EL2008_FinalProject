#include "serial_comm.h"
#include <windows.h>
#include <stdio.h>
#include <string.h>
 
static HANDLE hSerial = INVALID_HANDLE_VALUE;
 
void initSerialPort(const char* portName) {
    printf("[SYSTEM] Mencoba terhubung ke %s (Windows)...\n", portName);
 
    char portPath[32];
    snprintf(portPath, sizeof(portPath), "\\\\.\\%s", portName);
 
    hSerial = CreateFileA(
        portPath,
        GENERIC_READ | GENERIC_WRITE,
        0, NULL, OPEN_EXISTING, 0, NULL
    );
 
    if (hSerial == INVALID_HANDLE_VALUE) {
        printf("[ERROR] Gagal membuka port %s. Pastikan tidak sedang dipakai Arduino IDE.\n", portName);
        return;
    }
 
    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
 
    if (!GetCommState(hSerial, &dcbSerialParams)) {
        printf("[ERROR] Gagal membaca status port.\n");
        CloseHandle(hSerial);
        hSerial = INVALID_HANDLE_VALUE;
        return;
    }
 
    dcbSerialParams.BaudRate = CBR_9600;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity   = NOPARITY;
 
    if (!SetCommState(hSerial, &dcbSerialParams)) {
        printf("[ERROR] Gagal mengatur parameter port.\n");
        return;
    }
 
    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout         = 100;  // Naikkan agar tidak timeout terlalu cepat
    timeouts.ReadTotalTimeoutConstant    = 500;  // Total maks 500ms per baca
    timeouts.ReadTotalTimeoutMultiplier  = 10;
    timeouts.WriteTotalTimeoutConstant   = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;
 
    SetCommTimeouts(hSerial, &timeouts);
 
    printf("[SYSTEM] Port terbuka. Menunggu 2.5 detik untuk Arduino boot...\n");
    Sleep(2500);
    PurgeComm(hSerial, PURGE_RXCLEAR | PURGE_TXCLEAR);
    printf("[SYSTEM] Koneksi Serial Siap!\n");
}
 
void sendSerialData(const char* dataStr) {
    if (hSerial == INVALID_HANDLE_VALUE) return;
 
    DWORD bytesWritten;
    WriteFile(hSerial, dataStr, strlen(dataStr), &bytesWritten, NULL);
    Sleep(50);
}
 
void receiveSerialData(char* buffer) {
    if (hSerial == INVALID_HANDLE_VALUE) {
        strcpy(buffer, "ERR_PORT_CLOSED");
        return;
    }
 
    DWORD bytesRead;
    char  temp_char;
    int   total_bytes = 0;
 
    while (ReadFile(hSerial, &temp_char, 1, &bytesRead, NULL) && bytesRead > 0) {
        buffer[total_bytes++] = temp_char;
        if (total_bytes >= 4095) break;
    }
    buffer[total_bytes] = '\0';
}
 
void closeSerialPort() {
    if (hSerial != INVALID_HANDLE_VALUE) {
        CloseHandle(hSerial);
        printf("[SYSTEM] Koneksi Serial Windows diputus.\n");
        hSerial = INVALID_HANDLE_VALUE;
    }
}
 
void serial_send(const char* dataStr) {
    sendSerialData(dataStr);
}
 
// [FIX BUG 3 — sumber masalah]
// Versi lama memanggil receiveSerialData() yang membaca SELURUH buffer serial
// sekaligus. Ketika dipanggil dalam while-loop di searchByID() / displaySummary():
//   - Iterasi 1: semua data masuk ke buffer, tapi hanya baris pertama diproses.
//   - Iterasi 2: serial sudah kosong → return 0 → loop selesai.
// Akibatnya hanya item pertama yang pernah diproses.
//
// Fix: baca karakter satu per satu hingga '\n' ditemukan atau timeout.
// Setiap pemanggilan menghasilkan tepat SATU baris → while-loop di caller bekerja.
//
// CATATAN: Fungsi ini kini tidak lagi dipakai oleh app_logic.c (sudah direfaktor
// ke pola bulk-read + parse in-memory), tetapi tetap diperbaiki di sini
// agar tidak menjadi jebakan bug di masa depan.
int serial_receive_line(char* buffer, int maxLength) {
    if (hSerial == INVALID_HANDLE_VALUE) return 0;
 
    DWORD bytesRead;
    char  temp_char;
    int   pos = 0;
 
    while (pos < maxLength - 1) {
        if (!ReadFile(hSerial, &temp_char, 1, &bytesRead, NULL) || bytesRead == 0) {
            break; // Timeout — tidak ada lagi data masuk
        }
        if (temp_char == '\n') break;       // Akhir baris
        if (temp_char != '\r') {            // Abaikan carriage-return
            buffer[pos++] = temp_char;
        }
    }
    buffer[pos] = '\0';
    return pos > 0;
}