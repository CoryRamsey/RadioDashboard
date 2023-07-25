#ifndef RECORD_H
#define RECORD_H

#include <stdbool.h>

#define SAMPLE_RATE 48000
#define NUM_CHANNELS 2
#define FRAMES_PER_BUFFER 1024
#define FILENAME_BASE "recorded_audio"
#define VOLUME_SCALE 5.0 // amplify volume


// Function to get the next available filename for recording
void getNextFilename(char *filename, int counter);

// Function to start recording audio
int record();

// Function to stop recording audio
void stop_recording();
#endif // AUDIO_RECORDER_H