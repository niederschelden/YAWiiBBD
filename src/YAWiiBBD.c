#include "YAWiiBBessentials.h"

/**
 * @file YAWiiBBD.c
 * @brief Hauptdatei für die YAWiiBB-Anwendung, die die Interaktion mit einem Wii Balance Board ermöglicht.
 *
 * Diese Datei enthält die Hauptfunktionalität der Anwendung, einschließlich der
 * Initialisierung der Verbindungen, der Hauptschleife zur Verarbeitung von
 * Befehlen und der Verwaltung von Threads. Abhängig von den Kompilierungsflags
 * kann die Debug-Stufe variieren, was die Protokollierung und den Status der Anwendung
 * beeinflusst.
 *
 * Es wird eine Verbindung zum Wii Balance Board über Bluetooth hergestellt,
 * und die Anwendung verarbeitet Daten in einer Schleife, solange das Board
 * aktiv ist.
 *
 * @note Um die Anwendung zu kompilieren, verwenden Sie die folgenden Befehle:
 *       - Für die Standardversion: 
 *         @code
 *         gcc -Wall -o simple YAWiiBBD.c -lbluetooth
 *         @endcode
 *       - Für die erweiterte Version:
 *         @code
 *         gcc -DYAWIIBB_EXTENDED -Wall -o complex YAWiiBBD.c YAWiiBBessentials.c -lbluetooth
 *         @endcode
 */

#ifdef YAWIIBB_EXTENDED
const LogLevel debug_level = DEBUG; 
#else
const LogLevel debug_level = RAW; 
#endif // YAWIIBB_EXTENDED


/**
 * @brief Hauptschleife der Anwendung.
 *
 * Diese Funktion führt die Kernoperationen der Anwendung aus, indem sie
 * verschiedene Aktionen basierend auf den Flags im `WiiBalanceBoard`-Objekt
 * ausführt. Dazu gehören Statusabfragen, Kalibrierung, Aktivierung und
 * das Ein-/Ausschalten der LED. Die empfangenen Daten werden verarbeitet,
 * und die Schleife pausiert für 10 Millisekunden, um die CPU-Belastung
 * zu minimieren.
 *
 * @param board Ein Zeiger auf das `WiiBalanceBoard`-Objekt, das die
 *               aktuellen Statusinformationen und Flags enthält.
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
 * @brief Haupteinstiegspunkt der Anwendung.
 *
 * Diese Funktion initialisiert das `WiiBalanceBoard`, überprüft die
 * Gültigkeit der übergebenen MAC-Adresse und stellt eine Bluetooth-
 * Verbindung zum Balance Board her. Ein Hintergrundthread wird gestartet,
 * der für die Benutzereingabe zuständig ist. Die Hauptschleife wird
 * ausgeführt, solange das `is_running`-Flag auf `true` gesetzt ist.
 * Am Ende werden alle Ressourcen aufgeräumt und die Verbindung zum
 * Balance Board geschlossen.
 *
 * @param argc Anzahl der übergebenen Argumente beim Programmstart.
 * @param argv Array von Zeichenfolgen mit den übergebenen Argumenten.
 * @return 0, wenn das Programm erfolgreich abgeschlossen wurde.
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
