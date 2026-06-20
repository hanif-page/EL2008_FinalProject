#ifndef DATABASE_H
#define DATABASE_H

#include <stdint.h>

#define MAX_ITEMS 85
#define ITEM_SIZE 12

// 96-BIT (12-BYTE) STRUCTURE
struct __attribute__((packed)) ItemData {
    uint8_t id : 7;

    uint8_t pName0 : 6;
    uint8_t pName1 : 6;
    uint8_t pName2 : 6;
    uint8_t pName3 : 6;
    uint8_t pName4 : 6;
    uint8_t pName5 : 6;
    uint8_t pName6 : 6;
    uint8_t pName7 : 6;          

    uint8_t category : 2;
    uint8_t location : 6;        
    
    uint8_t qtyTersedia : 6;     
    uint8_t qtyDipinjam : 6;     
    uint8_t qtyRusak : 6;        
    
    uint16_t pic : 15;           
};

typedef struct ItemData ItemData;

// LINKED LIST NODE
struct Node {
    ItemData data;
    struct Node* next;
};
typedef struct Node Node;

// GLOBAL VARIABLES
extern Node* head;
extern Node* tail;

// FUNCTION PROTOTYPES
void initUART(void);
void printString(const char* str);
void initDatabase(void);
void processSerialInput(void);

void encodeChar6Bit(char inChar, uint8_t* outByte);
void decodeChar6Bit(uint8_t inByte, char* outChar);
void encodeChar5Bit(char inChar, uint8_t* outByte);
void decodeChar5Bit(uint8_t inByte, char* outChar);

void packItemData(ItemData* item, uint8_t id, const char* name, uint8_t cat, uint8_t loc, uint8_t qTer, uint8_t qDip, uint8_t qRus, const char* pic);
void unpackItemName(ItemData* item, char* outBuffer);
void unpackItemPIC(ItemData* item, char* outBuffer);

void insertItem(ItemData newData);
void deleteItem(uint8_t targetId);
void updateItem(uint8_t targetId, uint8_t qTer, uint8_t qDip, uint8_t qRus);
void syncListToEEPROM(void);
void printInventory(void);
void parseAndAddCommand(char* commandString);

#endif