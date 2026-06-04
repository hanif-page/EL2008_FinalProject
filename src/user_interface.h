#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Menampilkan menu utama sistem inventarisasi laboratorium
void PrintMainMenu();

// Menerima input pilihan menu dari user (1-7)
void InputChoice(int* choice);

// Meminta input string dari user dengan prompt tertentu
void PromptString(const char* promptText, char* inputBuffer, int maxLength);

// Meminta input integer dari user dengan prompt tertentu
void PromptInt(const char* promptText, int* inputValue);

// Menampilkan header tabel inventaris
void PrintTableHeader();

// Menampilkan footer tabel inventaris
void PrintTableFooter();

#endif
