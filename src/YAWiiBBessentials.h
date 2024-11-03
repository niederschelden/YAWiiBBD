#ifndef YAWIIBBESSENTIALS_H
#define YAWIIBBESSENTIALS_H
//#define YAWIIBB_EXTENDED


/**
 * @file YAWiiBBessentials.h
 * @brief Header file for core definitions and utility functions used in the YAWiiBB project.
 *
 * This header file provides essential definitions, data structures, and function declarations 
 * that support core functionality for the YAWiiBB (Yet Another Wii Balance Board Driver) project. 
 * It serves as a foundational component, defining the main interfaces and constants required 
 * for interacting with a Wii Balance Board over Bluetooth.
 *
 * ## Key Components
 * - **Data Structures**: Contains the primary data structures for representing the state and 
 *   configuration of the Wii Balance Board, including fields for sensor data, calibration 
 *   parameters, and device status.
 * - **Constants and Macros**: Defines constants, macros, and flag values used throughout the 
 *   project to standardize operations, such as status flags and timing parameters.
 * - **Utility Functions**: Declares utility functions for tasks like initializing the board, 
 *   processing sensor data, and managing connection status. These functions are designed 
 *   to be called within the main application loop and support modular expansion.
 *
 * ## Essential vs. Extended Functionality
 * Originally, the project aimed to split core (`essentials`) and extended (`extended`) 
 * functionality across separate files. However, as an intermediate programmer, I encountered 
 * challenges in managing memory access violations when handling this separation. After 
 * considerable testing, I integrated the extended functionality directly within `YAWiiBBessentials.h`, 
 * within an `EXTENDED` section, for stability and to simplify memory management.
 *
 * This integrated approach ensures that both core and additional features operate seamlessly, 
 * although it may require restructuring in the future if modularity is prioritized or 
 * memory handling becomes more robust.
 *
 * ## Dependencies
 * This file depends on the `bluez` library for Bluetooth communication, which provides 
 * the essential functions and structures required to connect and interact with the Wii 
 * Balance Board. Ensure the following libraries are available and correctly linked:
 * - `bluez` (Bluetooth library for Linux, required for L2CAP connections)
 * - Standard C libraries (e.g., `stdlib.h`, `stdio.h`, `unistd.h`) for basic operations and 
 *   system-level interactions.
 * 
 * ## Usage Notes
 * This file should be included in any source files that need direct access to the core 
 * functionality of YAWiiBB. By isolating these definitions and functions, 
 * `YAWiiBBessentials.h` promotes modularity and ensures that key components of the application 
 * can be maintained and extended independently.
 *
 * ## Future Extensions
 * - **Stronger Use of Pointers**: Future revisions could aim to reduce variable passing and 
 *   further rely on pointers to optimize performance and minimize memory allocation issues. 
 *   Currently, variable passing has been used where pointer implementation led to compilation 
 *   errors or memory access issues, but transitioning to pointers where possible could 
 *   simplify the function calls and improve runtime efficiency.
 * - **Modular Separation**: Revisit the combined structure for `essentials` and `extended` 
 *   functionality, separating them into distinct files once memory handling and stability 
 *   are manageable, allowing for a clearer modular structure.
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h> 
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <pthread.h>
#include <ctype.h>

#define WII_BALANCE_BOARD_ADDR "00:23:CC:43:DC:C2"  /**< Default MAC address for the Wii Balance Board */
#define BUFFER_SIZE 24  /**< Buffer size for data reception  - for the Wii Balance Board 24 byte is enough*/

/** 
 * @enum LogLevel
 * @brief Specifies the level of detail for logging output.
 *
 * This enumeration allows the user to choose between different verbosity
 * levels when displaying information.
 */
typedef enum { 
    RAW,     /**< Outputs raw data as received without interpretation */
    #ifdef YAWIIBB_EXTENDED
    DECODE,  /**< Outputs big endian converted value of two bytes, readings in gramm */
    DEBUG,   /**< Provides debugging information and readings in Kilo */
    VERBOSE  /**< currently unused */
    #endif //YAWIIBB_EXTENDED
} LogLevel;

extern const LogLevel debug_level;

/**
 * @brief Buffer for storing responses from the Wii Balance Board.
 *
 * This buffer holds incoming data from the Balance Board, with a fixed maximum 
 * length of bytes, as defined in BUFFER_SIZE
 */
extern unsigned char buffer[BUFFER_SIZE];

/**
 * @defgroup CommandDefinitions Command Definitions
 * @brief Command arrays for interacting with the Wii Balance Board based on the Wiimote protocol.
 *
 * This section contains predefined command arrays that follow the Wiimote protocol structure, 
 * as outlined in Wiibrew documentation. Each command array begins with the byte `0x52`, which 
 * addresses the Wii Balance Board, followed by specific instructions to execute various functions 
 * (e.g., status request, activation, LED control, calibration, and continuous data reporting).
 *
 * ## Command Structure
 * Each command array uses a consistent structure:
 * - `0x52` as the first byte to address the Wii Balance Board.
 * - A second byte that specifies the command function (e.g., `0x12` for status request).
 * - Additional bytes as required by each function, providing parameters or configuration settings.
 * 
 * ## Command Arrays
 * - **Status Request** (`status_command`): Retrieves the current status of the Balance Board, such 
 *   as connection state and battery level.
 *   - `const unsigned char status_command[] = { 0x52, 0x12, 0x00, 0x32 };`
 *   - Structure: `0x52` (address) | `0x12` (status command) | `0x00` (reserved) | `0x32` (end byte).
 *   - **Note**: The `0x32` byte at the end may act as a terminator, checksum byte, or something else I do not fully understand.

 * - **Activation Command** (`activate_command`): Activates the Balance Board sensors, preparing the 
 *   board to send data.
 *   - `const unsigned char activate_command[] = { 0x52, 0x13, 0x04 };`
 *   - Structure: `0x52` (address) | `0x13` (activate command) | `0x04` (activation flag).
 *   - **Note**: The role of `0x04` as an activation flag is unclear. I included it because some protocols suggest this might act as a "start data" signal. More testing is needed to determine its necessity, but I do not fully understand its purpose.

 * - **Calibration Command** (`calibration_command`): Initiates calibration to capture sensor baselines.
 *   - `const unsigned char calibration_command[] = { 0x52, 0x17, 0x04, 0xa4, 0x00, 0x24, 0x00, 0x18 };`
 *   - Structure: `0x52` (address) | `0x17` (calibration command) | Sequence of bytes for calibration.
 *   - **Note**: The sequence following `0x17` is protocol-defined calibration data, likely setting initial sensor baselines. I copied this from other implementations without fully understanding its function.

 * - **LED Control Command** (`led_on_command`): Controls the LED on the Balance Board. This command 
 *   can toggle the LED based on the specified mask.
 *   - `const unsigned char led_on_command[] = { 0x52, 0x11, 0x10 };`
 *   - Structure: `0x52` (address) | `0x11` (LED command) | `0x10` (LED mask for on state).
 *   - **Note**: The mask `0x10` represents the bit to turn on the single LED on the Balance Board. This part is clear from Wiibrew’s Wiimote documentation.

 * - **Data Dump Command** (`data_dump_command`): Starts continuous data reporting from the Balance Board, 
 *   sending real-time sensor readings.
 *   - `const unsigned char data_dump_command[] = { 0x52, 0x15, 0x00, 0x32 };`
 *   - Structure: `0x52` (address) | `0x15` (data dump command) | `0x00` (reserved) | `0x32` (end byte).
 *   - **Note**: The `0x32` byte here might act as an end marker or handshake byte to confirm readiness for data streaming, or something completely different—I am unsure.
 * 
 * ## Future Extensions
 * Additional commands may be added as necessary to extend functionality. In the future, 
 * command arrays could be dynamically filled based on inputs from `stdin` within a monitoring 
 * thread, allowing real-time command adjustments. For now, the command arrays are statically 
 * defined in `YAWiiBBessentials.c`, ensuring core commands are readily available.
 *
 * @{
 */
extern const unsigned char status_command[];
extern const unsigned char activate_command[];
extern const unsigned char calibration_command[];
extern const unsigned char led_on_command[];
extern const unsigned char data_dump_command[];
/** @} */


/**
 * @struct WiiBalanceBoard
 * @brief Represents the Wii Balance Board connection and status.
 *
 * This structure holds essential information about the Wii Balance Board,
 * including its MAC address, control and interrupt socket descriptors, and
 * a flag indicating the board's active state.
 */

typedef struct {
    char mac[19];                   /**< Bluetooth MAC address of the Wii Balance Board */
    int control_sock;               /**< Socket descriptor for control channel */
    int receive_sock;               /**< Socket descriptor for interrupt (data) channel */
    bool needStatus;                /**< Status request flag */
    bool needActivation;            /**< Calibration request flag */
    bool needCalibration;           /**< Calibration request flag */
    bool led;                       /**< LED Status */
    bool needDumpStart;             /**< Start continuous dump request flag */
    bool is_running;                /**< Flag to indicate if the board is actively running */
    #ifdef YAWIIBB_EXTENDED
    uint16_t calibration[3][4];     /**< Calibration data array */
    #endif //YAWIIBB_EXTENDED
} WiiBalanceBoard;

/**
 * @brief Logs messages with different verbosity levels, providing diagnostics and raw data output.
 *
 * This function outputs messages to the console with optional raw data from the Wii Balance Board.
 * It supports multiple verbosity levels (RAW, DEBUG, and VERBOSE), controlled by the `LogLevel` enum. 
 * The function is essential for debugging and tracking the board's status and output during development.
 *
 * This function's implementation interacts closely with the Wiimote protocol, pushing the limits 
 * of my current understanding of the protocol. Despite attempts to isolate functionality in `essentials`,
 * it remains a complex function with room for refinement.
 * 
 * Potential areas for improvement include:
 * - **Consistent use of pointers**: Aiming to rely on pointers wherever possible to reduce stack usage 
 *   and optimize memory management.
 * - **Modularizing verbosity level handling**: Simplifying logic for handling different verbosity levels, 
 *   potentially through helper functions to make the code more manageable.
 *
 * @param level Pointer to a `LogLevel` enum indicating the verbosity level (RAW, DEBUG, VERBOSE).
 * @param message Constant character string representing the message to be logged.
 * @param buffer Optional byte array containing raw data to display if needed. This parameter 
 *               is processed only when provided (not NULL).
 * @param length Length of the byte array (ignored if buffer is NULL).
 * @param board Pointer to the `WiiBalanceBoard` structure for accessing board-specific data.
 */
void print_info(const LogLevel* is_debug_level, const char* message, const unsigned char* buffer, int length, const WiiBalanceBoard* board);
/**
 * @defgroup EssentialFunctions Essential Functions
 * @brief Core functions necessary for basic interaction with the Wii Balance Board.
 * 
 * This group includes the primary functions needed to find, connect, and 
 * send commands to the Wii Balance Board. These functions form the foundation 
 * of the YAWiiBB project and enable essential Bluetooth communication and control.
 * @{
 */
/**
 * @brief Finds the Wii Balance Board by scanning nearby Bluetooth devices.
 * 
 * Requires the device to be in pairing mode and not jet paired
 *
 * This function scans for Bluetooth devices in the area and identifies the Wii
 * Balance Board based on its specific name. If found, the board’s MAC address
 * is stored directly in the @p board structure.
 *
 * @param board Pointer to WiiBalanceBoard structure where the MAC address is stored.
 * @return 0 on success (board found), -1 if not found.
 */
int find_wii_balance_board(WiiBalanceBoard* board);

/**
 * @brief Sends a command to the Wii Balance Board over the control socket.
 *
 * This function transmits a command to the board using the specified socket.
 * If an error occurs, the program will exit with a failure status.
 *
 * @param sock Integer control socket descriptor.
 * @param command Byte array of command data to be sent.
 * @param length Integer representing the length of the command array.
 */
void send_command(int sock, const unsigned char* command, int length);

/**
 * @brief Establishes a L2CAP connection with the Wii Balance Board.
 *
 * This function creates an L2CAP Bluetooth socket and connects to the specified
 * service (PSM) on the Wii Balance Board using its MAC address. It returns the
 * socket descriptor or exits on failure.
 *
 * @param bdaddr_str Constant character string representing the Bluetooth MAC address.
 * @param psm Integer specifying the Protocol/Service Multiplexer (PSM) channel.
 * @return Socket descriptor on success, exits program on failure.
 */
int connect_l2cap(const char* bdaddr_str, uint16_t psm);
/** @} */
/**
 * @defgroup CommandHandlers Command Handlers
 * @brief Functions for handling specific commands to the Wii Balance Board.
 * 
 * These functions individually handle specific commands for the Wii Balance Board, such as 
 * status checks, calibration, LED control, and activation. Each function is called 
 * when its corresponding flag in the `WiiBalanceBoard` structure is set.
 * 
 * While these commands are currently implemented as separate functions, they could 
 * potentially be unified into a single, generic `handle_send_command` function in 
 * the future. This separation was initially implemented to manage any specific 
 * cases or exceptions as outlined in `Wiimote` on `Wiibrew`, which I must admit, I did not fully understand. 
 * 
 * If it continues to be straightforward, these functions can later be consolidated 
 * to streamline the code and remove redundancy.
 * @{
 */

/**
 * @brief Processes sending a status command to the Wii Balance Board.
 * 
 * This function is called when the `needStatus` flag is set, 
 * and sends the corresponding status command to the board. 
 * 
 * @param board Pointer to the WiiBalanceBoard structure that holds the current status.
 */
void handle_status(WiiBalanceBoard* board);

/**
 * @brief Processes sending a calibration command to the Wii Balance Board.
 * 
 * This function is called when the `needCalibration` flag is set, 
 * and sends the corresponding calibration command to the board.
 * 
 * @param board Pointer to the WiiBalanceBoard structure that holds the current status.
 */
void handle_calibration(WiiBalanceBoard* board);

/**
 * @brief Processes sending an LED on command to the Wii Balance Board.
 * 
 * This function is called when the LED flag `led` is not set,
 * and turns on the board's LED.
 * 
 * @param board Pointer to the WiiBalanceBoard structure that holds the current status.
 */
void handle_led_on(WiiBalanceBoard* board);

/**
 * @brief Processes sending an activation command to the Wii Balance Board.
 * 
 * This function is called when the `needActivation` flag is set, 
 * and sends the corresponding activation command to the board.
 * 
 * @param board Pointer to the WiiBalanceBoard structure that holds the current status.
 */
void handle_activation(WiiBalanceBoard* board);

/**
 * @brief Processes sending a data dump command (continous report of readings) to the Wii Balance Board.
 * 
 * This function is called when the `needDumpStart` flag is set, 
 * and starts the continuous transmission of board data.
 * 
 * @param board Pointer to the WiiBalanceBoard structure that holds the current status.
 */
void handle_data_dump(WiiBalanceBoard* board);
/** @} */

/**
 * @brief Processes received data from the Wii Balance Board.
 * 
 * This function checks the received data and evaluates it. If
 * a certain condition is met (e.g., the receive code indicates an error),
 * the `is_running` status is set to `false`.
 * 
 * @param bytes_read Number of bytes read in the `buffer`.
 * @param buffer     Buffer containing the received data.
 * @param board      Pointer to the WiiBalanceBoard structure that holds the current status.
 */
void process_received_data(int bytes_read, unsigned char* buffer, WiiBalanceBoard* board);

/**
 * @brief Thread function for monitoring user input to control the Wii Balance Board.
 *
 * Waits for user input via the console or the power button of the board. If the user presses the button or the enter key
 * without additional characters, the function sets the `is_running` flag of the `WiiBalanceBoard`
 * object to `false` and exits the thread.
 *
 * @param arg A void pointer to the `WiiBalanceBoard` object passed to the function
 *            and called at runtime to change the status.
 * @return Always `NULL` – indicates that the thread has terminated.
 */
void* threadFunction(void* arg);

/**
 * @brief Creates a new thread and starts the `threadFunction` to monitor user control.
 *
 * This function creates a new thread and instructs it to execute the `threadFunction`.
 * In case of errors, an error message is displayed, the `is_running` flag of the Wii Balance Board is
 * set to `false`, and the program exits with an error code.
 *
 * @param board Pointer to the `WiiBalanceBoard` object monitored by the threadFunction.
 * @param threadId Pointer to the `pthread_t` variable where the ID of the new thread will be stored.
 */
void createThread(WiiBalanceBoard* board, pthread_t* threadId);

/**
 * @brief Validates a given MAC address for format and content.
 *
 * Checks whether a valid MAC address has been passed as a single argument.
 * The address must be exactly 17 characters long and conform to the format `XX:XX:XX:XX:XX:XX`,
 * where `X` is a hexadecimal character (`0-9`, `A-F`, `a-f`). The function returns
 * `1` if the input meets the requirements; otherwise, it returns `0`.
 * 
 * In case of an invalid format or erroneous inputs, specific error messages
 * are generated to indicate possible causes.
 *
 * @param argc Number of arguments passed to the program.
 * @param argv Array of arguments, where the second (`argv[1]`) should be the MAC address.
 * @return `1` if the MAC address is valid; otherwise, `0`.
 */
int is_valid_mac(int argc, char *argv[]);



#ifdef YAWIIBB_EXTENDED
/**
 * @defgroup ExtendedProcessing Extended Data Processing
 * @brief Functions for interpreting and displaying Wii Balance Board data.
 *
 * These functions provide additional capabilities for interpreting and converting 
 * raw data from the Wii Balance Board into a human-readable format. While not essential 
 * for the primary operation, they enable a deeper understanding of data processing 
 * and assist in debugging.
 * 
 * Currently, these functions accept variables in some cases, though the goal is to 
 * refactor them to exclusively use pointers where possible to streamline memory 
 * management and improve efficiency. Additionally, they would ideally be organized 
 * in a separate file, but due to challenges in managing memory access and avoiding 
 * segmentation faults, they remain integrated within this file for now.
 * 
 * @note To activate these extended features, compile with the `YAWIIBB_EXTENDED` flag.
 *   @code
 *   gcc -DYAWIIBB_EXTENDED -Wall -o complex YAWiiBBD.c YAWiiBBessentials.c -lbluetooth
 *   @endcode
 * @{
 */

/**
 * @brief Processes the calibration data from the Wii Balance Board.
 *
 * This function analyzes a data buffer with calibration information 
 * and extracts this into the calibration array of the passed 
 * `WiiBalanceBoard` instance. The calibration data are 16-bit values 
 * stored in a big-endian format and consist of 4 pairs 
 * for each of the four corners (topright, topleft, bottomright, bottomleft).
 *
 * @param bytes_read Number of bytes actually read in the buffer.
 * @param buffer Pointer to a buffer containing the received bytes. 
 *               The expected structure is:
 *               - Bytes 7-14: Calibration set 0 (4 pairs, 8 bytes)
 *               - Bytes 15-22: Calibration set 1 (4 pairs, 8 bytes)
 *               - If Byte 15 == 0x00:
 *                 - Bytes 7-14: Calibration set 1
 *                 - Bytes 15-22: Calibration set 2
 * @param board Pointer to the `WiiBalanceBoard` instance where calibration data will be stored.
 *
 * @note This function assumes that the `buffer` contains at least 
 *       23 bytes in order to process the expected calibration data.
 *
 * @details 
 * The operation of the function is as follows:
 * 1. Check if the second calibration packet has been received 
 *    (Byte 15 == 0x00).
 * 2. If the second packet is not present:
 *    - Bytes 7-14 are stored in `calibration[0]` (Calibration set 0).
 *    - Bytes 15-22 are stored in `calibration[1]` (Calibration set 1).
 * 3. If the second packet is present:
 *    - Bytes 7-14 are stored in `calibration[2]` (Calibration set 2).
 *
 * Example:
 * - Received data as an example:
 *   - buffer[7] = 0x01, buffer[8] = 0x02 (Calibration 0, Pair 0)
 * In this case, the function would store the values in `board->calibration[0][0]` 
 *   - buffer[15] = 0x00 (indicates that this is packet 2)
 * In this case, the function would only read bytes 7-8,[..],13-14 and store them in `board->calibration[2][i]`
 */
void process_calibration_data(int* bytes_read, unsigned char* buffer, WiiBalanceBoard* board);


/**
 * @brief Converts two consecutive bytes in the buffer from Big-Endian representation to a decimal integer.
 * 
 * Interprets two bytes in the given `buffer` starting from the provided `position` as a
 * 16-bit integer in Big-Endian format (most significant byte first). The function returns this
 * integer so that it can be output in decimal form.
 * 
 * @param buffer     Pointer to the buffer that contains the bytes.
 * @param position   The starting position of the most significant byte in the buffer.
 * @return uint16_t  The Big-Endian number interpreted as a decimal integer.
 */
uint16_t bytes_to_int_big_endian(const unsigned char *buffer, size_t position, int* max);

/**
 * @brief Calculates the weight in grams from the raw data of the Wii Balance Board.
 *
 * This function uses calibration data from the Wii Balance Board to calculate 
 * a weight in grams based on the given raw data. The weight 
 * is interpolated within four calibration ranges or, if the value 
 * exceeds the highest calibration range, is linearly extrapolated.
 *
 * @param board A constant pointer to the Wii Balance Board structure that contains
 *              the calibration data.
 * @param raw   The raw data value representing the weight, measured from one
 *              of the four sensor positions (Top-Right, Bottom-Right, etc.).
 * @param pos   The position of the sensor (0 to 3) for the calibration data of the raw value.
 *              Must be within the range [0, 3].
 *
 * @return The calculated weight in grams as `uint16_t`.
 * 
 * @note The function can interpolate weights up to 34 kg with the given 
 *       calibration values. For values above this range, 
 *       the weight is linearly extrapolated.
 *
 * The calculation uses four cases based on the raw data:
 *   - If the raw value is less than the calibration for 0 kg, 0 g is returned.
 *   - If the raw value is between the calibrations for 0 kg and 17 kg, linear 
 *     interpolation occurs between these points.
 *   - If the raw value is between the calibrations for 17 kg and 34 kg, 
 *     interpolation occurs within this range.
 *   - If the raw value is greater than or equal to the calibration for 34 kg, 
 *     linear extrapolation is performed based on the last range.
 */
uint16_t calc_mass(const WiiBalanceBoard* board, uint16_t raw, int pos);

/**
 * @brief Displays the stored calibration data.
 *
 * This function is solely for debugging purposes and is no longer used.
 * If one wants to know whether the calibration data has been stored, they can implement it.
 */
void print_calibration_data(const WiiBalanceBoard* board);
/** @} */
#endif //YAWIIBB_EXTENDED
#endif // YAWIIBBESSENTIALS_H
