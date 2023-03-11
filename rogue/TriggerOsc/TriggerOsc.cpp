#include "daisy_rogue.h"
#include "daisysp.h"

#define TRIGGER_SCAN_MS      20

// Use the daisy namespace to prevent having to type
// daisy:: before all libdaisy functions
using namespace daisy;
using namespace daisysp;

// Declare a DaisyRogue object
DaisyRogue  rogue;

Oscillator osc;
bool ledState = false;

void SeedAudioCallback(AudioHandle::InterleavingInputBuffer  in,
                       AudioHandle::InterleavingOutputBuffer out,
                       size_t                                size)
{
    float osc_out;

    //Fill the block with samples
    for(size_t i = 0; i < size; i += 2)
    {
       //get the next oscillator sample
        osc_out = 0.0f;

        //Set the left and right outputs
        out[i]     = osc_out;
        out[i + 1] = osc_out;
    }
}

void RogueAudioCallback(AudioHandle::TdmInputBuffer in,
                        AudioHandle::TdmOutputBuffer out,
                        int numChans,
                        size_t size)
{
    float osc_out;

    //Fill the block with samples
    int n = 0;
    for(size_t i = 0; i < size; i++)
    {
        //get the next oscillator sample
        osc_out = osc.Process();

        for (int c = 0; c < numChans; c++) {
            out[n++] = osc_out;
        }
    }  
}

void doTrigger(int trig) {

}

int main(void)
{

    uint32_t tickFreq;
    uint32_t currTime;
    uint32_t lastTime;
   
    // Configure and Initialize the Daisy Seed
    rogue.Init();

    // The rogue TDM buffers are currently hard-coded to 64 samples, so we'll
    //  set the Seed's audio blocksize to that as well.
    rogue.seed.SetAudioBlockSize(64);

    //How many samples we'll output per second
    float samplerate = rogue.seed.AudioSampleRate();

    //Set up our oscillator
    osc.Init(samplerate);
    osc.SetWaveform(osc.WAVE_SIN);
    osc.SetAmp(1.f);
    osc.SetFreq(1000);

    //Start the audio callbacks
    rogue.seed.StartAudio(SeedAudioCallback);
    rogue.StartRogueAudio(RogueAudioCallback);

    tickFreq = rogue.system.GetTickFreq() / 1000;
    lastTime = rogue.system.GetTick() / tickFreq;

    // Loop forever
    for(;;) {

        rogue.ProcessDigitalControls();
        for (int t = 0; t < 8; t++) {
            if (rogue.GetSwitch(t)->RisingEdge()) {
                rogue.SetSeedLed(true);
            }
            if (rogue.GetSwitch(t)->FallingEdge()) {
                rogue.SetSeedLed(false);
            }
         }

        currTime = rogue.system.GetTick() / tickFreq;

        //If time to do so, scan the input triggers
        if ((currTime - lastTime) > TRIGGER_SCAN_MS) {
            lastTime = currTime;

        }
    }
}

