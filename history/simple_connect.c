#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>

// Wii Balance Board MAC-Adresse (Meines)
#define WII_BALANCE_BOARD_ADDR "00:23:CC:43:DC:C2"

// Puffergröße für eingehende Daten
#define BUFFER_SIZE 256

// Sendet einen Befehl über den Control Socket
void send_command(int sock, unsigned char* command, int length) {
    if (send(sock, command, length, 0) < 0) {
        perror("Fehler beim Senden des Befehls");
        exit(1);
    }
}

// Empfangt die Statusdaten vom Control-Kanal, gibt sie roh aus und interpretiert sie
void receive_status_data(int control_sock) {
    unsigned char buffer[10];  // Statusantwort hat typischerweise 10 Bytes
    int bytes_read = recv(control_sock, buffer, sizeof(buffer), 0);
    if (bytes_read <= 0) {
        perror("Fehler beim Empfangen der Statusdaten");
        exit(1);
    }

    // Rohdaten ausgeben
    printf("Statusdaten (roh, %d Bytes): ", bytes_read);
    for (int i = 0; i < bytes_read; i++) {
        printf("%02x ", buffer[i]);
    }
    printf("\n");

    // Interpretation der Statusdaten
    unsigned char battery_level = buffer[7]; // Byte 7: Batteriestand
    unsigned char extension_status = buffer[3] & 0x02; // Byte 3: Bit 1 zeigt den Erweiterungsstatus

    printf("Interpretierte Statusdaten:\n");
    printf("Batteriestand: %u%%\n", battery_level);  // Der Batteriestand ist direkt im Byte gespeichert
    if (extension_status) {
        printf("Erweiterung erkannt\n");
    } else {
        printf("Keine Erweiterung erkannt\n");
    }
}


// Empfang der Kalibrierungsdaten
void receive_calibration_data(int control_sock) {
    unsigned char buffer[23];  // 23 Byte Kalibrierungsdaten (entsprechend dem Python-Skript)
    int bytes_read = recv(control_sock, buffer, sizeof(buffer), 0);
    if (bytes_read <= 0) {
        perror("Fehler beim Empfangen der Kalibrierungsdaten");
        exit(1);
    }

    printf("Kalibrierungsdaten empfangen (%d Bytes): ", bytes_read);
    for (int i = 0; i < bytes_read; i++) {
        printf("%02x ", buffer[i]);
    }
    printf("\n");
}

// Baut eine L2CAP-Verbindung auf
int connect_l2cap(const char* bdaddr_str, uint16_t psm) {
    struct sockaddr_l2 addr = { 0 };
    int sock = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
    if (sock < 0) {
        perror("Fehler beim Erstellen des Sockets");
        exit(1);
    }

    // Bluetooth Adresse setzen
    addr.l2_family = AF_BLUETOOTH;
    addr.l2_psm = htobs(psm);
    str2ba(bdaddr_str, &addr.l2_bdaddr);

    // Verbindung herstellen
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Fehler beim Herstellen der Verbindung");
        close(sock);
        exit(1);
    }

    return sock;
}

int main() {
    int control_sock, interrupt_sock;
    unsigned char buffer[BUFFER_SIZE];

    // Verbindet zu den Control- und Interrupt-Sockets des Wii Balance Boards
    control_sock = connect_l2cap(WII_BALANCE_BOARD_ADDR, 0x11);  // Control-Kanal
    interrupt_sock = connect_l2cap(WII_BALANCE_BOARD_ADDR, 0x13);  // Interrupt-Kanal

    // Handshake durchführen

    // 1. Status anfordern (0x12 Befehl)
    unsigned char status_command[] = { 0x52, 0x12, 0x00, 0x32 };
    send_command(control_sock, status_command, sizeof(status_command));

    // Statusdaten empfangen und interpretieren
    receive_status_data(control_sock);

    // 2. Erweiterte Funktionen aktivieren (0x13 Befehl)
    unsigned char activate_command[] = { 0x52, 0x13, 0x04 };
    send_command(control_sock, activate_command, sizeof(activate_command));

    // 3. Kalibrierungsdaten anfordern (0x17 Befehl)
    unsigned char calibration_command[] = { 0x52, 0x17, 0x04, 0xa4, 0x00, 0x24, 0x00, 0x18 };
    send_command(control_sock, calibration_command, sizeof(calibration_command));

    // Kalibrierungsdaten empfangen und ausgeben
    receive_calibration_data(control_sock);

    // 4. Datenmodus aktivieren (0x15 Befehl)
    unsigned char data_mode_command[] = { 0x52, 0x15, 0x00, 0x32 };
    send_command(control_sock, data_mode_command, sizeof(data_mode_command));

    // Daten kontinuierlich empfangen und ausgeben
    while (1) {
        int bytes_read = recv(interrupt_sock, buffer, sizeof(buffer), 0);
        if (bytes_read > 0) {
            printf("Empfangene Daten (%d Bytes): ", bytes_read);
            for (int i = 0; i < bytes_read; i++) {
                printf("%02x ", buffer[i]);
            }
            printf("\n");
        } else {
            perror("Fehler beim Empfangen der Daten");
            break;
        }
        usleep(10000);  // 10ms warten
    }

    // Sockets schließen
    close(control_sock);
    close(interrupt_sock);

    return 0;
}
