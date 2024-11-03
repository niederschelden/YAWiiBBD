#include "YAWiiBBessentials.h"
/**
 * @mainpage YAWiiBB Project Documentation
 * 
 * Welcome to the documentation for the YAWiiBB (Yet Another Wii Balance Board Driver) project. 
 * This project was born out of necessity: as a physical therapist, I rely on simple and precise measurements of the body’s center of gravity to support assessments and therapy tracking. However, I cannot justify the five-figure costs for medical products that exceed my requirements and stretch my budget. With the Wii system gradually disappearing from the second-hand market and the Balance Board available for as little as 5-10 € on eBay, this project offers a practical solution for capturing data at a fraction of the cost.
 * 
 * Initially, I explored larger projects like Wiimote and CWii, but they quickly overwhelmed me as an intermediate programmer. Other projects, especially those in Python, also proved challenging due to limited development progress, which prevented me from tailoring them to my needs. Thus, the YAWiiBB project was created.
 * 
 * The primary goal here is straightforward: to read data from the Balance Board via the L2CAP level and output this information as a continuous data stream to stdout. This documentation aims to capture my thought process and invite more skilled developers to understand, improve, and contribute to the project, helping make the Wii Balance Board a simple, accessible tool for body balance measurements.
 * 
 * ## About this Documentation
 * - **Starting Points**: This project is intentionally built in C, not C++. As a result, the main entry point for documentation is found at `files.html`.
 * - **Contribution**: If you're a programmer interested in accessible data streaming from the Wii Balance Board, consider this a foundation you can build upon. Any improvements, optimizations, or suggestions are highly welcome!
 * 
 * Thank you for exploring YAWiiBB! I look forward to any collaborative insights or contributions, making this project stronger together.
 */


/**
 * @file YAWiiBBD.c
 * @brief Core file for the YAWiiBB application, facilitating interaction with a Wii Balance Board.
 *
 * This file contains the main functionality of the application, including 
 * setup for Bluetooth connections, the primary loop for data handling, and 
 * thread management. The compilation flags control debug levels, which influence 
 * logging and application status reporting.
 *
 * The application scans for the Wii Balance Board and attempts to establish 
 * two L2CAP connections: one for command transmission and one for receiving 
 * responses. Once connected, it enters a loop to process and output data 
 * continuously as long as the board remains active.
 * 
 * ### Termination
 * The main loop can be exited by pressing Enter in the terminal or by pressing 
 * the power button on the Wii Balance Board.
 *
 * ## Compilation Instructions
 * Compile the application using the following commands based on the intended configuration:
 * - **Standard Version**: Basic setup for data streaming.
 *   @code
 *   gcc -Wall -o simple YAWiiBBD.c -lbluetooth
 *   @endcode
 * - **Extended Version**: Includes additional features and functions found in `YAWiiBBessentials.c`.
 *   @code
 *   gcc -DYAWIIBB_EXTENDED -Wall -o complex YAWiiBBD.c YAWiiBBessentials.c -lbluetooth
 *   @endcode
 * 
 * @note Ensure all required Bluetooth dependencies are installed and configured 
 * to allow successful connection and data handling from the Wii Balance Board.
 */

#ifdef YAWIIBB_EXTENDED
const LogLevel debug_level = DEBUG; //Options RAW, DECODE, DEBUG, (VERBOSE)
#else
const LogLevel debug_level = RAW; 
#endif // YAWIIBB_EXTENDED


/**
 * @brief Main loop of the application.
 *
 * This function executes the core operations of the application, performing 
 * various actions based on the flags set within the `WiiBalanceBoard` object. 
 * These actions include status checks, calibration, activation, and toggling 
 * the LED on or off. The received data from the Balance Board is processed 
 * and handled within this loop.
 *
 * The loop includes a 10-millisecond delay to reduce CPU load and ensure 
 * efficient processing of data. It runs continuously until a termination 
 * signal is received.
 *
 * @param board A pointer to the `WiiBalanceBoard` object containing current 
 *              status information and control flags.
 */


void main_loop(WiiBalanceBoard* board) {
    if (board->needStatus) handle_status(board);
    if (board->needCalibration) handle_calibration(board);
    if (!board->led) handle_led_on(board);
    if (board->needActivation) handle_activation(board);
    if (board->needDumpStart) handle_data_dump(board);

    int bytes_read = recv(board->receive_sock, buffer, sizeof(buffer), 0);
    process_received_data(bytes_read, buffer, board);

    usleep(10000);
}

/**
 * @brief Main entry point of the application.
 *
 * This function initializes the `WiiBalanceBoard`, checks the validity 
 * of the provided MAC address, and establishes a Bluetooth connection 
 * to the Balance Board. If a valid MAC address is detected, a message 
 * is displayed to the user, suggesting a command line format for 
 * establishing an immediate connection on subsequent runs:
 *
 * @code
 * printf("YOU MAY USE \"%s %s\" FOR IMMEDIATE CONNECTION\n", argv[0], board.mac);
 * @endcode
 *
 * A background thread is started to handle user input. Currently, this 
 * thread only supports program termination commands, allowing the user 
 * to end the main loop. However, in future versions, this thread could 
 * be expanded to accept additional commands, such as status requests 
 * or calibration triggers, by setting corresponding flags in the 
 * `WiiBalanceBoard` object.
 * 
 * The main loop operates as long as the `is_running` flag remains set to `true`.
 * Upon termination, the function performs cleanup by releasing all 
 * resources and closing the Bluetooth connection to the Balance Board.
 *
 * @param argc Number of arguments passed to the program at startup.
 * @param argv Array of strings containing the arguments passed.
 * @return 0 if the program completes successfully.
 */

int main(int argc, char *argv[]) {
    WiiBalanceBoard board = {
        .needStatus = true,
        .needCalibration = true,
        .needActivation = true,
        .led = false,
        .needDumpStart = true,
        .is_running = true
    };


    if (is_valid_mac(argc, argv)) strcpy(board.mac,argv[1]);
    else if(find_wii_balance_board(&board) != 0) strcpy(board.mac, WII_BALANCE_BOARD_ADDR);

    board.control_sock = connect_l2cap(board.mac, 0x11);
    board.receive_sock = connect_l2cap(board.mac, 0x13);

    // Thread erstellen, der im Hintergrund läuft
    pthread_t threadId;
    createThread(&board, &threadId);

    // Hauptschleife, die so lange läuft, wie is_running true ist
    while (board.is_running) {
        main_loop(&board);
    }

    // Ressourcen aufräumen
    pthread_join(threadId, NULL);
    close(board.control_sock);
    close(board.receive_sock);
    printf("\n");
    printf("YOU MAY USE \"%s %s\" FOR IMMEDIATE CONNECTION\n",argv[0], board.mac);
    return 0;
}
