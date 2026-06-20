#include <avr/io.h>
#include "database.h"

int main(void) {
    // Setupa dengan Serial.begin(9600)
    initUART();
    
    // Serupa dengan fungsi setup()
    initDatabase();

    // Serupa dengan fungsi loop()
    while (1) {
        processSerialInput();
    }

    return 0;
}