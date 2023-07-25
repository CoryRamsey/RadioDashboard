# RadioDashboard


This is a simple radio dashboard for the Yaesu FT-991A.

It currently supports chaning the frequency of the radio and recording audio received from the radio.

![RadioDashboard](https://github.com/CoryRamsey/RadioDashboard/blob/main/images/dash.png?raw=true)



---
### Dependencies: 
* hamlib
* gtk3
* glib2.0-dev
* pulseaudio
* libsndfile1-dev

### Building:

- make all


### Usage:

./dash will run the GUI program. User may need to be in dialout group to access the serial port. The port is currently hardcoded. 

#### Audio Recording:

Recordings will be saved in ./recordings/. 