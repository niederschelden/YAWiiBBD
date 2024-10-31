
//achtungung man braucht mehr rechte: sudo setcap cap_dac_read_search+ep ./dateiname
//wieder rechte wegnehmen sudo setcap -r ./dateiname



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
    int fd, bytes;
    struct input_event ev;

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

    printf("Listening to raw data from %s...\n", device_path);

    // Hauptschleife zur Rohdatenerfassung
    int i = 0;
    while (i++ < 1000) {
        // Lesen der Eingabedaten vom Gerät
        bytes = read(fd, &ev, sizeof(struct input_event));
        if (bytes < (int)sizeof(struct input_event)) {
            perror("Failed to read input event");
            break;
        }

        // Ausgabe der Rohdaten ohne weitere Verarbeitung
        printf("Time: %ld.%06ld\tType: %d\tCode: %d\tValue: %d\n", 
               ev.time.tv_sec, ev.time.tv_usec, ev.type, ev.code, ev.value);
    }

    // Freigeben des Geräts
    ioctl(fd, EVIOCGRAB, 0);
    close(fd);
    return 0;
}
