#include "database.h"
#include <avr/io.h>
#include <avr/eeprom.h>
#include <stdlib.h>
#include <string.h>

Node* head = NULL;
Node* tail = NULL;

// ==========================================
// BARE-METAL UART DRIVER (16 MHz Clock)
// ==========================================
#define F_CPU 16000000UL
#define BAUD 9600
#define UBRR_VAL ((F_CPU/16/BAUD)-1)

void initUART(void) {
    UBRR0H = (uint8_t)(UBRR_VAL >> 8);
    UBRR0L = (uint8_t)UBRR_VAL;
    UCSR0B = (1 << RXEN0) | (1 << TXEN0); // Aktifkan RX dan TX
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8-bit data
}

void transmitUART(char data) {
    while (!(UCSR0A & (1 << UDRE0))); // Tunggu buffer kosong
    UDR0 = data;
}

void printString(const char* str) {
    while (*str) {
        transmitUART(*str++);
    }
}

// berubah!
// Pengganti Serial.available()
void dataAvailableUART(uint8_t* available) {
    *available = (UCSR0A & (1 << RXC0));
}

// berubah!
// Pengganti Serial.read()
void receiveUART(char* receivedData) {
    while (!(UCSR0A & (1 << RXC0)));
    *receivedData = UDR0;
}

// Fungsi konversi integer ke string manual (pengganti Serial.print(int))
void printInt(int num) {
    char buf[10];
    itoa(num, buf, 10);
    printString(buf);
}

// ==========================================
// BIT-LEVEL COMPRESSION & DECOMPRESSION
// ==========================================
void encodeChar6Bit(char inChar, uint8_t* outByte) {
    if (inChar >= 'A' && inChar <= 'Z') *outByte = inChar - 'A';
    else if (inChar >= 'a' && inChar <= 'z') *outByte = (inChar - 'a') + 26;
    else if (inChar >= '0' && inChar <= '9') *outByte = (inChar - '0') + 52;
    else *outByte = 62; 
}

void decodeChar6Bit(uint8_t inByte, char* outChar) {
    if (inByte <= 25) *outChar = inByte + 'A';
    else if (inByte <= 51) *outChar = (inByte - 26) + 'a';
    else if (inByte <= 61) *outChar = (inByte - 52) + '0';
    else *outChar = ' ';
}

void encodeChar5Bit(char inChar, uint8_t* outByte) {
    if (inChar >= 'A' && inChar <= 'Z') *outByte = inChar - 'A';
    else *outByte = 26; 
}

void decodeChar5Bit(uint8_t inByte, char* outChar) {
    if (inByte <= 25) *outChar = inByte + 'A';
    else *outChar = ' ';
}

void packItemData(ItemData* item, uint8_t id, const char* name, uint8_t cat, uint8_t loc, uint8_t qTer, uint8_t qDip, uint8_t qRus, const char* picStr) {
    item->id = id;
    item->category = cat;
    item->location = loc;
    item->qtyTersedia = qTer;
    item->qtyDipinjam = qDip;
    item->qtyRusak = qRus;

    uint8_t tempChar = 0;
    char padName[8] = {' ',' ',' ',' ',' ',' ',' ',' '};
    for(int i=0; i<8 && name[i]!='\0'; i++) padName[i] = name[i];

    encodeChar6Bit(padName[0], &tempChar); item->pName0 = tempChar;
    encodeChar6Bit(padName[1], &tempChar); item->pName1 = tempChar;
    encodeChar6Bit(padName[2], &tempChar); item->pName2 = tempChar;
    encodeChar6Bit(padName[3], &tempChar); item->pName3 = tempChar;
    encodeChar6Bit(padName[4], &tempChar); item->pName4 = tempChar;
    encodeChar6Bit(padName[5], &tempChar); item->pName5 = tempChar;
    encodeChar6Bit(padName[6], &tempChar); item->pName6 = tempChar;
    encodeChar6Bit(padName[7], &tempChar); item->pName7 = tempChar;

    char padPic[3] = {' ',' ',' '};
    for(int i=0; i<3 && picStr[i]!='\0'; i++) padPic[i] = picStr[i];
    
    uint8_t c1, c2, c3;
    encodeChar5Bit(padPic[0], &c1);
    encodeChar5Bit(padPic[1], &c2);
    encodeChar5Bit(padPic[2], &c3);
    item->pic = (c1 << 10) | (c2 << 5) | c3;
}

void unpackItemName(ItemData* item, char* outBuffer) {
    decodeChar6Bit(item->pName0, &outBuffer[0]);
    decodeChar6Bit(item->pName1, &outBuffer[1]);
    decodeChar6Bit(item->pName2, &outBuffer[2]);
    decodeChar6Bit(item->pName3, &outBuffer[3]);
    decodeChar6Bit(item->pName4, &outBuffer[4]);
    decodeChar6Bit(item->pName5, &outBuffer[5]);
    decodeChar6Bit(item->pName6, &outBuffer[6]);
    decodeChar6Bit(item->pName7, &outBuffer[7]);
    outBuffer[8] = '\0';
}

void unpackItemPIC(ItemData* item, char* outBuffer) {
    uint8_t c1 = (item->pic >> 10) & 0x1F;
    uint8_t c2 = (item->pic >> 5) & 0x1F;
    uint8_t c3 = item->pic & 0x1F;
    
    decodeChar5Bit(c1, &outBuffer[0]);
    decodeChar5Bit(c2, &outBuffer[1]);
    decodeChar5Bit(c3, &outBuffer[2]);
    outBuffer[3] = '\0';
}

// ==========================================
// LINKED LIST & MEMORY MANAGEMENT
// ==========================================
void initDatabase(void) {
    head = NULL;
    tail = NULL;
    
    for (int i = 0; i < MAX_ITEMS; i++) {
        ItemData temp;
        // Pengganti EEPROM.get
        eeprom_read_block(&temp, (const void*)(i * ITEM_SIZE), ITEM_SIZE);
        
        if (temp.id != 127 && temp.id <= 127) { // Validasi tambahan keamanan
            Node* newNode = (Node*)malloc(sizeof(Node));
            if(newNode == NULL) continue; // Cegah crash jika SRAM penuh
            
            newNode->data = temp;
            newNode->next = NULL;

            if (head == NULL) {
                head = newNode;
                tail = newNode;
            } else {
                tail->next = newNode;
                tail = newNode;
            }
        }
    }
}

void syncListToEEPROM(void) {
    Node* current = head;
    int index = 0;
    
    // Perbarui data yang ada di Linked List
    while (current != NULL && index < MAX_ITEMS) {
        // eeprom_update_block speed > eeprom_write_block speed
        // reason: karena hanya menulis byte yang benar-benar mengalami perubahan
        eeprom_update_block(&(current->data), (void*)(index * ITEM_SIZE), ITEM_SIZE);
        current = current->next;
        index++;
    }
    
    // Kasih tanda batas akhir (Terminator) ke sisa slot yang kosong
    // Tidak perlu menghapus 1.024 byte. Cukup rusak byte pertama dari sisa slot menjadi 255 (sehingga membuat ID = 127 / Kosong).
    while (index < MAX_ITEMS) {
        eeprom_update_byte((uint8_t*)(index * ITEM_SIZE), 255);
        index++;
    }
}

void insertItem(ItemData newData) {
    // check maximum capacity
    int currentCount = 0;
    Node* temp = head;
    while (temp != NULL) {
        currentCount++;
        temp = temp->next;
    }

    if (currentCount >= MAX_ITEMS) {
        // reject request and send error message to the client
        printString("ERR: STORAGE_FULL\n");
        return; 
    }

    // if there's still room in the actual capacity (< 85), then continue
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        printString("ERR: SRAM_FULL\n");
        return;
    }
    
    newNode->data = newData;
    newNode->next = NULL;

    if (head == NULL) {
        head = newNode;
        tail = newNode;
    } else {
        tail->next = newNode;
        tail = newNode;
    }

    syncListToEEPROM();
    printString("ACK_ADD\n");
}

void deleteItem(uint8_t targetId) {
    if (head == NULL) {
        printString("ERR: EMPTY\n");
        return;
    }

    Node* current = head;
    Node* previous = NULL;

    while (current != NULL && current->data.id != targetId) {
        previous = current;
        current = current->next;
    }

    if (current == NULL) {
        printString("ERR: NOT_FOUND\n");
        return;
    }

    if (previous == NULL) {
        head = current->next;
        if (head == NULL) tail = NULL;
    } else {
        previous->next = current->next;
        if (current->next == NULL) tail = previous;
    }

    free(current); // Pengganti fungsi delete
    syncListToEEPROM();
    printString("ACK_DEL\n");
}

void updateItem(uint8_t targetId, uint8_t qTer, uint8_t qDip, uint8_t qRus) {
    Node* current = head;

    while(current != NULL) {
        if(current->data.id == targetId) {
            current->data.qtyTersedia = qTer;
            current->data.qtyDipinjam = qDip;
            current->data.qtyRusak = qRus;

            syncListToEEPROM();
            printString("ACK_UPDATE\n");
            return;
        }
        current = current->next;
    }
    printString("ERR: NOT_FOUND\n");
}

// ==========================================
// SERIAL I/O & PARSING
// ==========================================
void printInventory(void) {
    Node* current = head;
    if (current == NULL) {
        printString("EMPTY\n");
        return;
    }

    char nameBuf[9];
    char picBuf[4];

    while (current != NULL) {
        unpackItemName(&(current->data), nameBuf);
        unpackItemPIC(&(current->data), picBuf);

        printInt(current->data.id); printString(",");
        printString(nameBuf); printString(",");
        printInt(current->data.category); printString(",");
        printInt(current->data.location); printString(",");
        printInt(current->data.qtyTersedia); printString(",");
        printInt(current->data.qtyDipinjam); printString(",");
        printInt(current->data.qtyRusak); printString(",");
        printString(picBuf); printString("\n");

        current = current->next;
    }
    printString("END\n");
}

void parseAndAddCommand(char* commandString) {
    char* token = strtok(commandString, ",");
    if (token == NULL) return;

    token = strtok(NULL, ","); uint8_t id = atoi(token);
    token = strtok(NULL, ","); char* name = token;
    token = strtok(NULL, ","); uint8_t cat = atoi(token);
    token = strtok(NULL, ","); uint8_t loc = atoi(token);
    token = strtok(NULL, ","); uint8_t qTer = atoi(token);
    token = strtok(NULL, ","); uint8_t qDip = atoi(token);
    token = strtok(NULL, ","); uint8_t qRus = atoi(token);
    token = strtok(NULL, ","); char* pic = token;

    ItemData newItem;
    packItemData(&newItem, id, name, cat, loc, qTer, qDip, qRus, pic);
    insertItem(newItem);
}

// berubah!
// Pengganti readBytesUntil manual
void processSerialInput(void) {
    static char buffer[64];
    static uint8_t index = 0;

    uint8_t available = 0;
    dataAvailableUART(&available);

    if (available) {
        char c;
        receiveUART(&c);
        
        if (c == '\n') {
            buffer[index] = '\0'; // Tutup string
            
            if (strncmp(buffer, "ADD", 3) == 0) {
                parseAndAddCommand(buffer);
            } 
            else if (strncmp(buffer, "DEL", 3) == 0) {
                char* token = strtok(buffer, ",");
                token = strtok(NULL, ",");
                if (token != NULL) deleteItem(atoi(token));
            }
            else if (strncmp(buffer,"UPDATE",6) == 0) {
                char* token = strtok(buffer,",");
                token = strtok(NULL,","); uint8_t id = atoi(token);
                token = strtok(NULL,","); uint8_t qTer = atoi(token);
                token = strtok(NULL,","); uint8_t qDip = atoi(token);
                token = strtok(NULL,","); uint8_t qRus = atoi(token);
                updateItem(id,qTer,qDip,qRus);
            }
            else if (strncmp(buffer, "GET_ALL", 7) == 0) {
                printInventory();
            }
            
            index = 0; // Reset buffer untuk perintah selanjutnya
        } 
        else if (index < 63) {
            buffer[index++] = c;
        }
    }
}