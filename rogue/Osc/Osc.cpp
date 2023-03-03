#include "daisy_rogue.h"
#include "daisysp.h"

// Use the daisy namespace to prevent having to type
// daisy:: before all libdaisy functions
using namespace daisy;
using namespace daisysp;

// Declare a DaisyRogue object called hardware
DaisyRogue  hardware;
Oscillator osc1;
Oscillator osc2;
int hdResult;
bool ledState = false;
bool debugState = false;

void SeedAudioCallback(AudioHandle::InterleavingInputBuffer  in,
                       AudioHandle::InterleavingOutputBuffer out,
                       size_t                                size)
{
    float osc_out;

    //Fill the block with samples
    for(size_t i = 0; i < size; i += 2)
    {
       //get the next oscillator sample
        osc_out = osc1.Process();

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

    debugState = !debugState;
    hardware.SetDebugOut(ledState);

    //Fill the block with samples
    int n = 0;
    for(size_t i = 0; i < size; i++)
    {
        //get the next oscillator sample
        osc_out = osc2.Process();

        for (int c = 0; c < numChans; c++) {
            out[n++] = osc_out;
        }
    }  
}

void HandleMidiMessage(MidiEvent m) {

}

int main(void)
{

    uint32_t tickFreq;
    uint32_t currTime;
    uint32_t lastTime;

    // Configure and Initialize the Daisy Seed
    hdResult = hardware.Init();
    if (hdResult == 0)
        hardware.SetSeedLed(true);
    hardware.SetSeedAudioBlockSize(128);

    //How many samples we'll output per second
    float samplerate = hardware.SeedAudioSampleRate();

    //Set up oscillator 1
    osc1.Init(samplerate);
    osc1.SetWaveform(osc1.WAVE_SIN);
    osc1.SetAmp(1.f);
    osc1.SetFreq(1000);

    //Set up oscillator 2
    osc2.Init(samplerate);
    osc2.SetWaveform(osc2.WAVE_SIN);
    osc2.SetAmp(1.f);
    osc2.SetFreq(440);

    //Start the audio callbacks
    hardware.StartSeedAudio(SeedAudioCallback);
    hdResult = hardware.StartRogueAudio(RogueAudioCallback);
    if (hdResult != 0)
        hardware.SetSeedLed(false);

    hardware.midi.StartReceive();

    tickFreq = hardware.system.GetTickFreq() / 1000;
    lastTime = hardware.system.GetTick() / tickFreq;

    // Loop forever
    for(;;) {
        // Wait 500ms
        //System::Delay(500);

        hardware.midi.Listen();
        while (hardware.midi.HasEvents()) {
            HandleMidiMessage(hardware.midi.PopEvent());
        }

        currTime = hardware.system.GetTick() / tickFreq;

        if ((currTime - lastTime) > 1000) {
            lastTime = currTime;
            ledState = !ledState;
            hardware.SetSeedLed(ledState);
        }
    }
}
