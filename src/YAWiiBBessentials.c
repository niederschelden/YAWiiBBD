#include "YAWiiBBessentials.h"
/**
 * @file YAWiiBBessentials.c
 * @brief Core file for funktions predefined in YAWiiBBessentials.h.
 */


// Kommandos als konstante Byte-Arrays wie in der Header Datei beschrieben
const unsigned char status_command[] = { 0x52, 0x12, 0x00, 0x32 };
const unsigned char activate_command[] = { 0x52, 0x13, 0x04 };
const unsigned char calibration_command[] = { 0x52, 0x17, 0x04, 0xa4, 0x00, 0x24, 0x00, 0x18 };
const unsigned char led_on_command[] = { 0x52, 0x11, 0x10 };
const unsigned char data_dump_command[] = { 0x52, 0x15, 0x00, 0x32 };
unsigned char buffer[BUFFER_SIZE];


void print_info(const LogLevel* is_debug_level, const char* message, const unsigned char* buffer, int length, const WiiBalanceBoard* board) {
    // Ausgabe basierend auf dem Log-Level
    if(length>1){
        switch (debug_level) {
            case RAW:
                if(buffer[1] == 0x32) {
                    printf("Sensor:      ");
                    for (int i = 0; i < length; i++) printf("%i:%02x ", i, buffer[i]);
                    printf("\n");
                    }
                if(buffer[1] == 0x21) {
                    printf("Kalibration: ");
                    for (int i = 0; i < length; i++) printf("%i:%02x ", i, buffer[i]);
                    printf("\n");
                    }
                if(buffer[1] == 0x20) {
                    printf("Status:      ");
                    for (int i = 0; i < length; i++) printf("%i:%02x ", i, buffer[i]);
                    printf("\n");
                    }
            break;
            #ifdef YAWIIBB_EXTENDED
            case DECODE:
                if (buffer[1] == 0x32) {                    
                   uint16_t gramm[4];
                   for(int i=0; i<4; i++){
                    uint16_t raw = bytes_to_int_big_endian(buffer, 4 + (2 * i), &length);
                    gramm[i] = calc_mass(board, raw, i);
                   }
                uint16_t summe = 0;
                for (int i = 0; i < 4; i++) {
                    printf("%u,", gramm[i]);
                    summe += (uint16_t)gramm[i]/1000;
                    }
                printf("%u       \r", summe);
                }
                break;
            case DEBUG:
                 if (buffer[1] == 0x32) {                    
                   uint16_t gramm[4];
                   for(int i=0; i<4; i++){
                    uint16_t raw = bytes_to_int_big_endian(buffer, 4 + (2 * i), &length);
                    gramm[i] = calc_mass(board, raw, i);
                   }
                printf("Vorne rechts %.2f, hinten rechts %.2f, vorne links %.2f, hinten links %.2f \n", gramm[0] / 1000.0, gramm[1] / 1000.0, gramm[2] / 1000.0, gramm[3] / 1000.0);
                 }
                 if(buffer[1] == 0x21) {
                    printf("Kalibration: ");
                    for (int i = 0; i < length; i++) printf("%i:%02x ", i, buffer[i]);
                    printf("\n");
                    }
                if(buffer[1] == 0x20) {
                    printf("Status:      ");
                    for (int i = 0; i < length; i++) printf("%i:%02x ", i, buffer[i]);
                    printf("\n");
                    }
                break;
            case VERBOSE:
                // noch leer
                printf("VERBOSE: %s", message);
                break;
            #endif //YAWIIBB_EXTENDED
        }
    }
}

int find_wii_balance_board(WiiBalanceBoard* board) {
    inquiry_info *ii = NULL;
    int max_rsp, num_rsp;
    int dev_id, sock, len, flags;
    char addr[19] = { 0 };
    char name[248] = { 0 };

    dev_id = hci_get_route(NULL);
    sock = hci_open_dev(dev_id);
    if (dev_id < 0 || sock < 0) {
        perror("Fehler beim Öffnen des lokalen Bluetooth-Geräts");
        return -1;
    }

    len = 8;
    max_rsp = 255;
    flags = IREQ_CACHE_FLUSH;
    ii = (inquiry_info*)malloc(max_rsp * sizeof(inquiry_info));

    num_rsp = hci_inquiry(dev_id, len, max_rsp, NULL, &ii, flags);
    if (num_rsp < 0) {
        perror("Fehler bei der Bluetooth-Abfrage");
        close(sock);
        free(ii);
        return -1;
    }

    for (int i = 0; i < num_rsp; i++) {
        ba2str(&(ii+i)->bdaddr, addr);
        if (hci_read_remote_name(sock, &(ii+i)->bdaddr, sizeof(name), name, 0) < 0)
            strcpy(name, "[unbekannt]");

        printf("Gefundenes Gerät: %s (%s)\n", addr, name);

        if (strcmp(name, "Nintendo RVL-WBC-01") == 0) {
            strcpy(board->mac, addr);
            printf("Wii Balance Board gefunden: %s\n", board->mac);
            close(sock);
            free(ii);
            return 0; // Erfolg
        }
    }

    close(sock);
    free(ii);
    return -1; // Gerät nicht gefunden
}

void send_command(int sock, const unsigned char* command, int length) {
    if (send(sock, command, length, 0) < 0) {
        perror("Fehler beim Senden des Befehls");
        exit(1);
    }
}

int connect_l2cap(const char* bdaddr_str, uint16_t psm) {
    struct sockaddr_l2 addr = { 0 };
    int sock = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
    if (sock < 0) {
        perror("Fehler beim Erstellen des Sockets");
        exit(1);
    }

    addr.l2_family = AF_BLUETOOTH;
    addr.l2_psm = htobs(psm);
    str2ba(bdaddr_str, &addr.l2_bdaddr);

    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Fehler beim Herstellen der Verbindung");
        close(sock);
        exit(1);
    }
    return sock;
}


void handle_status(WiiBalanceBoard* board) {
    send_command(board->control_sock, status_command, sizeof(status_command));
    board->needStatus = false;
    print_info(&debug_level, "Hole Status", 0, 0, 0);
}

void handle_calibration(WiiBalanceBoard* board) {
    send_command(board->control_sock, calibration_command, sizeof(calibration_command));
    board->needCalibration = false;
    print_info(&debug_level, "Hole Kalibrierungsdaten", 0, 0, 0);
}

void handle_led_on(WiiBalanceBoard* board) {
    send_command(board->control_sock, led_on_command, sizeof(led_on_command));
    board->led = true;
    print_info(&debug_level, "Schalte LED an", 0, 0, 0);
}

void handle_activation(WiiBalanceBoard* board) {
    send_command(board->control_sock, activate_command, sizeof(activate_command));
    board->needActivation = false;
    print_info(&debug_level, "Sende Aktivierung", 0, 0, 0);
}

void handle_data_dump(WiiBalanceBoard* board) {
    send_command(board->control_sock, data_dump_command, sizeof(data_dump_command));
    board->needDumpStart = false;
    print_info(&debug_level, "Starte Dump", 0, 0, 0);
}

void process_received_data(int bytes_read, unsigned char* buffer, WiiBalanceBoard* board) {
    if (bytes_read > 1) {
        print_info(&debug_level, "Empfangene Daten: ", buffer, bytes_read, board);
        if (buffer[1] == 0x32 && buffer[3] == 0x08) board->is_running = 0;
        #ifdef YAWIIBB_EXTENDED
        if (buffer[1]== 0x21) process_calibration_data(&bytes_read, buffer, board);
        #endif // YAWIIBB_EXTENDED
    } else {
        perror("Fehler beim Empfangen der Daten");
        board->is_running = 0;
    }
}


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

int is_valid_mac(int argc, char *argv[]) {
    // Überprüfen, ob genau ein Argument (MAC-Adresse) übergeben wurde - falls nicht, ohne fehler zurück
    if (argc != 2) {
        return 0;
    }

    const char *mac = argv[1];

    // MAC-Adresse sollte 17 Zeichen lang sein
    if (strlen(mac) != 17) {
        perror("Fehler: MAC-Adresse muss genau 17 Zeichen lang sein.\n");
        return 0;
    }

    // Prüfen, ob die MAC-Adresse das Format XX:XX:XX:XX:XX:XX hat
    for (int i = 0; i < 17; i++) {
        if (i % 3 == 2) {
            // Alle dritten Zeichen sollten Doppelpunkte sein
            if (mac[i] != ':') {
                perror("Fehler: Ungültiges Format. Verwenden Sie das Format XX:XX:XX:XX:XX:XX.\n");
                return 0;
            }
        } else {
            // Alle anderen Zeichen sollten Hexadezimalzahlen sein
            if (!isxdigit(mac[i])) {
                perror("Fehler: Ungültiges Zeichen in der MAC-Adresse.\n");
                return 0;
            }
        }
    }
    return 1;
}

#ifdef YAWIIBB_EXTENDED


uint16_t bytes_to_int_big_endian(const unsigned char *buffer, size_t position, int* max){

    // Überprüfen, ob die Position gültig ist
    if (buffer == NULL || position < 0 || position + 1 >= *max) {
        return 0; // Fehlerbehandlung: ungültige Position
    }
    // Kombinieren des Bytes an der angegebenen Position (high_byte)
    // und dem nächsten Byte (low_byte)
    uint8_t high_byte = buffer[position];       // Hochbyte
    uint8_t low_byte = buffer[position + 1];    // Niedriges Byte

    //printf("High: %i Low: %i \n", high_byte, low_byte);


    return (high_byte << 8) | low_byte; // Rückgabe der kombinierten Zahl
}

void process_calibration_data(int* bytes_read, unsigned char* buffer, WiiBalanceBoard* board) {
    // Prüfen, ob das zweite Kalibrierungspaket empfangen wurde
    bool second_packet =  (buffer[15] == 0x00);
    // Kalibrierungsdaten aus dem Buffer extrahieren
    //Bytes 7-14 werden für die Kalibrierung 0 verwendet (4 Paare).
    //Bytes 15-22 werden für die Kalibrierung 1 verwendet (4 Paare).
    //Für den zweiten Kalibrierungspaket if(byte[15]==0x00):
    //Bytes 7-14 werden für die Kalibrierung 3 verwendet (4 Paare)
    if(!second_packet) 
        for (uint8_t i = 0; i < 4; i++) {
            // Erster Kalibrierungssatz bytes 
            board->calibration[0][i] = bytes_to_int_big_endian(buffer, 7 + (2 * i), bytes_read);
            board->calibration[1][i] = bytes_to_int_big_endian(buffer, 15 + (2 * i), bytes_read);
    }
    // Wenn der zweite Paket nicht vorhanden ist, die Werte in calibration[2] speichern
    else 
        for (uint8_t i = 0; i < 4; i++) 
            board->calibration[2][i] = bytes_to_int_big_endian(buffer, 7 + (2 * i), bytes_read);
 
}

uint16_t calc_mass(const WiiBalanceBoard* board, uint16_t raw, int pos) {
    // Berechnung des Gewichts in Gramm basierend auf rohen Daten
    uint16_t mass;

    // Fall 1: Rohdaten sind kleiner als die Kalibrierung für 0 kg
    if (raw < board->calibration[0][pos]) {
        return 0; // 0 Gramm
    } 
    // Fall 2: Rohdaten liegen zwischen der Kalibrierung für 0 kg und 17 kg
    else if (raw < board->calibration[1][pos]) {
        mass = (uint16_t)(34000 * ((float)(raw - board->calibration[0][pos]) /
                                    (board->calibration[1][pos] - board->calibration[0][pos])));
        return mass; // Gewicht in Gramm zurückgeben
    } 
    // Fall 3: Rohdaten liegen zwischen der Kalibrierung für 17 kg und 34 kg
    else if (raw < board->calibration[2][pos]) {
        mass = 17000 + (uint16_t)(17000 * ((float)(raw - board->calibration[1][pos]) /
                                             (board->calibration[2][pos] - board->calibration[1][pos])));
        return mass; // Gewicht in Gramm zurückgeben
    } 
    // Fall 4: Rohdaten sind größer oder gleich der Kalibrierung für 34 kg
    else {
        // Lineare Extrapolation: Gewicht über 34 kg
        mass = 34000 + (uint16_t)(17000 * ((float)(raw - board->calibration[2][pos]) / 
                                             (board->calibration[2][pos] - board->calibration[1][pos])));
        return mass; // Gewicht in Gramm zurückgeben
    }
}

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

#endif