#include "YAWiiBBessentials.h"

// Compile: gcc -Wall -o complex YAWiiBBD.c YAWiiBBessentials.c -lbluetooth
// Compile Extended: gcc -DYAWIIBB_EXTENDED -Wall -o complex YAWiiBBD.c YAWiiBBessentials.c YAWiiBBextended.c -lbluetooth

#ifdef YAWIIBB_EXTENDED
const LogLevel debug_level = DEBUG; 
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
    if (board == NULL) {
        printf("Board ist nicht initialisiert.\n");
        return;
    }

    printf("Kalibrierungsdaten:\n");
    
    for (int i = 0; i < 3; i++) {
        printf("Kalibrierung %d:\n", i);
        
        for (int j = 0; j < 4; j++) {
            printf("Position %d: %u\t", j, board->calibration[i][j]);
        }
        printf("\n"); // Neue Zeile nach jeder Kalibrierungsebene
    }
}
#endif // YAWIIBB_EXTENDED

void* threadFunction(void* arg) {
    WiiBalanceBoard* board = (WiiBalanceBoard*)arg;  // Typumwandlung
    char ch;

    // Warten auf die Benutzereingabe
    while (true) {
        ch = getchar();
        if (ch == '\n') {  // Wenn nur Enter gedrückt wird
            board->is_running = false;  // Setze die boolesche Variable auf false
            break; 
        }
    }

    return NULL;  // Thread beendet sich
}

void createThread(WiiBalanceBoard* board, pthread_t* threadId) {
    // Thread erstellen
    if (pthread_create(threadId, NULL, threadFunction, (void*)board) != 0) {
        perror("Fehler beim Erstellen des Threads");
        board->is_running = false;  // Setze die boolesche Variable auf false
        exit(1);
    }
}

int main() {
    WiiBalanceBoard board = {
        .needStatus = true,
        .needCalibration = true,
        .needActivation = true,
        .led = false,
        .needDumpStart = true,
        .is_running = true
    };

    if(find_wii_balance_board(&board) != 0) strcpy(board.mac, WII_BALANCE_BOARD_ADDR);

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
    // print_calibration_data(&board);
    return 0;
}
