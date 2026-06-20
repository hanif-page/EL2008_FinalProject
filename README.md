# Laboratory Inventory Management System

**PROBLEM SOLVING WITH PROGRAMMING (EL2008)**

This system is a C-based *Client-Server* architecture designed to overcome the non-volatile memory resource limitations of the Arduino UNO. The system distributes workload between a user interface running on a computer (PC) and a physical database stored in the Arduino's EEPROM.

---

## Development Team

| Name                        | Student ID |
| :-------------------------- | :--------- |
| Muhammad Ammar Hanif        | 13224087   |
| Maria Vanessa Soritan       | 13224078   |
| Hufnagel Ruben Lenard S.    | 13224015   |
| Christian Jonathan Hutajulu | 13224108   |

---

## Data Structure and Format (Compressed Memory)

The system utilizes *bit-field* manipulation and custom character compression techniques to store inventory records in a highly memory-efficient structure. The following constraints apply to each inventory item:

* **ID:** `0 - 127` (7-bit Integer)
* **Item Name:** Maximum 8 characters. Limited to uppercase letters (A-Z), lowercase letters (a-z), and digits (0-9). Encoded using 6 bits per character.
* **Category:** `0 - 3` (2-bit Integer). Numeric representation:

  * `0` = Microcontroller
  * `1` = Actuator
  * `2` = Sensor
  * `3` = Others
* **Available Quantity:** `0 - 63` (6-bit Integer)
* **Borrowed Quantity:** `0 - 63` (6-bit Integer)
* **Damaged Quantity:** `0 - 63` (6-bit Integer)
* **Item PIC:** Maximum 3 characters. Limited to uppercase letters (A-Z) only. Encoded using 5 bits per character.

> **Note:** The **Item Owner** attribute is intentionally not stored in the hardware database to maximize EEPROM storage capacity.

---

## System Architecture

The project is divided into two main directories that operate independently while communicating through the Serial (UART) protocol.

### 1. `avr_mcu` (Database Server)

Contains the Arduino firmware written entirely in C (*Bare-Metal Programming*). The firmware is responsible for:

* Low-level register-based I/O operations
* EEPROM data storage management
* SRAM-based linked list structures
* UART serial communication
* Processing commands received from the client

### 2. `client_interface_macos` / `client_interface_windows` (User Client)

A terminal-based user interface written entirely in C. The application is responsible for:

* User input validation (*Fail-Fast Validation*)
* Displaying inventory data in tabular format
* Sending commands to the Arduino via UART
* Displaying server responses to the user

---

## Repository Structure

```text
.
├── avr_mcu/
│   ├── src/
│   ├── include/
│   ├── Makefile
│   └── ...
│
├── client_interface_macos/
│   ├── src/
│   ├── include/
│   ├── Makefile
│   └── ...
│
├── client_interface_windows/
│   ├── src/
│   ├── include/
│   ├── Makefile
│   └── ...
|
├── docs/
|   ├── Spesifikasi Tugas Besar PMP.pdf
|   ├── EL2008_LaporanTugasBesar_K03-6.pdf
|   ├── EL2008_SlideTugasBesar_K03-6.pdf
│
└── README.md
```

---

## System Requirements

### Hardware

* Arduino Uno
* USB Data Cable
* macOS Computer

### Software

* GNU Make
* AVR-GCC Toolchain
* AVRDUDE
* macOS Terminal

---

# Compilation and Execution Guide (macOS)

Follow the steps below to run the complete system from the macOS terminal. Ensure that the AVR toolchain (`avr-gcc`, `avrdude`) and GNU Make are installed.

---

## Step 1: Hardware Preparation

1. Connect the Arduino Uno to your Mac using a USB cable.
2. Ensure that the Arduino IDE is **closed** so that the serial port is not occupied by another application.
3. Identify your Arduino serial port by running the following command:

```bash
ls /dev/cu.usbmodem*
```

Example output:

```bash
/dev/cu.usbmodem11301
```

> Copy the detected port name, as it will be used when launching the client application.

---

## Step 2: Upload Firmware to the Arduino (`avr_mcu`)

Open a terminal and navigate to the microcontroller firmware directory:

```bash
cd avr_mcu
make clean
make upload
```

Explanation:

* `make clean` removes previously generated build files.
* `make upload` cross-compiles the firmware and uploads it to the Arduino using AVRDUDE.

Wait until the process completes and the terminal displays a message similar to:

```text
avrdude done. Thank you.
```

Once this message appears, the firmware has been successfully uploaded and the EEPROM database is ready to receive commands from the client.

---

## Step 3: Launch the Client Interface (`client_interface_macos`)

Open a new terminal tab (or return to the project root directory), then navigate to the client directory:

```bash
cd client_interface_macos
make clean
make
```

Explanation:

* `make clean` removes previously generated build files.
* `make` compiles and links all program modules to generate the executable file.

After a successful build and the `inventory` executable has been created, run the program by providing the serial port identified in Step 1:

```bash
./inventory /dev/cu.usbmodem11301
```

Replace `/dev/cu.usbmodem11301` with the serial port detected on your computer.

The application will perform a handshake with the Arduino and display the Main Menu once the connection is successfully established.

---

## Exiting the Program

Use the following menu option:

```text
7. Exit
```

to safely disconnect the serial connection before unplugging the Arduino USB cable.

---

This project was developed for the **EL2008 – Problem Solving With Programming** course at the **Bandung Institute of Technology (Institut Teknologi Bandung)**.
