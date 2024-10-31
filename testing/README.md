markdown

# Nur ein Versuch

Dieser Code basiert auf den Ideen von [keldu/WiiWeight](https://github.com/keldu/WiiWeight), wurde jedoch von mir neu geschrieben.

Um das Gerät zu identifizieren, benötigst du `evtest`:

```bash
gcc -o test interesting_option.c

# Finde das Gerät heraus und beende evtest
evtest

# Starte das Programm mit der entsprechenden Event-Nummer
./test /dev/input/event[nummer]
```
# Just a Try

This code is inspired by the ideas of [keldu/WiiWeight](https://github.com/keldu/WiiWeight) but has been rewritten by me.

You will need evtest to identify the device:

```bash

gcc -o test interesting_option.c

# Find the device and exit evtest
evtest

# Run the program with the appropriate event number
./test /dev/input/event[number]

```