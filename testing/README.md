# Nur ein Versuch

Dieser Code basiert auf den Ideen von [keldu/WiiWeight](https://github.com/keldu/WiiWeight), wurde jedoch von mir neu geschrieben.

```bash
gcc -o test newTry.c
# rechte erhöhen um devices zu lesen
sudo setcap cap_dac_read_search+ep ./test
# Starte das Programm 
./test
#Wähle nicht die NUmmer!!, sondern die devicenummer aus

```
# Just a Try

This code is inspired by the ideas of [keldu/WiiWeight](https://github.com/keldu/WiiWeight) but has been rewritten by me.

```bash
gcc -o test newTry.c
# grat extended rights
sudo setcap cap_dac_read_search+ep ./test
# run the  program 
./test
#choose the devicenumber at end of the line; not the number in front

```
