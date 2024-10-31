#ifndef YAWIIBBESSENTIALS_H
#define YAWIIBBESSENTIALS_H

//#define YAWIIBB_EXTENDED


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

// Bedingte Einbindung von YAWiiBBextended.h, wenn YAWIIBB_EXTENDED aktiviert ist


#define WII_BALANCE_BOARD_ADDR "00:23:CC:43:DC:C2"  /**< Default MAC address for the Wii Balance Board */
#define BUFFER_SIZE 24  /**< Buffer size for data reception */



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
extern unsigned char buffer[BUFFER_SIZE];

/* Externe Deklarationen der Befehls-Arrays */
extern const unsigned char status_command[];
extern const unsigned char activate_command[];
extern const unsigned char calibration_command[];
extern const unsigned char led_on_command[];
extern const unsigned char data_dump_command[];



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
 * @brief Logs messages with different verbosity levels.
 *
 * This function displays messages to the console with an optional buffer,
 * allowing selection of verbosity levels: RAW, DEBUG, or VERBOSE.
 *
 * @param level LogLevel enum indicating the verbosity level.
 * @param message Constant character string message to be logged.
 * @param buffer Optional byte array to display raw data if provided.
 * @param length Length of the byte array (ignored if buffer is NULL).
 */
void print_info(const LogLevel* is_debug_level, const char* message, const unsigned char* buffer, int length, const WiiBalanceBoard* board);

/**
 * @brief Finds the Wii Balance Board by scanning nearby Bluetooth devices.
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


/**
 * @brief Verarbeitet das Senden eines Status-Kommandos an das Wii Balance Board.
 * 
 * Diese Funktion wird aufgerufen, wenn das Flag `needStatus` gesetzt ist, 
 * und schickt den entsprechenden Status-Befehl an das Board. 
 * 
 * @param board Pointer zur WiiBalanceBoard-Struktur, die den aktuellen Status hält.
 */
void handle_status(WiiBalanceBoard* board);

/**
 * @brief Verarbeitet das Senden eines Kalibrierungs-Kommandos an das Wii Balance Board.
 * 
 * Diese Funktion wird aufgerufen, wenn das Flag `needCalibration` gesetzt ist, 
 * und schickt den entsprechenden Kalibrierungs-Befehl an das Board.
 * 
 * @param board Pointer zur WiiBalanceBoard-Struktur, die den aktuellen Status hält.
 */
void handle_calibration(WiiBalanceBoard* board);

/**
 * @brief Verarbeitet das Senden eines LED-Anschalt-Kommandos an das Wii Balance Board.
 * 
 * Diese Funktion wird aufgerufen, wenn das LED-Flag `led` noch nicht gesetzt ist,
 * und schaltet die LED des Boards ein.
 * 
 * @param board Pointer zur WiiBalanceBoard-Struktur, die den aktuellen Status hält.
 */
void handle_led_on(WiiBalanceBoard* board);

/**
 * @brief Verarbeitet das Senden eines Aktivierungs-Kommandos an das Wii Balance Board.
 * 
 * Diese Funktion wird aufgerufen, wenn das Flag `needActivation` gesetzt ist, 
 * und schickt den entsprechenden Aktivierungs-Befehl an das Board.
 * 
 * @param board Pointer zur WiiBalanceBoard-Struktur, die den aktuellen Status hält.
 */
void handle_activation(WiiBalanceBoard* board);

/**
 * @brief Verarbeitet das Senden eines Daten-Dump-Kommandos an das Wii Balance Board.
 * 
 * Diese Funktion wird aufgerufen, wenn das Flag `needDumpStart` gesetzt ist, 
 * und startet das kontinuierliche Übertragen der Board-Daten.
 * 
 * @param board Pointer zur WiiBalanceBoard-Struktur, die den aktuellen Status hält.
 */
void handle_data_dump(WiiBalanceBoard* board);

/**
 * @brief Verarbeitet empfangene Daten vom Wii Balance Board.
 * 
 * Diese Funktion überprüft die empfangenen Daten und wertet sie aus. Falls
 * eine bestimmte Bedingung erfüllt ist (z.B. der Empfangscode zeigt einen Fehler),
 * wird der `is_running`-Status auf `false` gesetzt.
 * 
 * @param bytes_read Anzahl der gelesenen Bytes im `buffer`.
 * @param buffer     Buffer, der die empfangenen Daten enthält.
 * @param board      Pointer zur WiiBalanceBoard-Struktur, die den aktuellen Status hält.
 */
void process_received_data(int bytes_read, unsigned char* buffer, WiiBalanceBoard* board);

/**
 * @brief Thread-Funktion zur Überwachung der Benutzereingabe zur Steuerung des Wii Balance Boards.
 *
 * Wartet auf eine Benutzereingabe über die Konsole. Wenn der Benutzer die Eingabetaste
 * ohne weitere Zeichen drückt, setzt die Funktion das `is_running`-Flag des `WiiBalanceBoard`
 * Objekts auf `false` und beendet den Thread.
 *
 * @param arg Ein void-Pointer auf das `WiiBalanceBoard`-Objekt, das der Funktion übergeben wird
 *            und zur Laufzeit aufgerufen wird, um den Status zu ändern.
 * @return Immer `NULL` – zeigt an, dass der Thread beendet ist.
 */

void* threadFunction(void* arg);

/**
 * @brief Erstellt einen neuen Thread und startet die `threadFunction` zur Überwachung der Benutzersteuerung.
 *
 * Diese Funktion erstellt einen neuen Thread und weist ihn an, die Funktion `threadFunction` auszuführen.
 * Bei Fehlern wird eine Fehlermeldung ausgegeben, das `is_running`-Flag des Wii Balance Boards wird
 * auf `false` gesetzt, und das Programm wird mit einem Fehlercode beendet.
 *
 * @param board Zeiger auf das `WiiBalanceBoard`-Objekt, das die threadFunction überwacht.
 * @param threadId Zeiger auf die `pthread_t`-Variable, in der die ID des neuen Threads gespeichert wird.
 */

void createThread(WiiBalanceBoard* board, pthread_t* threadId);

/**
 * @brief Validiert eine übergebene MAC-Adresse auf Format und Inhalt.
 *
 * Überprüft, ob eine gültige MAC-Adresse als einziges Argument übergeben wurde.
 * Die Adresse muss genau 17 Zeichen lang sein und das Format `XX:XX:XX:XX:XX:XX` erfüllen,
 * wobei `X` ein Hexadezimalzeichen (`0-9`, `A-F`, `a-f`) ist. Die Funktion gibt
 * `1` zurück, wenn die Eingabe den Anforderungen entspricht, andernfalls `0`.
 * 
 * Bei ungültigem Format oder fehlerhaften Eingaben werden spezifische Fehlermeldungen
 * ausgegeben, die auf mögliche Ursachen hinweisen.
 *
 * @param argc Anzahl der an das Programm übergebenen Argumente.
 * @param argv Array mit den Argumenten, von denen das zweite (`argv[1]`) die MAC-Adresse sein sollte.
 * @return `1`, wenn die MAC-Adresse gültig ist, ansonsten `0`.
 */

int is_valid_mac(int argc, char *argv[]);

#ifdef YAWIIBB_EXTENDED
/**
 * @brief Verarbeitet die Kalibrierungsdaten vom Wii Balance Board.
 *
 * Diese Funktion analysiert einen Datenpuffer mit Kalibrierungsinformationen 
 * und extrahiert diese in das Kalibrierungsarray der übergebenen 
 * `WiiBalanceBoard`-Instanz. Die Kalibrierungsdaten sind 16-Bit-Werte, 
 * die in einem Big-Endian-Format gespeichert werden und aus 4 Paaren 
 * für jede der vier Ecken bestehen (topright, topleft, bottomright, bottomleft).
 *
 * @param bytes_read Anzahl der tatsächlich gelesenen Bytes im Puffer.
 * @param buffer Zeiger auf einen Puffer mit den empfangenen Bytes. 
 *               Die erwartete Struktur ist:
 *               - Bytes 7-14: Kalibrierungssatz 0 (4 Paare, 8 Bytes)
 *               - Bytes 15-22: Kalibrierungssatz 1 (4 Paare, 8 Bytes)
 *               - Wenn Byte 15 == 0x00:
 *                 - Bytes 7-14: Kalibrierungssatz 2 (4 Paare, 8 Bytes)
 * @param board Zeiger auf eine Instanz von `WiiBalanceBoard`, 
 *              in die die Kalibrierungsdaten gespeichert werden.
 *
 * @note Diese Funktion geht davon aus, dass der `buffer` mindestens 
 *       23 Bytes enthält, um die erwarteten Kalibrierungsdaten 
 *       verarbeiten zu können.
 *
 * @details 
 * Der Ablauf der Funktion ist wie folgt:
 * 1. Überprüfung, ob das zweite Kalibrierungspaket empfangen wurde 
 *    (Byte 15 == 0x00).
 * 2. Wenn das zweite Paket nicht vorhanden ist:
 *    - Bytes 7-14 werden in `calibration[0]` gespeichert (Kalibrierungssatz 0).
 *    - Bytes 15-22 werden in `calibration[1]` gespeichert (Kalibrierungssatz 1).
 * 3. Wenn das zweite Paket vorhanden ist:
 *    - Bytes 7-14 werden in `calibration[2]` gespeichert (Kalibrierungssatz 2).
 *
 * Beispiel:
 * - Empfangene Daten könnten folgendes enthalten:
 *   - buffer[7] = 0x01, buffer[8] = 0x02 (Kalibrierung 0, Paar 0)
 *   - buffer[15] = 0x00 (zeigt an, dass das zweite Paket nicht empfangen wurde)
 * 
 *   In diesem Fall würde die Funktion die Werte in `board->calibration[0]` 
 *   und `board->calibration[2]` speichern.
 */
void process_calibration_data(int* bytes_read, unsigned char* buffer, WiiBalanceBoard* board);


/**
 * @brief Konvertiert zwei aufeinanderfolgende Bytes im Buffer aus Big-Endian-Darstellung in eine dezimale Ganzzahl.
 * 
 * Interpretiert zwei Bytes im angegebenen `buffer` ab der übergebenen `position` als eine
 * 16-Bit-Ganzzahl im Big-Endian-Format (höherwertiges Byte zuerst). Die Funktion gibt diese
 * Ganzzahl zurück, sodass sie in dezimaler Form ausgegeben werden kann.
 * 
 * @param buffer     Zeiger auf den Buffer, der die Bytes enthält.
 * @param position   Die Startposition des höherwertigen Bytes im Buffer.
 * @return uint16_t  Die als dezimale Ganzzahl interpretierte Big-Endian-Zahl.
 */
uint16_t bytes_to_int_big_endian(const unsigned char *buffer, size_t position, int* max);

/**
 * @brief Berechnet das Gewicht in Gramm aus den Rohdaten des Wii Balance Boards.
 *
 * Diese Funktion nutzt Kalibrierungsdaten des Wii Balance Boards, um basierend 
 * auf den gegebenen Rohdaten ein Gewicht in Gramm zu berechnen. Das Gewicht 
 * wird innerhalb von vier Kalibrierungsbereichen interpoliert oder, wenn der Wert 
 * über dem höchsten Kalibrierungsbereich liegt, linear extrapoliert.
 *
 * @param board Ein konstanter Zeiger auf die Struktur des Wii Balance Boards, die
 *              die Kalibrierungsdaten enthält.
 * @param raw   Der Rohdatenwert, der das Gewicht repräsentiert, gemessen von einer
 *              der vier Sensorpositionen (Top-Right, Bottom-Right, etc.).
 * @param pos   Die Position des Sensors (0 bis 3) für die Kalibrierungsdaten des Rohwerts.
 *              Muss im Bereich [0, 3] liegen.
 *
 * @return Das berechnete Gewicht in Gramm als `uint16_t`.
 * 
 * @note Die Funktion kann bis zu einem Gewicht von 34 kg mit den gegebenen 
 *       Kalibrierungswerten interpolieren. Für Werte über diesem Bereich wird 
 *       das Gewicht linear extrapoliert.
 *
 * Die Berechnung verwendet vier Fälle basierend auf den Rohdaten:
 *   - Wenn der Rohwert kleiner als die Kalibrierung für 0 kg ist, wird 0 g zurückgegeben.
 *   - Liegt der Rohwert zwischen den Kalibrierungen für 0 kg und 17 kg, erfolgt eine lineare 
 *     Interpolation zwischen diesen Punkten.
 *   - Liegt der Rohwert zwischen den Kalibrierungen für 17 kg und 34 kg, erfolgt eine 
 *     Interpolation innerhalb dieses Bereichs.
 *   - Wenn der Rohwert größer oder gleich der Kalibrierung für 34 kg ist, wird eine 
 *     lineare Extrapolation basierend auf dem letzten Bereich vorgenommen.
 */

uint16_t calc_mass(const WiiBalanceBoard* board, uint16_t raw, int pos);

void print_calibration_data(const WiiBalanceBoard* board);

#endif //YAWIIBB_EXTENDED
#endif // YAWIIBBESSENTIALS_H
