/*
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "app_logic.h"
#include "user_interface.h"
#include "serial_comm.h"

void runApp() {
    // Initialize serial port, display main menu in a while loop, 
    // and route the user's choice to the functions below.
}

void addInventory() {
    // Prompt UI for new item details, format into ADD string, 
    // send via serial, and print Arduino's ACK response.
}

void deleteInventory() {
    // Prompt UI for ID, format into DEL string, 
    // send via serial, and print Arduino's ACK response.
}

void searchByID() {
    // Prompt UI for ID, call GET_ALL via serial, 
    // parse the returned data, and display only the matching item.
}

void updateStockAndStatus() {
    // Prompt UI for ID, prompt for new quantities, 
    // format into ADD/UPDATE string, and send via serial to overwrite.
}

void displayAllInventory() {
    // Call GET_ALL via serial, parse the incoming CSV data, 
    // and print it using the UI header and formatted rows.
}

void displaySummary() {
    // Call GET_ALL via serial, parse data to calculate 
    // total available/borrowed/broken items, and print the math summary.
}
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "app_logic.h"
#include "user_interface.h"
#include "serial_comm.h"

char serialBuffer[1024];

void runApp()
{
    int pilihan;
    int running = 1;

    while(running)
    {
        printMainMenu();
        getUserChoice(&pilihan);

        switch(pilihan)
        {
            case 1:
                addInventory();
                break;
            case 2:
                deleteInventory();
                break;
            case 3:
                searchByID();
                break;
            case 4:
                updateStockAndStatus();
                break;
            case 5:
                displayAllInventory();
                break;
            case 6:
                displaySummary();
                break;
            case 7:
                printf("\nKeluar dari program...\n");
                running = 0;
                break;
            default:
                printf("\nPilihan tidak valid!\n");
        }
    }
}

void addInventory()
{
    int id, kategori,lokasi,qTer,qDip,qRus;
    char nama[20],pic[10];
    char command[128];

    printf("\n===== TAMBAH INVENTARIS =====\n");
    promptInt("ID (0-127): ", &id);
    promptString(
        "Nama (maks 8 karakter): ",
        nama,
        sizeof(nama)
    );
    promptInt(
        "Kategori (0-3): ",
        &kategori
    );
    promptInt(
        "Lokasi (0-63): ",
        &lokasi
    );
    promptInt(
        "Qty Tersedia (0-63): ",
        &qTer
    );
    promptInt(
        "Qty Dipinjam (0-63): ",
        &qDip
    );
    promptInt(
        "Qty Rusak (0-63): ",
        &qRus
    );
    promptString(
        "PIC (3 huruf): ",
        pic,
        sizeof(pic)
    );

    if(id < 0 || id > 127)
    {
        printf("ERROR: ID harus 0-127\n");
        return;
    }
    if(kategori < 0 || kategori > 3)
    {
        printf("ERROR: Kategori harus 0-3\n");
        return;
    }
    if(lokasi < 0 || lokasi > 63)
    {
        printf("ERROR: Lokasi harus 0-63\n");
        return;
    }
    if(qTer < 0 || qTer > 63 ||
       qDip < 0 || qDip > 63 ||
       qRus < 0 || qRus > 63)
    {
        printf("ERROR: Qty harus 0-63\n");
        return;
    }

    snprintf(
        command,
        sizeof(command),
        "ADD,%d,%.8s,%d,%d,%d,%d,%d,%.3s\n",id,nama,kategori,lokasi,qTer,qDip,qRus,pic
    );

    sendSerialData(command);
    receiveSerialData(serialBuffer);
    printf("\n[Arduino] %s\n", serialBuffer);
}

void deleteInventory()
{
    int id;
    char command[32];

    printf("\n===== HAPUS INVENTARIS =====\n");
    promptInt(
        "Masukkan ID: ",
        &id
    );
    snprintf(
        command,
        sizeof(command),
        "DEL,%d\n",
        id
    );

    sendSerialData(command);
    receiveSerialData(serialBuffer);
    printf("\n[Arduino] %s\n", serialBuffer);
}

void searchByID()
{
    int targetID;
    printf("\n===== CARI BERDASARKAN ID =====\n");
    promptInt(
        "Masukkan ID: ",
        &targetID
    );
    sendSerialData("GET_ALL\n");
    receiveSerialData(serialBuffer);

    char *line = strtok(serialBuffer, "\n");

    while(line != NULL)
    {
        if(strcmp(line, "END") == 0)
        {
            break;
        }

        int id, kategori,lokasi,qTer,qDip,qRus;
        char nama[20],pic[10];
        int parsed =
            sscanf(
                line,
                "%d,%[^,],%d,%d,%d,%d,%d,%[^,\n]",&id,nama,&kategori,&lokasi,&qTer,&qDip,&qRus,pic
            );

        if(parsed == 8 && id == targetID)
        {
            printf("\n===== DATA DITEMUKAN =====\n");
            printf("ID          : %d\n", id);
            printf("Nama        : %s\n", nama);
            printf("Kategori    : %d\n", kategori);
            printf("Lokasi      : %d\n", lokasi);
            printf("Tersedia    : %d\n", qTer);
            printf("Dipinjam    : %d\n", qDip);
            printf("Rusak       : %d\n", qRus);
            printf("PIC         : %s\n", pic);

            return;
        }

        line = strtok(NULL, "\n");
    }

    printf("\nID tidak ditemukan.\n");
}

void updateStockAndStatus()
{
    int id,qTer,qDip,qRus;
    char command[64];

    printf("\n===== UPDATE INVENTARIS =====\n");
    promptInt(
        "ID Barang: ",
        &id
    );
    promptInt(
        "Qty Tersedia Baru: ",
        &qTer
    );
    promptInt(
        "Qty Dipinjam Baru: ",
        &qDip
    );
    promptInt(
        "Qty Rusak Baru: ",
        &qRus
    );
    snprintf(
        command,
        sizeof(command),
        "UPDATE,%d,%d,%d,%d\n",id,qTer,qDip,qRus
    );

    sendSerialData(command);
    receiveSerialData(serialBuffer);
    printf("\n[Arduino] %s\n", serialBuffer);
}

void displayAllInventory()
{
    sendSerialData("GET_ALL\n");
    receiveSerialData(serialBuffer);
    printTableHeader();

    char *line = strtok(serialBuffer, "\n");

    while(line != NULL)
    {
        if(strcmp(line, "END") == 0)
        {
            break;
        }

        int id, kategori,lokasi,qTer,qDip,qRus;
        char nama[20],pic[10];

        int parsed =
            sscanf(
                line,
                "%d,%[^,],%d,%d,%d,%d,%d,%[^,\n]",&id,nama,&kategori,&lokasi,&qTer,&qDip,&qRus,pic
            );

        if(parsed == 8)
        {
            printf(
                "%3d | %-8s | %2d | %2d | %2d | %2d | %2d | %-3s\n",
                id,nama,kategori,lokasi,qTer,qDip,qRus,pic
            );
        }

        line = strtok(NULL, "\n");
    }

    printTableFooter();
}

void displaySummary()
{
    int totalJenis = 0;
    int totalTersedia = 0;
    int totalDipinjam = 0;
    int totalRusak = 0;

    sendSerialData("GET_ALL\n");

    receiveSerialData(serialBuffer);

    char *line = strtok(serialBuffer, "\n");

    while(line != NULL)
    {
        if(strcmp(line, "END") == 0)
        {
            break;
        }

        int id, kategori,lokasi,qTer,qDip,qRus;
        char nama[20],pic[10];

        int parsed =
            sscanf(
                line,
                "%d,%[^,],%d,%d,%d,%d,%d,%[^,\n]",&id,nama,&kategori,&lokasi,&qTer,&qDip,&qRus,pic
            );

        if(parsed == 8)
        {
            totalJenis++;
            totalTersedia += qTer;
            totalDipinjam += qDip;
            totalRusak += qRus;
        }

        line = strtok(NULL, "\n");
    }

    printf("\n========== RINGKASAN ==========\n");
    printf("Jumlah Jenis Barang : %d\n", totalJenis);
    printf("Total Tersedia      : %d\n", totalTersedia);
    printf("Total Dipinjam      : %d\n", totalDipinjam);
    printf("Total Rusak         : %d\n", totalRusak);
    printf("===============================\n");
}