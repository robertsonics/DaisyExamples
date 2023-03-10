#include "daisy_rogue.h"
#include "daisysp.h"
#include <string>

using namespace daisy;
using namespace daisysp;

DaisyRogue rogue;

// Typical Switch case for Message Type.
void HandleMidiMessage(MidiEvent m)
{
    switch(m.type)
    {
        case NoteOn:

            if(m.data[1] != 0)
                rogue.SetSeedLed(true);
            else
                rogue.SetSeedLed(false);

        break;
        case NoteOff:
                rogue.SetSeedLed(false);
        break;
    
        default: break;
    }
}


// Main -- Init, and Midi Handling
int main(void)
{
    rogue.Init();


    // Start stuff.
    rogue.midi.StartReceive();
    for(;;)
    {
        rogue.midi.Listen();
        // Handle MIDI Events
        while(rogue.midi.HasEvents())
        {
            HandleMidiMessage(rogue.midi.PopEvent());
        }
    }
}
