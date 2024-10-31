# Nur ein Versuch

Dieser Code basiert auf den Ideen von [keldu/WiiWeight](https://github.com/keldu/WiiWeight), wurde jedoch von mir neu geschrieben.

Um das Gerät zu identifizieren, benötigst du `evtest`:

```bash
gcc -o test newTry.c
sudo setcap cap_dac_read_search+ep ./dateiname
# Starte das Programm 
./list
#Wähle nicht die NUmmer!!, sondern die devicenummer aus

```
# Just a Try

This code is inspired by the ideas of [keldu/WiiWeight](https://github.com/keldu/WiiWeight) but has been rewritten by me.

You will need evtest to identify the device:

```bash
gcc -o test newTry.c
sudo setcap cap_dac_read_search+ep ./dateiname
# run the  program 
./list
#choose the devicenumber at end of the line; not the number in front

```
