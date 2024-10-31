#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>
#include <time.h>

int main(int argc, char** argv) {
    char* input_device = NULL;
    int fd, bytes;
    struct input_event ev;

    // Überprüfen, ob ein Eingabegerät angegeben wurde
    if (argc < 2) {
        printf("Usage: %s <input_device>\n", argv[0]);
        return 1;
    }
    input_device = argv[1];

    // Öffnen des Eingabegeräts im Nur-Lese-Modus
    fd = open(input_device, O_RDONLY);
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

    printf("Listening to raw data from %s...\n", input_device);

    // Hauptschleife zur Rohdatenerfassung
    int i = 0;
    while (i++ < 1000) {
        // Lesen der Eingabedaten vom Gerät
        bytes = read(fd, &ev, sizeof(struct input_event));
        if (bytes < (int) sizeof(struct input_event)) {
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
