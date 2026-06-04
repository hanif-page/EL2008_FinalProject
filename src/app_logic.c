#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "app_logic.h"
#include "user_interface.h"
#include "serial_comm.h"

// Helper: ambil satu baris dari buffer secara aman tanpa strtok.
// Mengembalikan pointer ke baris berikutnya, atau NULL jika sudah habis.
// *line_out diisi pointer ke baris saat ini (sudah di-null-terminate).
static char* next_line(char* pos, char** line_out) {
    if (pos == NULL || *pos == '\0') {
        *line_out = NULL;
        return NULL;
    }

    *line_out = pos;

    char* newline = strchr(pos, '\n');
    if (newline) {
        *newline = '\0';                  // akhiri baris di sini
        // Buang \r sebelum \n (Windows line ending)
        if (newline > pos && *(newline - 1) == '\r') {
            *(newline - 1) = '\0';
        }
        return newline + 1;              // kembalikan posisi baris berikutnya
    }

    // Baris terakhir (tidak ada \n di akhir)
    size_t len = strlen(pos);
    if (len > 0 && pos[len - 1] == '\r') pos[len - 1] = '\0';
    return pos + len; // pointer ke '\0', loop akan berhenti
}

// ==========================================
// MAIN LOOP
// ==========================================

void runApp() {
    int pilihan;
    while (1) {
        printMainMenu();
        getUserChoice(&pilihan);

        if      (pilihan == 1) addInventory();
        else if (pilihan == 2) deleteInventory();
        else if (pilihan == 3) searchByID();
        else if (pilihan == 4) updateStockAndStatus();
        else if (pilihan == 5) displayAllInventory();
        else if (pilihan == 6) displaySummary();
        else if (pilihan == 7) { closeSerialPort(); exit(0); }
        else printf("Pilihan tidak valid. Masukkan angka 1-7.\n");
    }
}

// ==========================================
// FUNGSI CRUD
// ==========================================

void addInventory() {
    int id, kat, lokasi, sedia, pinjam, rusak;
    char nama[50], pic[50];
    char cmd[256];
    char resp[128];

    promptInt("\nMasukkan ID (0-127): ", &id);
    promptString("Nama Barang (Max 8 Char): ", nama, sizeof(nama));

    while (1) {
        promptInt("Pilih Kategori (0-3): ", &kat);
        if (kat >= 0 && kat <= 3) break;
        printf("Kategori tidak valid!\n");
    }
    while (1) {
        promptInt("Lokasi Rak (0-63): ", &lokasi);
        if (lokasi >= 0 && lokasi <= 63) break;
        printf("Lokasi tidak valid! Masukkan 0-63.\n");
    }

    promptInt("Jumlah Barang Tersedia: ", &sedia);
    promptInt("Jumlah Barang Dipinjam: ", &pinjam);
    promptInt("Jumlah Barang Rusak: ",    &rusak);
    promptString("PIC Barang (Max 3 Char): ", pic, sizeof(pic));

    sprintf(cmd, "ADD,%d,%s,%d,%d,%d,%d,%d,%s\n",
            id, nama, kat, lokasi, sedia, pinjam, rusak, pic);
    sendSerialData(cmd);

    Sleep(200);
    receiveSerialData(resp);
    printf("Arduino: %s\n", resp);
}

void deleteInventory() {
    int id;
    char cmd[64], resp[128];

    promptInt("\nMasukkan ID barang yang ingin dihapus: ", &id);
    sprintf(cmd, "DEL,%d\n", id);
    sendSerialData(cmd);

    Sleep(200);
    receiveSerialData(resp);
    if (strlen(resp) > 0) printf("Arduino: %s\n", resp);
}

void updateStockAndStatus() {
    int id, s0, s1, s2;
    char cmd[128], resp[128];

    promptInt("\nMasukkan ID barang untuk update: ", &id);
    promptInt("Masukkan jumlah baru Tersedia: ",    &s0);
    promptInt("Masukkan jumlah baru Dipinjam: ",    &s1);
    promptInt("Masukkan jumlah baru Rusak: ",       &s2);

    sprintf(cmd, "UPDATE,%d,%d,%d,%d\n", id, s0, s1, s2);
    sendSerialData(cmd);

    Sleep(200);
    receiveSerialData(resp);
    if (strlen(resp) > 0) printf("Arduino: %s\n", resp);
}

// ==========================================
// FUNGSI DISPLAY
// (semua pakai strchr untuk loop baris,
//  strtok hanya untuk parsing koma per baris)
// ==========================================

void searchByID() {
    int target_id;
    char buffer[4096];
    int found = 0;

    promptInt("\nMasukkan ID barang yang dicari: ", &target_id);

    sendSerialData("GET_ALL\n");
    Sleep(1000);
    receiveSerialData(buffer);

    char* pos = buffer;
    char* line;
    while ((pos = next_line(pos, &line)) != NULL || (line != NULL && *line != '\0')) {
        if (line == NULL || *line == '\0') break;
        if (strncmp(line, "END",   3) == 0) break;
        if (strncmp(line, "EMPTY", 5) == 0) break;

        // strtok aman di sini karena tidak ada strtok lain yang berjalan
        // di luar fungsi ini untuk string yang sama.
        char* token;
        token = strtok(line, ","); if (!token) { pos = (pos && *pos) ? pos : NULL; continue; }
        int p_id = atoi(token);

        if (p_id != target_id) {
            // Bukan target, lanjut ke baris berikutnya
            if (pos == NULL || *pos == '\0') break;
            continue;
        }

        token = strtok(NULL, ","); char* p_nama   = token ? token : "";
        token = strtok(NULL, ","); int   p_kat    = token ? atoi(token) : 0;
        token = strtok(NULL, ","); int   p_lokasi = token ? atoi(token) : 0;
        token = strtok(NULL, ","); int   p_sedia  = token ? atoi(token) : 0;
        token = strtok(NULL, ","); int   p_pinjam = token ? atoi(token) : 0;
        token = strtok(NULL, ","); int   p_rusak  = token ? atoi(token) : 0;
        token = strtok(NULL, "\r\n"); char* p_pic = token ? token : "";

        printf("\n--- Data Ditemukan ---\n");
        printf("ID       : %d\n",   p_id);
        printf("Nama     : %s\n",   p_nama);
        printf("Kategori : %d\n",   p_kat);
        printf("Lokasi   : %d\n",   p_lokasi);
        printf("Total    : %d (Sedia: %d, Pinjam: %d, Rusak: %d)\n",
               p_sedia + p_pinjam + p_rusak, p_sedia, p_pinjam, p_rusak);
        printf("PIC      : %s\n",   p_pic);
        found = 1;
        break;
    }

    if (!found) printf("Barang dengan ID %d tidak ditemukan.\n", target_id);
}

void displayAllInventory() {
    char buffer[4096];
    printHeader();

    sendSerialData("GET_ALL\n");
    Sleep(1000);                  // Naikkan ke 1 detik — data banyak, 9600 baud lambat
    receiveSerialData(buffer);

    char* pos = buffer;
    char* line;
    // --- FIX NESTED STRTOK ---
    // Dulu: strtok(buffer, "\n") untuk loop baris, lalu strtok(tempLine, ",")
    // di dalam loop. Pemanggilan strtok kedua menimpa state strtok pertama,
    // sehingga strtok(NULL, "\n") di akhir loop selalu NULL → hanya 1 baris tampil.
    //
    // Fix: gunakan next_line() (berbasis strchr) untuk loop baris.
    // strtok hanya dipakai sekali per baris untuk parsing koma → tidak ada konflik.
    while ((pos = next_line(pos, &line)) != NULL || (line != NULL && *line != '\0')) {
        if (line == NULL || *line == '\0') break;
        if (strncmp(line, "END",   3) == 0) break;
        if (strncmp(line, "EMPTY", 5) == 0) break;

        char* token;
        token = strtok(line, ","); if (!token) { if (!pos || !*pos) break; continue; }
        int   p_id     = atoi(token);
        token = strtok(NULL, ","); char* p_nama   = token ? token : "";
        token = strtok(NULL, ","); int   p_kat    = token ? atoi(token) : 0;
        token = strtok(NULL, ","); int   p_lokasi = token ? atoi(token) : 0;
        token = strtok(NULL, ","); int   p_sedia  = token ? atoi(token) : 0;
        token = strtok(NULL, ","); int   p_pinjam = token ? atoi(token) : 0;
        token = strtok(NULL, ","); int   p_rusak  = token ? atoi(token) : 0;
        token = strtok(NULL, "\r\n"); char* p_pic = token ? token : "";

        printf("  | %-4d | %-16s | %-10d | %-10d | %-5d | %-5d | %-5d | %-14s |\n",
               p_id, p_nama, p_kat, p_lokasi,
               p_sedia, p_pinjam, p_rusak, p_pic);

        if (pos == NULL || *pos == '\0') break;
    }
    printTableFooter();
}

void displaySummary() {
    char buffer[4096];
    int total_jenis  = 0;
    int total_barang = 0;
    int total_rusak  = 0;

    sendSerialData("GET_ALL\n");
    Sleep(1000);
    receiveSerialData(buffer);

    char* pos = buffer;
    char* line;
    while ((pos = next_line(pos, &line)) != NULL || (line != NULL && *line != '\0')) {
        if (line == NULL || *line == '\0') break;
        if (strncmp(line, "END",   3) == 0) break;
        if (strncmp(line, "EMPTY", 5) == 0) break;

        char* token;
        strtok(line, ",");         // id
        strtok(NULL, ",");         // nama
        strtok(NULL, ",");         // kat
        strtok(NULL, ",");         // lokasi
        token = strtok(NULL, ","); int p_sedia  = token ? atoi(token) : 0;
        token = strtok(NULL, ","); int p_pinjam = token ? atoi(token) : 0;
        token = strtok(NULL, ","); int p_rusak  = token ? atoi(token) : 0;

        total_jenis++;
        total_barang += (p_sedia + p_pinjam + p_rusak);
        total_rusak  += p_rusak;

        if (pos == NULL || *pos == '\0') break;
    }

    printf("\n--- Ringkasan Inventaris ---\n");
    printf("Total Jenis Barang : %d\n", total_jenis);
    printf("Total Fisik Barang : %d\n", total_barang);
    printf("Total Barang Rusak : %d\n", total_rusak);
}