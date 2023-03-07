# SamplePlayer

## Author

Jamie Robertson

## Description

This is an advanced example application for the Daisy Rogue board. It demonstrates how to take advantage of the board's
10 audio output channels, MIDI in and microSD card to make a multi-channel sample player.

Upon reset, the app will look on the microSD card for ten wav files, named "sample1.wave" through "sample10.wav", and
will load them into SDRAM. These WAV files may be up to 10 seconds in length. MIDI Notes will then play each sample
through the corresponding output channel.

This is an ongoing work-in-progress; I'm using it to develop and test the Daisy Rogue board support in libDaisy.

