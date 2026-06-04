#ifndef MOCK_SERIAL_COMM_H
#define MOCK_SERIAL_COMM_H

void initSerialPort(const char* portName);
void sendSerialData(const char* dataStr);
void receiveSerialData(char* buffer);
void closeSerialPort();

#endif