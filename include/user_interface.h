#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

// --- VOID FUNCTION PROTOTYPES ---
void printMainMenu();
void getUserChoice(int* choice);
void promptString(const char* promptText, char* inputBuffer, int maxLength);
void promptInt(const char* promptText, int* inputValue);
void printTableHeader();
void printTableFooter();

#endif // USER_INTERFACE_H