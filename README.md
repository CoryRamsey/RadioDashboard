# RadioDashboard


This is a simple radio dashboard for the Yaesu FT-991A. It uses the hamlib library to control the radio. It is written in C and uses GTK3 for the GUI.

---

To build:

>gcc -o dash `pkg-config --cflags --libs gtk+-3.0` -lhamlib main.c radio_control.c