#include <stdio.h>
#include <stdlib.h>
#include <pulse/simple.h>
#include <pulse/error.h>
#include <sndfile.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#ifdef _WIN32
#include <conio.h>

// Function to check if a key is pressed (Windows)
int kbhit() {
    return _kbhit();
}

#else

// check for keypress(Linux)
int kbhit() {
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

#endif

#define SAMPLE_RATE 48000
#define NUM_CHANNELS 2
#define FRAMES_PER_BUFFER 1024
#define FILENAME_BASE "recorded_audio"
#define VOLUME_SCALE 5.0 // amplify volume

//determine file name
bool fileExists(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file) {
        fclose(file);
        return true;
    }
    return false;
}


void getNextFilename(char *filename, int counter) {
    if (counter == 0) {
        strcpy(filename, FILENAME_BASE);
    } else {
        sprintf(filename, "%s_%02d", FILENAME_BASE, counter);
    }
    strcat(filename, ".wav");
}

int main() {
    pa_simple *paConnection = NULL;
    pa_sample_spec paSampleSpec;
    int error;

    // PulseAudio sample spec
    paSampleSpec.format = PA_SAMPLE_S16LE;
    paSampleSpec.rate = SAMPLE_RATE;
    paSampleSpec.channels = NUM_CHANNELS;

    // Open the PulseAudio connection for recording from the specified device
    paConnection = pa_simple_new(NULL, "record_audio", PA_STREAM_RECORD, "alsa_input.usb-Burr-Brown_from_TI_USB_Audio_CODEC-00.analog-stereo", "record_audio", &paSampleSpec, NULL, NULL, &error); //todo config file to set device
    if (!paConnection) {
        fprintf(stderr, "Error opening PulseAudio connection: %s\n", pa_strerror(error));
        return 1;
    }

    // Read audio from PulseAudio, amplify, and write to the WAV file
    int16_t buffer[FRAMES_PER_BUFFER * NUM_CHANNELS];
    bool recording = true;
    bool message = false;
    int counter = 0;
    char filename[100];
    while (recording) {
        
        getNextFilename(filename, counter);

        // Check if the file exists, if it does, increment the counter
        while (fileExists(filename)) {
            counter++;
            getNextFilename(filename, counter);
        }

        // Open the output WAV 
        SF_INFO sfInfo;
        sfInfo.samplerate = SAMPLE_RATE;
        sfInfo.channels = NUM_CHANNELS;
        sfInfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
        SNDFILE *sndFile = sf_open(filename, SFM_WRITE, &sfInfo);
        if (!sndFile) {
            fprintf(stderr, "Error opening WAV file for writing: %s\n", sf_strerror(NULL));
            pa_simple_free(paConnection);
            return 1;
        }

        // Start writing wav to file
        while (recording) {
            if (pa_simple_read(paConnection, buffer, sizeof(buffer), &error) < 0) {
                fprintf(stderr, "Error reading from PulseAudio: %s\n", pa_strerror(error));
                break;
            } 

            if (!message) {
                printf("Recording... Press q to stop\n");
                message = true;
            }
            
            // Amplify the audio samples by scaling them
            for (int i = 0; i < FRAMES_PER_BUFFER * NUM_CHANNELS; i++) {
                buffer[i] = (int16_t)(buffer[i] * VOLUME_SCALE);
            }

            sf_write_short(sndFile, buffer, sizeof(buffer) / sizeof(int16_t));

            // Check for keyboard input to stop recording
            if (kbhit()) {
                char ch = getchar();
                if (ch == 'q' || ch == 'Q') {
                    recording = false;
                }
            }
        }

        // Close the WAV file for this recording
        sf_close(sndFile);

        counter++;
    }

    // Close the PulseAudio connection
    pa_simple_free(paConnection);

    return 0;
}
