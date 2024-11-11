
//achtungung man braucht mehr rechte: sudo setcap cap_dac_read_search+ep ./dateiname
//wieder rechte wegnehmen sudo setcap -r ./dateiname
//derzeitige mac des controllers  00:22:D7:E1:59:7E


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <string.h>

void list_input_devices() {
    struct dirent *entry;
    DIR *dp = opendir("/dev/input/");
    
    if (dp == NULL) {
        perror("Failed to open /dev/input/");
        return;
    }

    printf("Available input devices:\n");
    int index = 0;
    while ((entry = readdir(dp))) {
        if (strncmp(entry->d_name, "event", 5) == 0) {
            char device_path[256];
            snprintf(device_path, sizeof(device_path), "/dev/input/%s", entry->d_name);

            int fd = open(device_path, O_RDONLY | O_NONBLOCK);
            if (fd < 0) {
                perror("Failed to open device");
                continue;
            }

            char device_name[256] = "Unknown";
            if (ioctl(fd, EVIOCGNAME(sizeof(device_name)), device_name) < 0) {
                perror("Failed to get device name");
            }

            printf("%d: %s (%s)\n", index++, device_name, device_path);

            close(fd);
        }
    }

    closedir(dp);
}

int main(int argc, char **argv) {
    setbuf(stdout, NULL);
    int fd, bytes;
    struct input_event ev;
     // Aktuelle Werte für Code 3, 4 und 5
    int current_val_3 = 0;
    int current_val_4 = 0;
    int current_val_5 = 0;

    /*
    // Liste der Eingabegeräte anzeigen
    list_input_devices();

    // Benutzereingabe für die Auswahl des Geräts
    int device_number;
    printf("Enter the number of the device you want to read (0 - N): ");
    scanf("%d", &device_number);
    if(device_number == 99) exit(0);

    // Erstellen des Pfades zum ausgewählten Eingabegerät
    char device_path[20];
    snprintf(device_path, sizeof(device_path), "/dev/input/event%d", device_number);
    */
    char device_path[] = "/dev/input/event7";   
    // Öffnen des Eingabegeräts im Nur-Lese-Modus
    fd = open(device_path, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open device");
        return 1;
    }

    // Exklusive Kontrolle über das Eingabegerät übernehmen
    if (ioctl(fd, EVIOCGRAB, 1) < 0) {
        perror("Failed to get exclusive access to device");
        close(fd);
        return 1;
    }

    // printf("Listening to raw data from %s...\n", device_path);

    // Hauptschleife zur Rohdatenerfassung
    int i = 0;
    
    //printf("Code 3      Code 4      Code 5\n"); // Spaltenüberschrift für Klarheit

    while (i++ < 10000) {
        bytes = read(fd, &ev, sizeof(struct input_event));
        if (bytes < (int)sizeof(struct input_event)) {
            perror("Failed to read input event");
            break;
        }

        // Nur Typ 3 berücksichtigen
        if (ev.type == 3) {
            if (ev.code == 3) {
                current_val_3 = ev.value;
            } else if (ev.code == 4) {
                current_val_4 = ev.value;
            } else if (ev.code == 5) {
                current_val_5 = ev.value;
            }

            // Ausgabe der aktuellen Werte mit fester Breite
            printf("%-10d %-10d %-10d\r", current_val_3, current_val_4, current_val_5);
        }
    }
    printf("STOP\n");
    // Freigeben des Geräts
    ioctl(fd, EVIOCGRAB, 0);
    close(fd);
    return 0;
}
