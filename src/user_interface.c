/*
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "user_interface.h"

void printMainMenu() {
    // Print the welcome banner and the numbered list of 
    // available inventory operations (1-7).
}

void getUserChoice(int* choice) {
    // Print a prompt, capture the integer using scanf, 
    // and safely clear the input buffer.
}

void promptString(const char* promptText, char* inputBuffer, int maxLength) {
    // Print the prompt text, safely capture string using fgets 
    // up to maxLength, and strip the trailing newline character.
}

void promptInt(const char* promptText, int* inputValue) {
    // Print the prompt text, capture the integer using scanf, 
    // and safely flush the leftover newline from the stdin buffer.
}

void printHeader() {
    // Print the formatted table header (e.g., ID | Name | Tersedia | ...) 
    // for use before listing inventory items.
}
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "user_interface.h"

void printMainMenu() {
    printf("\n");
    printf("  +------------------------------------------+\n");
    printf("  |     SISTEM INVENTARISASI LABORATORIUM    |\n");
    printf("  +------------------------------------------+\n");
    printf("  |                                          |\n");
    printf("  |  1. Tambah Data                          |\n");
    printf("  |  2. Hapus Data                           |\n");
    printf("  |  3. Cari Data (ID)                       |\n");
    printf("  |  4. Update Stok & Status                 |\n");
    printf("  |  5. Tampilkan Semua                      |\n");
    printf("  |  6. Ringkasan Inventaris                 |\n");
    printf("  |  7. Keluar                               |\n");
    printf("  |                                          |\n");
    printf("  +------------------------------------------+\n");
}

void getUserChoice(int* choice) {
    printf("Pilihan (1-7): ");
    scanf("%d", choice);
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

void promptString(const char* promptText, char* inputBuffer, int maxLength) {
    printf("%s", promptText);
    
    if (fgets(inputBuffer, maxLength, stdin) != NULL) {
        size_t len = strlen(inputBuffer);
        if (len > 0 && inputBuffer[len - 1] == '\n') {
            inputBuffer[len - 1] = '\0';
        } else {
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}
        }
    }
}

void promptInt(const char* promptText, int* inputValue) {
    printf("%s", promptText);
    scanf("%d", inputValue);
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

void printTableHeader() {
    printf("\n");
    printf("  +------+------------------+------------+------------+-------+-------+-------+----------------+\n");
    printf("  |  ID  |   Nama Barang    |  Kategori  |   Lokasi   | Avail | Loan  | Broken|       PIC      |\n");
    printf("  +------+------------------+------------+------------+-------+-------+-------+----------------+\n");
}

void printTableFooter() {
    printf("  +------+------------------+------------+------------+-------+-------+-------+----------------+\n");
}

// Format nulis dalam tabel (kalau butuh)
//printf("  | %-4d | %-16s | %-10s | %-10s | %-5d | %-5d | %-5d | %-14s |\n",
//       db[i].id, db[i].nama, db[i].kategori, db[i].lokasi, 
//       db[i].tersedia, db[i].dipinjam, db[i].rusak, db[i].pic);