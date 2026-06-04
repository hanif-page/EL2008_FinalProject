#ifndef SERIAL_COMM_H
#define SERIAL_COMM_H

void initSerialPort(const char* portName);
void sendSerialData(const char* dataStr);
void receiveSerialData(char* buffer);
void closeSerialPort();

void serial_send(const char* dataStr);
int serial_receive_line(char* buffer, int maxLength);

#endif // SERIAL_COMM_H