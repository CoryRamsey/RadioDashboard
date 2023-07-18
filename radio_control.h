// radio_control.h

#ifndef RADIO_CONTROL_H
#define RADIO_CONTROL_H

#include <stdbool.h>
#include <hamlib/rig.h>

// Function to establish a connection to the radio
bool connect_radio();

// Function to get the radio's current frequency
double get_current_frequency();

// Function to close the radio connection
void disconnect_radio();

//Function to set frequency
int set_freq(freq_t hz);

#endif // RADIO_CONTROL_H
