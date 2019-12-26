Aria Salvatrice's VCV Rack Modules
==================================

Hiya hello hey hi. These here are a few cool and nice modules what for [VCV Rack](https://vcvrack.com/). I hope you like them.



Big Bend and Bendlet
--------------------

![Big Bend and Bendlet](/doc/big-bend.png)

**Polyphonic quantizing pitchbend helper**. Add it between your MIDI keyboard and your oscillator to get some dope glissandi and portamenti (it's polyphonic, so I get to use the fancy Italian plural, deal) or play it with your mouse: the knob quickly springs back in place, like on real hardware. 

These modules provide a zero setup building block of a keyboard synth, and I hope you will also find horrific ways to misuse them for unintended purposes.

Both modules offer the same features. Bendlet conceals them all in the right click menu, while Big Bend exposes them all, and lets you control many via CV. If virtual rack space usage isn't a concern, use Big Bend. 

The big knob only springs back if you control it with your mouse. To use this module with a physical pitchbend wheel, plug in a cable from a MIDI-CV's PW output to the module's PW input.    
Controlling the knob with MIDI automation makes it spring back once it stops receiving input: to obtain smooth results with a device such as [stoermelder ReMOVE Lite](https://github.com/stoermelder/vcvrack-packone/blob/v1/docs/ReMove.md), set it to the highest sampling rate you can.

The modules are polyphonic, and apply the same settings to every note received. Split the channels and use multiple instances for interesting effects!

Neato party tricks you simply must attempt with these modules:

- **Quantized bend**: make pitchbend expressed in scale degrees rather than semitones. Always end up in tune when you push the bend all way up and down!
- **Quantized output**: quantize the input so it matches a scale, quantize the output to get a  glissando with discrete steps, or do both if you do not know fear. (Bonus: you can use the module purely as a quantizer, if you like its color more than the 26 other quantizers available.)
- **Huge range**. Up to 120 semitones (±5V). Do I sound like I'm joking? I'm not joking, punk. I'm giving you a whole 120 semitones pitchbend. I'm that overly literal genie giving you exactly what you ask for without regard for the consequences. With great range comes great irresponsibility.

If you don't plug in anything to the V/Oct input, it centers on zero, letting you use the module purely as a knob that springs back in place. 

The quantizer employs sacred "Twelve tone equal temperament" mathematics.



Splort, Smerge, Spleet, Swerge, and Splirge
-------------------------------------------

![Splirge](/doc/splirge.png)

A colection of tiny cute polyphonic split and merges, with a neato trick: they can sort channels by voltage! They get sorted from lowest to highest. It's meant to be used to craft chords, for neat freaks who want their values in order, and for the advancement of science in general. I can't wait to see how you put them to use. 

- **Splort**: 5hp **sorting 16 channel split** 
- **Smerge**: 5hp **sorting 16 channel merge ** 
- **Spleet**: 3hp **sorting dual 4 channel split**.
- **Swerge**: 3hp **sorting dual 4 channel merge**.
- **Splirge**: 3hp **sorting 4 channel split and merge**. Saves you 7hp over using VCV Fundamental's Split and Merge if you just need four channels!

The sorting is disabled by default. When enabled, channels are continuously sorted, from lowest to highest. 

Splort and Smerge, the 16 channel modules, have two features omitted from the smaller modules: 

- **Link sort order**: daisy chain devices to make them all sort channels in the order of the first device in the chain. Useful to sort notes and gates together.
- **Trigger sort**: Sample & Hold sort order. When sorting is enabled and a cable is plugged in to this input, sort order is only updated when it receives a 10V gate or trigger.

On the 3hp devices, there's no linking, no sort trigger, and the sort button affects both banks - keeps things simple.

The word "Splirge" seems to have been coined by the late Håkan Müller as part of [a Reason Rack Extension](https://www.reasonstudios.com/shop/rack-extension/mxsplirger-cv-flexible-split-merge/), and I think it's a cool word. But you'll have an easier time identifying the devices by their visual design than their name: splits have a darker background, since they are comprised mostly of outputs. 

Advanced tip, for cool kids only, here's how the link feature works internally: it's a polyphonic cable with all 16 channels used, and each channel is set to either -1V (it means this channel isn't connected), or to a multiple of 0.1V to specify its order (so the first channel in sort order is 0.1V, the 12th one is 1.2V, etc). It expects precise values, and no duplicates. If fed random voltage, behavior will be erratic. 


Installation
------------
Those modules are part of the [VCV plugin library](https://vcvrack.com/plugins.html), this is the easiest way to install them. 


Lawyer's corner
---------------
The Aria Salvatrice VCV Rack modules are distributed under the Do What The Fuck You Want To Public License version 2. They come without any warranty and might recklessly endanger life and limb, the usual.

If you fork my modules for some bizarre reason, don't keep my signature on the faceplates, I don't want to sign your work. You can easily do it by blanking or altering the `res/components/signature.svg` file.


Acknowledgements
----------------
Thanks to Andrew Belt for creating VCV Rack and providing code suggestions. Thanks to everyone who created open-source modules I could learn from. Thanks to you for using my modules.


Contact
-------
Send me questions and dog gifs to <woof@aria.dog>. 

ttyl,

![Aria Salvatrice](/doc/signature.png)
