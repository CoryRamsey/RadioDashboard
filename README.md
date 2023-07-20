# RadioDashboard


This is a simple radio dashboard for the Yaesu FT-991A. It uses the hamlib library to control the radio. It is written in C and uses GTK3 for the GUI.

It currently supports viewing and changing the frequency of the radio.
<div style="text-align:center">
  ![RadioDashboard](https://github.com/CoryRamsey/RadioDashboard/blob/main/images/dash.png)
</div>


---
### Dependencies: 
* hamlib
* gtk3
* glib2.0-dev
* pulseaudio
* libsndfile1-dev

### Building:

Dashboard and Record:
- make all

Dashboard or Record Only:

- make dash
- make record


### Usage:

./dash will run the GUI program. User may need to be in dialout group to access the serial port. The port is currently hardcoded. 

#### Audio Recording:
./record is a command line tool that will record audio from the radio on the currently tuned frequency. It will record to a file called "recorded_audio.wav" in the current directory. The file can be played back with aplay or other audio players. The recording is done using pulseaudio. Currently, the device name is hardcoded. This may vary from system to system. The device name can be found by running "pactl list sources". The device name is the name in the <> brackets.