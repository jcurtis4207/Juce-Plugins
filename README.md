# Juce-Plugins

This is a collection of audio plugins made in C++ using the [Juce](https://juce.com) Framework.

Much inspiration has been taken from [The Audio Programmer](https://www.youtube.com/channel/UCpKb02FsH4WH4X_2xhIoJ1A) on Youtube.

# 

Since I'm using Reaper on Windows for testing, the plugin format is VST3.

The completed plugin builds are in the VST3_Plugins folder.

Since I'm primarily a Pro Tools user, I will hopefully build them as AAX at some point.

# 

The collection currently includes:

### Clipper
A simple hard clipper.
The GUI is yet another shameless ripoff of Waves L1 with only threshold and ceiling controls.

### Compressor
A rather versatile compressor. 
It includes a stereo gain reduction meter, a highpass filter sidechain, as well as switchable stereo and multi-mono linking modes.
I adapted the compression algorithm from Daniel Rudrich's [Simple Compressor](https://github.com/DanielRudrich/SimpleCompressor).

### Gain
A simple plugin that includes a gain knob with a range of -30dB to +30dB, a phase inversion button, and a level meter.

### E-eq
An SSL-style equalizer with two filters and four parametric bands, and a few tweaks.
The highpass and lowpass filters have optional slopes of 12, 24, and 36dB/Oct.
The high and low shelf bands can be switched to a bell, and two fully parametric bands.
All of the bands have access to the entire frequency spectrum.

### Limiter
A ripoff of the L1 limiter. I think the layout of the L1 is the best out there, so I haven't tried to reinvent the wheel (yet).

### Tilt-eq
A tilting equalizer, similar to the Tonelux Tilt.
It uses a high and low shelf with a fixed Q and inverse gain.
As the tilt knob turns clockwise, the high shelf increases and the low shelf decreases, and visa versa for the other direction.
There is also a frequency control to adjust where the tilt occurs.

### Modules
Various modular components that can be easily added to a plugin. Includes:
* DualFilter - a dsp module with highpass and lowpass filters
* Meter - A simple stereo level meter
* PowerLine - a graphics object for creating Linux-style powerline shapes with text

#

My ultimate goal for this is to be able to mix an entire song using only plugins I created.

These are the plugins I think are necessary (though I'm not sure how many will be doable):

* <s>Phase Inversion</s>
* <s>Metering</s> and Spectrum
* <s>Filter (highpass and lowpass)</s>
* <s>Parametric Equalizer</s>
* <s>Compressor</s> (with Sidechain)
* De-esser and/or Parametric sidechain compressor
* <s>Limiter</s>
* Expander/Gate
* <s>Clipper</s>
* Distortion
* Delay
* Reverb

#

This repository includes only the source code files and the vst3 plugins.

It does not include any of the Juce files or any of the VisualStudio2019 files.
