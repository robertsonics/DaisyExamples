Daisy Rogue Examples
==================================

Daisy Rogue is a carrier board from Robertsonics for the Electro-Smith Daisy Seed
that provides 10 unbalanced audio outputs, a microSD card socket, a powered USB
Host port, MIDI in and 8 GPIO pins. It was designed for use with Robertsonics
firmware that implements a low-latency, polyphonic embedded audio player similar
to the WAV Trigger and Tsunami but with increased MIDI capabilities.

As service to the Daisy community, I'm providing a board support package for the
Electro-Smith toolchain and libDaisy to enable you to write your own firmware for
the Daisy Rogue board, as well as providing a TDM implementation for the PCM1681
multi-channel audio DAC for those wanting to design their own hardware.

This is a work in progress. There are currently two simple example applications:

**SamplePlayer** - This example loads ten .wav files from the microSD card into
SDRAM. A range of MIDI Note events will play each sample through the corresponding
output channel. It demonstrates reading from the microSD card, MIDI input and
routing audio to specific channes via the TDM audio output buffer.

Audio files should be named "sample1.wav" through "sample10.wav" and can be either
16-bit fixed or 32-bit float, 48kHz, mono wav files. If the files are longer than
8 seconds, the data will be truncated. The first MIDI note of the range is note
number 48, but this can be easily changed.

This app could be the basis for a 10-output drum module. Next steps would be to
allow velocity to control volume and/or sample selection.

**TriggerOsc** - This simple example shows how to use the 8 trigger inputs to
send send oscillator data to different outputs. Each trigger will output an A440
sine wave to the corresponding output. Multiple triggers can be activated at the
same time.

A schematic for the Daisy Rogue PCB can be found at the bottom of this page:

[https://www.robertsonics.com/rogue-wav]

**ToDo**

The Daisy Rogue's TDM audio buffer is currently hardcoded for a 48kHz sample rate
with a block size of 64 samples, which means approximately 1ms of latency. The
SAI2 TDM implementation is currently in the board support source file and should
be rolled into the sai module of libdaisy, at which time things like sample rate
and blocksize can be made adjustable.

The Seed's stereo input/output channels (SAI1) are synchronized to the Rogue's 8
TDM channels (SAI2) by virtue of the fact that they run off the same CPU clocks,
but they are currently not started simultaneously. More work needs to be done to
see if/how audio input from SAI1 can be cleanly sent to SAI2 output channels. An
interesting application example would be a 10-channel panner, where for example,
mono input from channel 1 could be dyncamically panned around 8 or 10 output
channels under MIDI control.

