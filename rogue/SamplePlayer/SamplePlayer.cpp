#include "daisy_rogue.h"
#include "daisysp.h"
#include "Sample.h"

#define __DEBUG_MESSAGES__

#define NUM_CHANNELS    10
#define MAX_BUFFER_SIZE 144000

// Use the daisy namespace to prevent having to type
// daisy:: before all libdaisy functions
using namespace daisy;
using namespace daisysp;

// Declare a DaisyRogue object called rogue
DaisyRogue rogue;

bool ledState = false;

SdmmcHandler sdcard;
FatFSInterface fsi;

// Create a sample object for each of our output channels
Sample sample[NUM_CHANNELS];

WAV_FormatTypeDef wavHeader;
FIL fil;

// Allocate an SDRAM buffer for each sample, as well as a single
//  buffer for when we need to load and convert 16-bit files.
float DSY_SDRAM_BSS sampleBuff[NUM_CHANNELS][MAX_BUFFER_SIZE];
int16_t convertBuffer[MAX_BUFFER_SIZE];

void SeedAudioCallback(AudioHandle::InterleavingInputBuffer  in,
                       AudioHandle::InterleavingOutputBuffer out,
                       size_t                                size)
{
    //Fill the block with samples
    for(size_t i = 0; i < size; i += 2)
    {
        //Set the left and right outputs
        out[i]     = 0.0f;
        out[i + 1] = 0.0f;
    }
}

void RogueAudioCallback(AudioHandle::TdmInputBuffer in,
                        AudioHandle::TdmOutputBuffer out,
                        int numChans,
                        size_t size)
{
    //Fill the block with samples
    int n = 0;
    for(size_t i = 0; i < size; i++)
    {
        for (int c = 0; c < numChans; c++) {
            out[n++] = 0.0f;
        }
    }  
}

void HandleMidiMessage(MidiEvent m) {

   switch(m.type)
    {
        case NoteOn:
        {
            NoteOnEvent p = m.AsNoteOn();
            // Note Off can come in as Note On w/ 0 Velocity
            if(p.velocity == 0.f)
            {
                //voice_handler.OnNoteOff(p.note, p.velocity);
                sample[0].start();
            }
            else
            {
                sample[0].stop();
            }
        }
        break;
        case NoteOff:
        {
            //NoteOnEvent p = m.AsNoteOn();
            //voice_handler.OnNoteOff(p.note, p.velocity);
            sample[0].stop();
        }
        break;
        default: break;
    }
}

int main(void)
{

    uint32_t tickFreq;
    uint32_t currTime;
    uint32_t lastTime;

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
        sample[s].init((float *)&sampleBuff[s], MAX_BUFFER_SIZE);

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
        for (int s = 0; s < 2; s++) {

            sprintf(filename, "sample%d.wav", s + 1);
            if (f_open(&fil, filename, FA_OPEN_EXISTING | FA_READ) == FR_OK) {

                // Read the wav header. For the time being, we'll ignore the sample
                //  rate and only make sure that the file is mono.
                FRESULT fr = f_read(&fil, &wavHeader, sizeof(wavHeader), &br);
                if ((fr == FR_OK) && (wavHeader.NbrChannels == 1)) {

                    switch (wavHeader.BitPerSample) {

                        case 16:
                            sample[s].setNumSamples(wavHeader.SubCHunk2Size / 2);
                            fr = f_read(&fil, convertBuffer, wavHeader.SubCHunk2Size, &br);
                            for (uint32_t i = 0; i < sample[s].getNumSamples(); i++) {
                                sampleBuff[s][i] = s162f(convertBuffer[i]);
                            }
                        break;

                        case 32:
                            fr = f_read(&fil, &sampleBuff[s][0], wavHeader.SubCHunk2Size, &br);
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

    tickFreq = rogue.system.GetTickFreq() / 1000;
    lastTime = rogue.system.GetTick() / tickFreq;

    // Loop forever
    for(;;) {

        rogue.midi.Listen();
        while (rogue.midi.HasEvents()) {
            HandleMidiMessage(rogue.midi.PopEvent());
        }

        currTime = rogue.system.GetTick() / tickFreq;

        if ((currTime - lastTime) > 1000) {
            lastTime = currTime;
            ledState = !ledState;
            rogue.SetSeedLed(ledState);
            rogue.seed.PrintLine("Blink");
        }
    }
}
