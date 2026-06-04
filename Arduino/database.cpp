#include "database.h"

Node* head = NULL;
Node* tail = NULL;
// Update the maximum limit based on the new 12-byte size
const int MAX_ITEMS = 85; 
const int ITEM_SIZE = sizeof(ItemData); // Now automatically 12 bytes

// ==========================================
// BIT-LEVEL COMPRESSION & DECOMPRESSION
// ==========================================

void encodeChar6Bit(char inChar, uint8_t* outByte) {
    if (inChar >= 'A' && inChar <= 'Z') *outByte = inChar - 'A';
    else if (inChar >= 'a' && inChar <= 'z') *outByte = (inChar - 'a') + 26;
    else if (inChar >= '0' && inChar <= '9') *outByte = (inChar - '0') + 52;
    else *outByte = 62; // Space/Unknown
}

void decodeChar6Bit(uint8_t inByte, char* outChar) {
    if (inByte <= 25) *outChar = inByte + 'A';
    else if (inByte <= 51) *outChar = (inByte - 26) + 'a';
    else if (inByte <= 61) *outChar = (inByte - 52) + '0';
    else *outChar = ' ';
}

void encodeChar5Bit(char inChar, uint8_t* outByte) {
    if (inChar >= 'A' && inChar <= 'Z') *outByte = inChar - 'A';
    else *outByte = 26; // Space/Unknown
}

void decodeChar5Bit(uint8_t inByte, char* outChar) {
    if (inByte <= 25) *outChar = inByte + 'A';
    else *outChar = ' ';
}

// UPDATED: Pack the three new quantities into the struct
void packItemData(ItemData* item, uint8_t id, const char* name, uint8_t cat, uint8_t loc, uint8_t qTer, uint8_t qDip, uint8_t qRus, const char* picStr) {
    item->id = id;
    item->category = cat;
    item->location = loc;
    item->qtyTersedia = qTer;
    item->qtyDipinjam = qDip;
    item->qtyRusak = qRus;

    // Pack 8-character Name
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

    // Pack 3-character PIC into 15 bits
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

void initDatabase() {
    head = NULL;
    tail = NULL;
    
    for (int i = 0; i < MAX_ITEMS; i++) {
        ItemData temp;
        EEPROM.get(i * ITEM_SIZE, temp);
        
        if (temp.id != 127) {
            Node* newNode = new Node;
            newNode->data = temp;
            newNode->next = NULL;

            if (head == NULL) {
                head = newNode;
                tail = newNode; // First item is both head and tail
            } else {
                tail->next = newNode; // Attach directly to the tail in O(1) time
                tail = newNode;       // Update the tail pointer
            }
        }
    }
}

void insertItem(ItemData newData) {
    Node* newNode = new Node;
    newNode->data = newData;
    newNode->next = NULL;

    // Case 1: Empty List
    if (head == NULL) {
        head = newNode;
        tail = newNode;
    } 
    // Case 2: O(1) Insertion at the Tail
    else {
        tail->next = newNode; // Point the current tail to the new node
        tail = newNode;       // Make the new node the official tail
    }

    syncListToEEPROM();
    Serial.println("ACK_ADD");
}

void deleteItem(uint8_t targetId) {
    if (head == NULL) {
        Serial.println("ERR: EMPTY");
        return;
    }

    Node* current = head;
    Node* previous = NULL;

    // Traverse to find the ID
    while (current != NULL && current->data.id != targetId) {
        previous = current;
        current = current->next;
    }

    if (current == NULL) {
        Serial.println("ERR: NOT_FOUND");
        return;
    }

    // Case 1: Deleting the FIRST node (Head)
    if (previous == NULL) {
        head = current->next;
        if (head == NULL) {
            tail = NULL; // If we deleted the only item, list is now empty
        }
    } 
    // Case 2 & 3: Deleting the MIDDLE or LAST node
    else {
        previous->next = current->next;
        
        // CRITICAL: If we just deleted the very last node, we MUST update the tail pointer
        if (current->next == NULL) {
            tail = previous;
        }
    }

    delete current; // Free SRAM
    syncListToEEPROM();
    Serial.println("ACK_DEL");
}

void updateItem(uint8_t targetId,uint8_t qTer,uint8_t qDip,uint8_t qRus)
{
    Node* current = head;

    while(current != NULL)
    {
        if(current->data.id == targetId)
        {
            current->data.qtyTersedia = qTer;
            current->data.qtyDipinjam = qDip;
            current->data.qtyRusak = qRus;

            syncListToEEPROM();

            Serial.println("ACK_UPDATE");

            return;
        }

        current = current->next;
    }

    Serial.println("ERR: NOT_FOUND");
}

void syncListToEEPROM() {
    // Overwrite entire EEPROM with 127 (Empty State)
    for (int i = 0; i < EEPROM.length(); i++) {
        EEPROM.write(i, 255); 
    }

    // Dump Linked List back into sequential EEPROM array
    Node* current = head;
    int index = 0;
    while (current != NULL && index < MAX_ITEMS) {
        EEPROM.put(index * ITEM_SIZE, current->data);
        current = current->next;
        index++;
    }
}

// ==========================================
// SERIAL I/O & PARSING
// ==========================================

// UPDATED: Print the three quantities instead of status
void printInventory() {
    Node* current = head;
    if (current == NULL) {
        Serial.println("EMPTY");
        return;
    }

    char nameBuf[9];
    char picBuf[4];

    while (current != NULL) {
        unpackItemName(&(current->data), nameBuf);
        unpackItemPIC(&(current->data), picBuf);

        Serial.print(current->data.id); Serial.print(",");
        Serial.print(nameBuf); Serial.print(",");
        Serial.print(current->data.category); Serial.print(",");
        Serial.print(current->data.location); Serial.print(",");
        Serial.print(current->data.qtyTersedia); Serial.print(",");
        Serial.print(current->data.qtyDipinjam); Serial.print(",");
        Serial.print(current->data.qtyRusak); Serial.print(",");
        Serial.println(picBuf);

        current = current->next;
    }
    Serial.println("END");
}

// UPDATED: Parse the new string format (ADD,ID,Name,Cat,Loc,Ter,Dip,Rus,PIC)
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

void processSerialInput() {
    if (Serial.available() > 0) {
        char buffer[64];
        size_t len = Serial.readBytesUntil('\n', buffer, sizeof(buffer) - 1);
        buffer[len] = '\0'; 

        if (strncmp(buffer, "ADD", 3) == 0) {
            parseAndAddCommand(buffer);
        } 
        else if (strncmp(buffer, "DEL", 3) == 0) {
            char* token = strtok(buffer, ",");
            token = strtok(NULL, ",");
            deleteItem(atoi(token));
        }
        else if (strncmp(buffer,"UPDATE",6) == 0) {
            char* token;

            token = strtok(buffer,",");
            token = strtok(NULL,",");
            uint8_t id = atoi(token);

            token = strtok(NULL,",");
            uint8_t qTer = atoi(token);

            token = strtok(NULL,",");
            uint8_t qDip = atoi(token);

            token = strtok(NULL,",");
            uint8_t qRus = atoi(token);

            updateItem(id,qTer,qDip,qRus);
        }
        else if (strncmp(buffer, "GET_ALL", 7) == 0) {
            printInventory();
        }
    }
}