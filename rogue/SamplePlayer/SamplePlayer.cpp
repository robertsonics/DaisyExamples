#include "daisy_rogue.h"
#include "daisysp.h"
#include "Sample.h"

#define __DEBUG_MESSAGES__

// Number of output channels = number of samples; 
#define NUM_CHANNELS    10

// Max mono wav file size in samples = 8 secs
#define MAX_BUFFER_SIZE 384000

// First MIDI note number triggers first sample
#define FIRST_MIDI_NOTE 48

// Use the daisy namespace to prevent having to type
// daisy:: before all libdaisy functions
using namespace daisy;
using namespace daisysp;

// Declare a DaisyRogue object called rogue
DaisyRogue rogue;

bool ledState = false;

// SD card stuff that needs to be global
SdmmcHandler sdcard;
FatFSInterface fsi;
WAV_FormatTypeDef wavHeader;
FIL fil;

// Create a sample object for each of our output channels
Sample sample[NUM_CHANNELS];

// Allocate an SDRAM buffer for each sample, as well as a single
//  buffer for when we need to load and convert 16-bit files.
float DSY_SDRAM_BSS sampleBuff[NUM_CHANNELS][MAX_BUFFER_SIZE];
int16_t DSY_SDRAM_BSS convertBuffer[MAX_BUFFER_SIZE];

// ****************************************************************************
// Audio callback routine for the Seed's stereo input/output
void SeedAudioCallback(AudioHandle::InputBuffer  in,
                       AudioHandle::OutputBuffer out,
                       size_t                   size)
{
    //Prefill output buffer with silence
    memset(&out[0][0], 0, size * 2 * sizeof(float));

    // Fetch data if corresponding sample is playing
    if (sample[8].isPlaying())
        sample[8].getSamples(&out[0][0], size);
    if (sample[9].isPlaying())
        sample[9].getSamples(&out[1][0], size);

}

// ****************************************************************************
// Audio callback routine for the Rogue's 8 output channels TDM. The TDM
//  out buffer is not interleaved and is organized as:
//
//  float out[channel][size]
//
// The Daisy Rogue provides TDM output only, so the input buffer is not used.

void RogueAudioCallback(AudioHandle::TdmInputBuffer in,
                        AudioHandle::TdmOutputBuffer out,
                        int numChans,
                        size_t size)
{
    // Prefill all channels with silence
    memset(out, 0, (size * numChans * sizeof(float)));

    // Then fetch data for any samples that are currently playing
    for (int s = 0; s < 8; s++) {
        if (sample[s].isPlaying())
            sample[s].getSamples(&out[s * size], size);
    }
}

// ****************************************************************************
// MIDI input event handler
void HandleMidiMessage(MidiEvent m) {

    switch(m.type)
    {
        case NoteOn: {
            NoteOnEvent p = m.AsNoteOn();
            if ((p.note >= FIRST_MIDI_NOTE) && (p.note < FIRST_MIDI_NOTE + NUM_CHANNELS)) {
                if(p.velocity > 0.f)
                    sample[p.note - FIRST_MIDI_NOTE].start();
                else
                    sample[p.note - FIRST_MIDI_NOTE].start();
            }
        }
        break;

        case NoteOff: {
            NoteOnEvent p = m.AsNoteOn();
            if ((p.note >= FIRST_MIDI_NOTE) && (p.note < FIRST_MIDI_NOTE + NUM_CHANNELS)) {
                sample[p.note - FIRST_MIDI_NOTE].stop();
            }
        }
        break;
        default: break;
    }
}

// ****************************************************************************
// Main
int main(void) {

    uint32_t tickFreq;
    uint32_t currTime;
    uint32_t lastTime;
    uint32_t numSamples;
    bool audioPlaying = false;

    // Configure and Initialize the Daisy Seed
    rogue.Init();
    rogue.SetSeedLed(false);

    //rogue.seed.StartLog(true);
    //System::Delay(4000);
   
    // Rogue's TDM buffers are currently hard-coded to 64 samples so
    //  we'll set the Seed's block size to that as well
    rogue.seed.SetAudioBlockSize(64);

    // Initialize sample buffer pointers
    for (int s = 0; s < NUM_CHANNELS; s++)
        sample[s].init((float *)&sampleBuff[s][0], MAX_BUFFER_SIZE);

    // Initialize the microSD and FAT File System, and mount the card
    SdmmcHandler::Config sd_config;
    sd_config.Defaults();
    sdcard.Init(sd_config);
    fsi.Init(FatFSInterface::Config::MEDIA_SD);
    FATFS& fs = fsi.GetSDFileSystem();
    char filename[32];
    UINT br;

    if (f_mount(&fs, "/", 1) == FR_OK) {
 
         // Now search the microSD card for appropriately named wav files
        //  and load our samples accordingly.
        for (int s = 0; s < NUM_CHANNELS; s++) {

            sprintf(filename, "sample%d.wav", s + 1);
            if (f_open(&fil, filename, FA_OPEN_EXISTING | FA_READ) == FR_OK) {

                // Read the wav header. For the time being, we'll ignore the sample
                //  rate and only make sure that the file is mono.
                FRESULT fr = f_read(&fil, &wavHeader, sizeof(wavHeader), &br);
                if ((fr == FR_OK) && (wavHeader.NbrChannels == 1)) {

                    // Load the file according to the sample type. Make sure we don't
                    //  load more than will fit in our buffers.
                    switch (wavHeader.BitPerSample) {

                        case 16:
                            numSamples = wavHeader.SubCHunk2Size / 2;
                            if (numSamples > MAX_BUFFER_SIZE)
                                numSamples = MAX_BUFFER_SIZE;                           
                            sample[s].setNumSamples(numSamples);
                            fr = f_read(&fil, convertBuffer, (numSamples * 2), &br);
                            for (uint32_t i = 0; i < numSamples; i++) {
                                sampleBuff[s][i] = s162f(convertBuffer[i]);
                            }
                        break;

                        case 32:
                            numSamples = wavHeader.SubCHunk2Size / 4;
                            if (numSamples > MAX_BUFFER_SIZE)
                                numSamples = MAX_BUFFER_SIZE;                           
                            sample[s].setNumSamples(numSamples);
                            fr = f_read(&fil, &sampleBuff[s][0], (numSamples * 4), &br);
                        break;

                        default:
                        break;
                    }
                }
                f_close(&fil);
            }
        }
    }

     //Start the audio callbacks
    rogue.seed.StartAudio(SeedAudioCallback);
    rogue.StartRogueAudio(RogueAudioCallback);

    //Start MIDI input
    rogue.midi.StartReceive();

    // Initialize some timing variables.
    tickFreq = rogue.system.GetTickFreq() / 1000;
    lastTime = rogue.system.GetTick() / tickFreq;

    // Loop forever
    for(;;) {

        // Process any incoming MIDI events.
        rogue.midi.Listen();
        while (rogue.midi.HasEvents()) {
            HandleMidiMessage(rogue.midi.PopEvent());
        }

        // Turn on the Seed's LED whenever there's any audio playing
        audioPlaying  = false;
        for (int c = 0; c < NUM_CHANNELS; c++) {
            if (sample[c].isPlaying())
                audioPlaying = true;
        }
        rogue.SetSeedLed(audioPlaying);

        // In case we want to do anything at a regular interval...
        currTime = rogue.system.GetTick() / tickFreq;
        if ((currTime - lastTime) > 1000) {
            lastTime = currTime;
            //ledState = !ledState;
            //rogue.SetSeedLed(ledState);
            //rogue.seed.PrintLine("Blink");
        }
    }
}
