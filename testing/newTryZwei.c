#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/hidraw.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <string.h>
#include <sys/ioctl.h>

void list_hid_devices() {
    struct dirent *entry;
    DIR *dp = opendir("/dev/");
    
    if (dp == NULL) {
        perror("Failed to open /dev/");
        return;
    }

    printf("Available HID devices:\n");
    int index = 0;
    while ((entry = readdir(dp))) {
        if (strncmp(entry->d_name, "hidraw", 6) == 0) {
            char device_path[256];
            snprintf(device_path, sizeof(device_path), "/dev/%s", entry->d_name);

            int fd = open(device_path, O_RDONLY | O_NONBLOCK);
            if (fd < 0) {
                perror("Failed to open device");
                continue;
            }

            char device_name[256] = "Unknown";
            ioctl(fd, HIDIOCGRAWNAME(256), device_name);
            printf("%d: %s (%s)\n", index++, device_name, device_path);

            close(fd);
        }
    }

    closedir(dp);
}

int main() {
    int fd, bytes;
    unsigned char buf[256];  // Buffer für eingehende Rohdaten

    // Liste der HID-Geräte anzeigen
    list_hid_devices();

    // Benutzereingabe für die Auswahl des Geräts
    int device_number;
    printf("Enter the number of the device you want to read (0 - N): ");
    scanf("%d", &device_number);
    if(device_number == 99) exit(0);

    // Erstellen des Pfades zum ausgewählten HID-Gerät
    char device_path[20];
    snprintf(device_path, sizeof(device_path), "/dev/hidraw%d", device_number);

    // Öffnen des HID-Geräts im Nur-Lese-Modus
    fd = open(device_path, O_RDWR);  // R/W-Modus für bidirektionale Kommunikation
    if (fd < 0) {
        perror("Failed to open device");
        return 1;
    }

    printf("Listening to raw data from %s...\n", device_path);

    // Beispiel zum Senden eines Kommandos an die Wiimote (4-Byte-Befehl)
    unsigned char command[4] = {0x52, 0x12, 0x00, 0x32};  // Status
    if (write(fd, command, sizeof(command)) < 0) {
        perror("Failed to send command");
    } else {
        printf("Command sent successfully to Wiimote\n");
    }

    // Hauptschleife zur Rohdatenerfassung
    int i = 0;
    while (i++ < 5) {
        // Lesen der Eingabedaten vom HID-Gerät
        bytes = read(fd, buf, sizeof(buf));
        if (bytes > 0) {
            printf("Data received (%d bytes): ", bytes);
            for (int j = 0; j < bytes; j++) {
                printf("%02x ", buf[j]);
            }
            printf("\n");
        } else if (bytes < 0) {
            perror("Failed to read input event");
            break;
        }
        usleep(1000);  // Leichte Verzögerung
    }

    close(fd);
    return 0;
}