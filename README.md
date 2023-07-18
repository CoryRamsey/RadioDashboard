# RadioDashboard


This is a simple radio dashboard for the Yaesu FT-991A. It uses the hamlib library to control the radio. It is written in C and uses GTK3 for the GUI.

It currently supports viewing and changing the frequency of the radio.

---

### Building:

>gcc -o dash main.c radio_control.c -lhamlib \`pkg-config --cflags --libs gtk+-3.0\`