#include "daisy_rogue.h"
#include "daisysp.h"

#define LED_BLINK_MS      500

// Use the daisy namespace to prevent having to type
// daisy:: before all libdaisy functions
using namespace daisy;
using namespace daisysp;

// Declare a DaisyRogue object
DaisyRogue  rogue;

Oscillator osc;
bool ledState = false;

bool trigger[8];

// ****************************************************************************
void SeedAudioCallback(AudioHandle::InputBuffer  in,
                       AudioHandle::OutputBuffer out,
                       size_t                    size)
{
    memset(&out[0][0], 0, size * 2 * sizeof(float));
}

// ****************************************************************************
void RogueAudioCallback(AudioHandle::TdmInputBuffer in,
                        AudioHandle::TdmOutputBuffer out,
                        int numChans,
                        size_t size)
{
    float osc_out;

    // Prefill all channels with silence
    memset(out, 0, (size * numChans * sizeof(float)));

    // For each trigger that is active, fill that output channel with data from
    //  the oscillator.

    for(size_t i = 0; i < size; i++)
    {
        //get the next oscillator sample
        osc_out = osc.Process();

        // Check each trigger.
        for (int t = 0; t < 8; t++) {
            if (trigger[t]) {
                out[i + (t * size)] = osc_out;
            }
        }  
    }  
}

// ****************************************************************************
int main(void)
{

    uint32_t tickFreq;
    uint32_t currTime;
    uint32_t lastTime;
   
    // Configure and Initialize the Daisy Seed
    rogue.Init();

    for (int i = 0; i < 8; i++)
        trigger[i] = false;

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
                trigger[t] = true;
            }
            if (rogue.GetSwitch(t)->FallingEdge()) {
                trigger[t] = false;
            }
         }

        currTime = rogue.system.GetTick() / tickFreq;

        //If time to do so, toggle the heartbeat LED
        if ((currTime - lastTime) > LED_BLINK_MS) {
            lastTime = currTime;

            ledState = !ledState;
            rogue.SetSeedLed(ledState);

        }
    }
}

