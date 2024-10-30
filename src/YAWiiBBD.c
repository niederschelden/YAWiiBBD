#include "YAWiiBBessentials.h"

//Compile:          gcc -Wall -o complex YAWiiBBD.c YAWiiBBessentials.c -lbluetooth
//Compile Extended: gcc -DYAWIIBB_EXTENDED -Wall -o complex YAWiiBBD.c YAWiiBBessentials.c YAWiiBBextended.c -lbluetooth


//Debuglevel wählen:
//RAW       Outputs raw data as received without interpretation
//DECODE    Outputs big endian converted value of two bytes 
//DEBUG     Provides debugging information 
//VERBOSE   Outputs detailed information, including interpreted data 
#ifdef YAWIIBB_EXTENDED
const LogLevel debug_level = DECODE; 
#else
const LogLevel debug_level = RAW; // Setze den gewünschten Standardwert, z. B. DEFAULT
#endif // YAWIIBB_EXTENDED

// Hauptschleife mit separaten Funktionsaufrufen für jedes Flag
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

#ifdef YAWIIBB_EXTENDED
void print_calibration_data(const WiiBalanceBoard* board) {
    // Überprüfen, ob der Board-Zeiger gültig ist
    if (board == NULL) {
        printf("Board ist nicht initialisiert.\n");
        return;
    }

    printf("Kalibrierungsdaten:\n");
    
    // Schleife für jede Kalibrierungsebene
    for (int i = 0; i < 3; i++) {
        printf("Kalibrierung %d:\n", i);
        
        // Schleife für jede der vier Positionen
        for (int j = 0; j < 4; j++) {
            // Ausgabe der Werte in einem tabellarischen Format
            printf("Position %d: %u\t", j, board->calibration[i][j]);
        }
        printf("\n"); // Neue Zeile nach jeder Kalibrierungsebene
    }
}
#endif // YAWIIBB_EXTENDED

int main() {
    WiiBalanceBoard board = {
        .needStatus = true,
        .needCalibration = true,
        .needActivation = true,
        .led = false,
        .needDumpStart = true,
        .is_running = true
    };

    strcpy(board.mac, WII_BALANCE_BOARD_ADDR);

    board.control_sock = connect_l2cap(board.mac, 0x11);
    board.receive_sock = connect_l2cap(board.mac, 0x13);

    while (board.is_running) main_loop(&board);

    close(board.control_sock);
    close(board.receive_sock);
    printf("\n");
    //print_calibration_data(&board);
    return 0;
}
