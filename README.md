# YAWiiBB - Yet Another Wii Balance Board Driver

YAWiiBB is a simple C library to use the Wii Balance Board without the Wii console, specifically for physiotherapy exercises. Initially, I was excited about projects like [namacha's wiiboard](https://github.com/namacha/wiiboard) and [Pierrick Koch's wiiboard](https://github.com/PierrickKoch/wiiboard). However, since `pyBluez` seems no longer maintained, I couldn’t run a Python solution with Python 3.

Instead of dealing with heavy C/C++ drivers, which were too complex, I created my own lightweight C library that fulfills my requirements. YAWiiBB can be started as a Python subprocess and reliably provides sensor data from the Balance Board without unnecessary complexity.

The library is licensed under the GNU General Public License and is not affiliated with Nintendo. I am developing this project independently, using only the hardware.


> **Important Notice:** 
> ## Read first
>Before diving into this project, please take a moment to check out [keldu/WiiWeight](https://github.com/keldu/WiiWeight). The author has developed a significantly more efficient method to extract data from the Wii Balance Board.
>
> ### Why You Should Consider This
>
> The solution provided by **keldu** leverages the internal Linux Bluetooth stack to handle all the necessary pairing and data extraction from the Wii Balance Board. This means you won't have to deal with low-level Bluetooth communication or the complexities involved in parsing raw data from the board.
>
> ### Technical Details
>
> - **Bluetooth Management**: The Linux Bluetooth subsystem provides a robust framework for managing Bluetooth devices. It handles device discovery, pairing, and connection management seamlessly, allowing you to focus on higher-level functionality.
>
> - **Data Extraction**: Instead of manually reading raw data from the input device, keldu's implementation uses the built-in Bluetooth support in Linux. This ensures that all relevant data from the Wii Balance Board is captured effectively without requiring extensive additional code.
>
> - **Ease of Use**: By utilizing an established library, you can save a significant amount of development time and reduce the complexity of your project. This means you can quickly move on to implementing features rather than dealing with the intricacies of device communication.
>
> ### Conclusion
>
> In summary, if you're considering a project involving the Wii Balance Board, it's highly recommended to explore keldu's work. This approach can save you time, effort, and potentially lead to a more stable and reliable implementation. Don’t reinvent the wheel; leverage existing solutions for optimal results!


# You Have Been Warned: Continuing Will Lead You Into a World of Headaches


## Function Overview

YAWiiBB can:
- Discover and connect to the Balance Board via Bluetooth
- Handle communication and data transfer via L2CAP
- Read raw sensor data and calculate weight from these data
- Process calibration data
- Configure extended output and debugging through the `YAWIIBB_EXTENDED` flag

See also [Doxygen Documentation](https://niederschelden.github.io/YAWiiBBD/files.html).

### Main Features

- **Flexibility**: Extended debug mode can be activated via `YAWIIBB_EXTENDED`
- **Calibration**: Utilizes and processes calibration data for more accurate measurements
- **Multi-level Logging**: Additional log levels (RAW, DECODE, DEBUG, VERBOSE) when `YAWIIBB_EXTENDED` is active

## Installation and Requirements

### Requirements

- **Operating System**: Linux with Bluetooth support
- **Libraries**: `bluetooth` and `bluetooth-dev` (for HCI and L2CAP communication)
- **Compiler**: Supports `gcc` and `clang`

### Install Required Packages on Ubuntu/Debian:

```bash
sudo apt-get update
sudo apt-get install bluez libbluetooth-dev build-essential
```

## Installation

### Clone the Repository:

```bash
git clone https://github.com/niederschelden/YAWiiBBD
cd YAWiiBB
```

### Compile the Code:

```bash
gcc -Wall -o YAWiiBBD YAWiiBBD.c YAWiiBBessentials.c -lbluetooth
```

or alternatively with extensions:

```bash
gcc -Wall -o YAWiiBBD YAWiiBBD.c YAWiiBBessentials.c -lbluetooth -DYAWIIBB_EXTENDED
```

## Execution
Put the Balance Board in pairing mode, but do not pair it yet.
```bash
./YAWiiBBD
```
Exit by pressing Enter or pressing the main button.
At the end, a prompt will show how to skip the board search by entering the correct MAC address.

## Byte Mapping in the Data Stream

### Byte Positions of Sensor Data (when byte[1] == 0x32)

| Sensor Position   | Bytes | Description                   |
|-------------------|-------|-------------------------------|
| Button            | 3     | 0x08 == pressed               |
| Top-Right (TR)    | 4-5   | Raw data from top-right sensor|
| Bottom-Right (BR) | 6-7   | Raw data from bottom-right sensor |
| Top-Left (TL)     | 8-9   | Raw data from top-left sensor |
| Bottom-Left (BL)  | 10-11 | Raw data from bottom-left sensor|

### Byte Positions for Calibration Data (when byte[1] == 0x21)

| Calibration Level          | Top-Right (TR) | Bottom-Right (BR) | Top-Left (TL) | Bottom-Left (BL) |
|----------------------------|----------------|--------------------|---------------|-------------------|
| 0 kg (first data packet)   | 7-8            | 9-10              | 11-12         | 13-14            |
| 17 kg (first data packet)  | 15-16          | 17-18             | 19-20         | 21-22            |
| 34 kg (second data packet) | 7-8            | 9-10              | 11-12         | 13-14            |

### Byte Positions for Calibration Data (when byte[1] == 0x20)

Battery Status: The battery status values are coded in bytes 7(-8?) and based on 200 (e.g., 100 = 50%).
LED Status: I believe it’s coded in byte 4 and possibly also in byte 5.

## Wii Balance Board Commands

The following commands are used to communicate with the Wii Balance Board. These commands were adapted from the [WiiBrew website](https://wiibrew.org/wiki/Wii_Balance_Board#Wii_Initialisation_Sequence) and from the Python scripts mentioned above. Not all commands are required to receive data from the board. Some commands can be deactivated in the `main` function by setting their respective flags to `false` (except LED, where the logic is reversed: `false` means it’s turned on via `led_on_command`):

```Markdown
WiiBalanceBoard board = {
    .needStatus = true,
    .needCalibration = true,
    .needActivation = true,
    .led = false,
    .needDumpStart = true,
    .is_running = true
};
```

### Commands

Status Command:
```Markdown
status_command[] = { 0x52, 0x12, 0x00, 0x32 };
```
Activation Command:
```Markdown
activate_command[] = { 0x52, 0x13, 0x04 };
```
Calibration Command:
```Markdown
calibration_command[] = { 0x52, 0x17, 0x04, 0xa4, 0x00, 0x24, 0x00, 0x18 };
```
Toggle LED:
```Markdown
led_on_command[] = { 0x52, 0x11, 0x10 };
```
Data Dump Command:
```Markdown
data_dump_command[] = { 0x52, 0x15, 0x00, 0x32 };
```

### Notes

Most of these commands are not strictly necessary for data reception, but are included in the code since the exact purpose of some commands may be unclear. Only activate commands essential for your specific requirements.

### Adjusting the Log Level

The log level is controlled via `LogLevel`. By default, RAW is active; with `YAWIIBB_EXTENDED`, additional levels (DECODE, DEBUG, VERBOSE) are available.


To adjust the output level, set the desired option in YAWiiBBD.c before compiling:

```c

#ifdef YAWIIBB_EXTENDED
const LogLevel debug_level = DEBUG; // Options: RAW, DECODE, DEBUG, VERBOSE
#else...
```
This way, compiling with the -DYAWIIBB_EXTENDED flag enables additional logging levels.

## Licenses and Disclaimer

This project is licensed under the GNU General Public License v3.0. For further details, see the LICENSE file.

Disclaimer: This software was developed independently and is not affiliated with Nintendo or its subsidiaries. Nintendo holds all rights to the Wii Balance Board, and this software is provided without any liability. Use it at your own risk.

Enjoy YAWiiBB, and I welcome any feedback or suggestions! If you have any questions, feel free to open an issue on the GitHub repository or contact me directly.