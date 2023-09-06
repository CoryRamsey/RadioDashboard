#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <hamlib/rig.h>
#include <stdbool.h>

#define SERIAL_PORT "/dev/ttyUSB1" //TODO config file / accept parameter

bool is_connected = false;
RIG *ft991;

// establish connection
bool connect_radio() {
    
    if (is_connected) {
        printf("Radio is already connected.\n");
        return true;
    }

    rig_model_t myrig_model = RIG_MODEL_FT991; // Hardcoded ft991

    

    // DEBUG
    rig_set_debug_level(1);

    //Radio Setup
    hamlib_port_t myport;
    myport.type.rig = RIG_PORT_SERIAL;
    myport.parm.serial.rate = 38400;
    myport.parm.serial.data_bits = 8;
    myport.parm.serial.stop_bits = 1;
    myport.parm.serial.parity = RIG_PARITY_NONE;
    myport.parm.serial.handshake = RIG_HANDSHAKE_NONE;
    strncpy(myport.pathname, SERIAL_PORT, HAMLIB_FILPATHLEN - 1);


    ft991 = rig_init(myrig_model);
    strncpy(ft991->state.rigport.pathname, SERIAL_PORT, HAMLIB_FILPATHLEN - 1);

   int connect = rig_open(ft991);

    if (connect != RIG_OK) {
        printf("rig_open: error = %s\n", rigerror(connect));
        rig_cleanup(ft991); // Clean up memory if there's an error
        return false;
    } else {
        printf("Radio Connected!\n");
    }

    is_connected = true;
    return true;
}


double get_current_frequency() {
    if (is_connected) {
        RIG *rig = ft991; // Use the global rig instance

        freq_t freq;
        int retcode = rig_get_freq(rig, RIG_VFO_CURR, &freq);

        if (retcode == RIG_OK) {
            double frequency = freq / 1000000.0; // Convert to MHz
            return frequency;
        } else {
            printf("rig_get_freq: error = %s\n", rigerror(retcode));
            return 0.0; // Return 0 as an error indicator (false)
        }
    } else {
        printf("Error: Radio not connected.\n");
        return 0.0; // Return 0 as an error indicator (false)
    }
}

// Function to close the radio connection
void disconnect_radio() {
    if (is_connected) {
        rig_close(ft991);
        rig_cleanup(ft991);
        is_connected = false;
        printf("Radio disconnected.\n");
    } else {
        printf("Error: Radio not connected.\n");
    }
}

int set_freq(freq_t hz) {
    if (is_connected) {
        RIG *rig = ft991; // Use the global rig instance

        vfo_t VFO;
        rig_get_vfo(rig, &VFO);
        int retcode = rig_set_freq(rig, VFO, hz);

        if (retcode == RIG_OK) {
            return 1;
        } else {
            printf("rig_set_freq: error = %s\n", rigerror(retcode));
            return 0; // Return 0 as false
        }
    } else {
        printf("Error: Radio not connected.\n");
        return 0; // Return 0 as false
    }
}

