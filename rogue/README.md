Daisy Rogue Examples
==================================

Daisy Rogue is a carrier board from Robertsonics for the Electro-Smith Daisy Seed
that provides 10 unbalanced audio outputs, a microSD card socket, a powered USB
Host port, MIDI in and 8 GPIO pins. It was designed for use with Robertsonics
firmware that implements a low-latency, polyphonic embedded audio player similar
to the WAV Trigger and Tsunami but with increased MIDI capabilities.

As service to the Daisy community, I'm providing a board support package for the
Electro-Smith toolchain and libDaisy to enable you to write your own firmware for
the Daisy Rogue board, as well as to offer a TDM implementation for the PCM1682
multi-channel audio DAC for those wanting to design their own hardware.

This is a work in progress. I am currently providing three example applications:

**Osc** - is the Electro-Smith simple oscillator example modified to output on
all 10 output channels.

**Midi** - is a very simple MIDI input test app that blinks the Seed LED

**SamplePlayer** - is a more serious example that loads ten .wav files from
the microSD card, named "sampleX.wav" where X is 1 - 10, into SDRAM. MIDI Note
events will play each sample out the corresponding output channel. This example
demonstrates reading from the microSD card, MIDI input and routing audio via
the TDM audio output buffer.

A schematic for the Daisy Rogue PCB can be found at the bottom of this page:

[https://www.robertsonics.com/rogue-wav]

