#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h> // for Windows OS!
// #include <unistd.h>
#include "app_logic.h"
#include "user_interface.h"
#include "serial_comm.h"

char serialBuffer[4096]; // perbesar dari 1024, agar aman untuk maksimum 85 item

void fetchSerialUntil(const char* targetKeyword){
    serialBuffer[0] = '\0';
    char tempBuffer[4096];
    int attempts = 0;

    // waktu timeout maksimal 3 detik (30 iterasi * 100ms)
    while(attempts < 30){
        tempBuffer[0] = '\0';
        receiveSerialData(tempBuffer);

        if(strlen(tempBuffer) > 0){
            // gabungkan data baru ke buffer utama
            if (strlen(serialBuffer) + strlen(tempBuffer) < sizeof(serialBuffer)) {
                strcat(serialBuffer, tempBuffer);
            }
            
            // berhenti menunggu jika keyword target ATAU kata kunci error ditemukan
            if (strstr(serialBuffer, targetKeyword) != NULL || 
                strstr(serialBuffer, "ERR") != NULL || 
                strstr(serialBuffer, "ERROR") != NULL ||
                strstr(serialBuffer, "ERR: STORAGE_FULL") != NULL) {
                break; 
            }
        }
        Sleep(100); // tunggu 100ms agar CPU bekerja dengan aman
        attempts++; 
    }
}

void runApp()
{
    int pilihan;
    int running = 1;

    while(running)
    {
        // Sleep(3000); // for WIndows OS!
        // sleep(3); 
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
    if(id < 0 || id > 127)
    {
        printf("ERROR: ID harus 0-127\n");
        return;
    }

    sendSerialData("GET_ALL\n");
    fetchSerialUntil("END");

    char *line = strtok(serialBuffer, "\n");
    int idSudahAda = 0;
    int itemCount = 0;

    while(line != NULL)
    {
        if(strcmp(line, "END") == 0) break;
        itemCount++;
        int existingId;
        if(sscanf(line, "%d,", &existingId) == 1)
        {
            if(existingId == id)
            {
                idSudahAda = 1;
                break;
            }
        }
        line = strtok(NULL, "\n");
    }
    if(idSudahAda)
    {
        printf("ERROR: ID %d sudah digunakan!\n", id);
        return;
    }
    if (itemCount >= 85)
    {
        printf("\n=========================================================\n");
        printf("[GAGAL] Kapasitas penyimpanan EEPROM penuh (Maks 85 item)!\n");
        printf("[INFO]  Silakan hapus item lain terlebih dahulu untuk\n");
        printf("        menambahkan data inventaris terbaru.\n");
        printf("=========================================================\n");
        return;
    }
    
    promptString(
        "Nama (maks 8 karakter): ",
        nama,
        sizeof(nama)
    );

    // validasi string nama
    int lenNama = strlen(nama);
    if (lenNama == 0 || lenNama > 8)
    {
        printf("ERROR: Panjang nama harus 1-8 karakter\n");
        return;
    }
    for (int i = 0; i < lenNama; i++)
    {
        char c = nama[i];
        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')))
        {
            printf("ERROR: Nama hanya boleh berisi huruf dan angka\n");
            return;
        }
    }

    // keterangan kategori
    printf("\nBerikut keterangan untuk nomor kategori:\n");
    printf("0. Mikrokontroler\n");
    printf("1. Aktuator\n");
    printf("2. Sensor\n");
    printf("3. Lainnya\n");
    
    promptInt(
        "Kategori (0-3): ",
        &kategori
    );
    if(kategori < 0 || kategori > 3)
    {
        printf("ERROR: Kategori harus 0-3\n");
        return;
    }

    promptInt(
        "\nLokasi (0-63): ",
        &lokasi
    );
    if(lokasi < 0 || lokasi > 63)
    {
        printf("ERROR: Lokasi harus 0-63\n");
        return;
    }
    
    promptInt(
        "Qty Tersedia (0-63): ",
        &qTer
    );
    if(qTer < 0 || qTer > 63)
    {
        printf("ERROR: Qty harus 0-63\n");
        return;
    }
    promptInt(
        "Qty Dipinjam (0-63): ",
        &qDip
    );
    if(qDip < 0 || qDip > 63)
    {
        printf("ERROR: Qty harus 0-63\n");
        return;
    }
    promptInt(
        "Qty Rusak (0-63): ",
        &qRus
    );
    if(qRus < 0 || qRus > 63)
    {
        printf("ERROR: Qty harus 0-63\n");
        return;
    }
    
    promptString(
        "PIC (3 huruf): ",
        pic,
        sizeof(pic)
    );

    // validasi string PIC
    int lenPic = strlen(pic);
    if (lenPic != 3)
    {
        printf("ERROR: PIC harus tepat terdiri dari 3 karakter\n");
        return;
    }
    for (int i = 0; i < lenPic; i++)
    {
        char c = pic[i];
        if (!(c >= 'A' && c <= 'Z'))
        {
            printf("ERROR: PIC hanya boleh berisi huruf besar\n");
            return;
        }
    }

    snprintf(
        command,
        sizeof(command),
        "ADD,%d,%.8s,%d,%d,%d,%d,%d,%.3s\n",id,nama,kategori,lokasi,qTer,qDip,qRus,pic
    );

    printf("\nLoading...\n");

    sendSerialData(command);
    fetchSerialUntil("ACK_ADD"); // tunggu hingga EEPROM selesai ditulis
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

    printf("\nLoading...\n");

    sendSerialData(command);
    fetchSerialUntil("ACK_DEL"); 
    if (strstr(serialBuffer, "ERR") != NULL || strstr(serialBuffer, "ERROR") != NULL)
    {
        printf("ERROR: ID %d tidak ditemukan!\n", id);
    }
    else
    {
        printf("ID %d berhasil dihapus!", id);
    }
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
    fetchSerialUntil("END");

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
            const char* namaKategori[] = {"Mikrokontroler", "Aktuator", "Sensor", "Lainnya"};
            const char* strKategori = "X";

            if (kategori >= 0 && kategori <= 3) 
            {
                strKategori = namaKategori[kategori];
            }
            
            printf("\n===== DATA DITEMUKAN =====\n");
            printf("ID          : %d\n", id);
            printf("Nama        : %s\n", nama);
            printf("Kategori    : %s\n", strKategori);
            printf("Lokasi      : %d\n", lokasi);
            printf("Tersedia    : %d\n", qTer);
            printf("Dipinjam    : %d\n", qDip);
            printf("Rusak       : %d\n", qRus);
            printf("PIC         : %s\n", pic);
            printf("Pemilik     : Laboratorium Embedded Systems\n");

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

    sendSerialData("GET_ALL\n");
    fetchSerialUntil("END");

    // pengecekan id
    char *line = strtok(serialBuffer, "\n");
    int idDitemukan = 0;

    while(line != NULL)
    {
        if(strcmp(line, "END") == 0) break;
        int existingId;
        if(sscanf(line, "%d,", &existingId) == 1)
        {
            if(existingId == id)
            {
                idDitemukan = 1;
                break;
            }
        }
        line = strtok(NULL, "\n");
    }

    if(!idDitemukan)
    {
        printf("ERROR: ID %d tidak ditemukan!\n", id);
        return;
    }

    promptInt(
        "Qty Tersedia (0-63): ",
        &qTer
    );
    if(qTer < 0 || qTer > 63)
    {
        printf("ERROR: Qty harus 0-63\n");
        return;
    }
    promptInt(
        "Qty Dipinjam (0-63): ",
        &qDip
    );
    if(qDip < 0 || qDip > 63)
    {
        printf("ERROR: Qty harus 0-63\n");
        return;
    }
    promptInt(
        "Qty Rusak (0-63): ",
        &qRus
    );
    if(qRus < 0 || qRus > 63)
    {
        printf("ERROR: Qty harus 0-63\n");
        return;
    }
    
    snprintf(
        command,
        sizeof(command),
        "UPDATE,%d,%d,%d,%d\n",id,qTer,qDip,qRus
    );

    printf("\nLoading...\n");

    sendSerialData(command);
    fetchSerialUntil("ACK_UPDATE");
    printf("\nStock dan status berhasil diperbarui!\n");
}

void displayAllInventory()
{
    sendSerialData("GET_ALL\n");
    fetchSerialUntil("END");
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
            // mengubah angka kategori jadi nama kategorinya
            const char* namaKategori[] = {"Mikrokontroler", "Aktuator", "Sensor", "Lainnya"};
            const char* strKategori = "X";

            if (kategori >= 0 && kategori <= 3) 
            {
                strKategori = namaKategori[kategori];
            }

            printf(
                // format tabel diubah
                "  | %-3d | %-8.8s | %-15s |   %2d   |    %2d    |    %2d    |   %2d  | %-3.3s |\n",
                id,nama,strKategori,lokasi,qTer,qDip,qRus,pic
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

    fetchSerialUntil("END");

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