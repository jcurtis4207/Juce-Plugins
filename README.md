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
A simple hard clipper with 4x oversampling.
The GUI is yet another shameless ripoff of Waves L1 with only threshold and ceiling controls.

### Compressor
A rather versatile compressor. 
It includes a stereo gain reduction meter, a highpass filter sidechain, as well as switchable stereo and multi-mono linking modes.
The original inspiration is from Daniel Rudrich's [Simple Compressor](https://github.com/DanielRudrich/SimpleCompressor).
The envelope implementation is from [this flyingSand article](https://christianfloisand.wordpress.com/2014/06/09/dynamics-processing-compressorlimiter-part-1/).

### De-esser
A filter-based de-esser plugin.
It includes attack and release controls, as well as typical threshold and frequency controls.
There is a button to control whether the compression affects the entire frequency range or just the sidechain, a button to switch between linked stereo and dual mono modes, and a button to listen to the filtered sidechain do dial in the frequency.

### Delay
A stereo delay plugin with host tempo sync.
It features a maximum delay time of 2 seconds.
It includes standard delay controls, as well as integrated modulation, distortion, and filters.
It also has a width control to offset the left and right delay signals.

### Distortion
A waveshaper-based distortion plugin with 4x oversampling.
There are controls for switching between 4 different waveshaping functions, as well as an "Anger" control for modifying the severity of the function.
There are also filters and a switchable tilt-eq/low-shelf before the distortion stage.

### E-eq
An SSL-style equalizer with two filters and four parametric bands, and a few tweaks.
The highpass and lowpass filters have optional slopes of 12, 24, and 36dB/Oct.
The high and low shelf bands can be switched to a bell, and two fully parametric bands.
All of the bands have access to the entire frequency spectrum.

### Gain
A simple plugin that includes a gain knob with a range of -30dB to +30dB, a phase inversion button, and a level meter.

### Gate
An expander/gate plugin with standard threshold, ratio, attack, release, and hold controls.
It also has a filtered sidechain, with listen functionality.

### Limiter
A ripoff of the L1 limiter. 
I think the layout of the L1 is the best out there, so I haven't tried to reinvent the wheel (yet).
It also includes a button to link and unlink the channels, which the L1 doesn't have.
The backend is the same implementation as the Compressor plugin.

### Multiband Comp
A 4 band compressor, using the same backend as the Compressor plugin.
There are standard controls for attack, release, ratio, threshold, and makeup gain on each band.
Also there are controls for the crossover frequencies and listen on each band, as well as a button to switch between linked stereo and dual mono modes.

### Reverb
A reverb plugin using the builtin Juce DSP Reverb module.
It also has modulation, predelay, and filters.

### Tilt-eq
A tilting equalizer, similar to the Tonelux Tilt.
It uses a high and low shelf with a fixed Q and inverse gain.
As the tilt knob turns clockwise, the high shelf increases and the low shelf decreases, and visa versa for the other direction.
There is also a frequency control to adjust where the tilt occurs.

#

My ultimate goal for this is to be able to mix an entire song using only plugins I created.

These are the plugins I think are necessary (though I'm not sure how many will be doable):

* <s>Phase Inversion</s>
* <s>Metering</s> and Spectrum
* <s>Filter (highpass and lowpass)</s>
* <s>Parametric Equalizer</s>
* <s>Compressor</s> (with Sidechain)
* <s>De-esser</s> 
* <s>Multiband Compressor</s>
* <s>Limiter</s>
* <s>Expander/Gate</s>
* <s>Clipper</s>
* <s>Distortion</s>
* <s>Delay</s>
* <s>Reverb</s>

#

This repository includes only the source code files and the vst3 plugins.

It does not include any of the Juce files or any of the VisualStudio2019 files.
