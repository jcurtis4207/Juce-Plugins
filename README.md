# Juce-Plugins

This is a collection of audio plugins made in C++ using the [Juce](https://juce.com) Framework.

Much inspiration has been taken from [The Audio Programmer](https://www.youtube.com/channel/UCpKb02FsH4WH4X_2xhIoJ1A) on Youtube.

# 

Since I'm using Reaper on Windows for testing, the plugin format is VST3.

The completed plugin builds are in the VST3_Plugins folder.

Since I'm primarily a Pro Tools user, I will hopefully build them as AAX at some point.

# 

The collection currently includes:

### Gain
A simple plugin that includes a gain knob with a range of -30dB to +30dB, a phase inversion button, and a level meter.

### E-eq
An SSL-style equalizer with two filters and four parametric bands, and a few tweaks.
The highpass and lowpass filters have optional slopes of 12, 24, and 36dB/Oct.
The high and low shelf bands can be switched to a bell, and two fully parametric bands.
All of the bands have access to the entire frequency spectrum.

### Modules
Various modular components that can be easily added to a plugin. Includes:
* Meter - A simple stereo level meter

#

My ultimate goal for this is to be able to mix an entire song using only plugins I created.

These are the plugins I think are necessary (though I'm not sure how many will be doable):

* <s>Phase Inversion</s>
* <s>Metering</s> and Spectrum
* <s>Filter (highpass and lowpass)</s>
* <s>Parametric Equalizer</s>
* Compressor (with Sidechain)
* Limiter
* Expander/Gate
* Clipper
* Distortion
* Delay
* Reverb

#

This repository includes only the source code files and the vst3 plugins.

It does not include any of the Juce files or any of the VisualStudio2019 files.
