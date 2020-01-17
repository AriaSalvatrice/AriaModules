Aria Salvatrice's Signature Series VCV Rack Modules
===================================================

Hiya hello hey hi. These here are a few cool and nice modules what for [VCV Rack](https://vcvrack.com/). I hope you like them.



Split and Merge: Splort, Smerge, Spleet, Swerge, and Splirge
------------------------------------------------------------

![Split and merge](/doc/split.png)

A collection of tiny cute polyphonic splits and merges, with a neato trick: they can sort channels by voltage! It's meant for neat freaks who want their values in order, and for the advancement of science in general. I did not make them because a use case exists. I made them because I want to see what kinds of use cases you will come up with. 

- **Splort**: _16 channels polyphonic split with optional chainable sort mode._ 5hp.
- **Smerge**: _16 channels polyphonic merge with optional chainable sort mode._ 5hp.
- **Spleet**: _Dual 4 channels / single 8 channels polyphonic split with optional sort mode._ 3hp. If nothing is plugged in the second input, the 4 outputs of the second bank act as channel 5 to 8 of the first input. 
- **Swerge**: _Dual 4 channels / single 8 channels polyphonic merge with optional sort mode._ 3hp. If nothing is plugged in the first output, both banks act as a single 8 channel merge on the second output.
- **Splirge**: _4 channels polyphonic split and merge with optional sort mode._ 3hp. When nothing is plugged in the split input, it's internally connected to the merge output, so you can use the module only to sort values. Saves you 7hp over using VCV Fundamental's Split and Merge if you just need four channels! 

The sorting is disabled by default on all devices. Once enabled, channels are continuously sorted by voltage, from lowest to highest. Don't limit yourself to feeding them CV, send them audio and listen what happens! You can create hybrid waveforms by sending them multiple oscillators. On the 3hp devices, the sort button affects both banks.

Splort and Smerge, the 16 channel modules, have a special feature: they can **Link the sort order**. Daisy chain Splort and/or Smerge devices to make them all sort their channels in the same order as the first device in the chain. Useful to sort notes and gates together. Link has no effect when the "Sort" button is disabled, but if there's a link input plugged in, the output will still forward it.

The word "Splirge" seems to have been coined by the late Håkan Müller as part of [a Reason Rack Extension](https://www.reasonstudios.com/shop/rack-extension/mxsplirger-cv-flexible-split-merge/), and I think it's a cool word. The rest of the names were crafted by a consortium of expert linguists and personal branding gurus. 

**Protip for cool kids only:** here's how the link feature works internally: it's a polyphonic cable, each channel is set to either 0V (means the channel isn't connected), or to a multiple of 0.1V to specify its order (so the first channel in sort order is 0.1V, the 12th one is 1.2V, etc). It doesn't expect specific values, just for them to be in the correct order. Try out a polyphonic sample and hold on the Link cable! 




Darius Branching Step Sequencer
-------------------------------

![Darius](/doc/darius.gif)

Are you still looking for the perfect sequencer, the one that allows you to effortlessly express any musical idea? 

Sucks. 

Here comes yet another gimmick module challenging you to integrate its bizarre ideas to your song instead.

**Darius** is a 8-step sequencer where each node branches into two possible paths, creating repeating patterns that start similarly and resolve differently. Takes a whole 32hp of space to fit all its knobs. 

To get started, patch in a clock, randomize the CV, and patch the main output.

On each node:

- **CV Knob** (left): sets the CV for that step (0V~10V)
- **Random route knob** (right): alters the probability to pick the top or the bottom node on the next step. Arrow pointing to the right means 50/50. 
- **Output**: when active, passes through the clock input (both gates and trigs work), or sends 10V continuously if no step input is plugged in.

You can randomize the CV and the routes separately. If you leave all the **Random route** knobs to the default, the pattern will naturally end up towards the center most of the time. Use this information wisely, or don't. 

Darius is named after the eponymous arcade shoot-em-up game series, known for its surreal visuals, its fish-themed enemies, its [unique soundtracks](https://www.youtube.com/watch?v=6FEdlAL3bX0), its multi-display arcade cabinets, and for allowing the player to select their route through the game via a branching map. For the most authentic experience possible, set the pattern length to 7 and write your song in 7/8 time.

![Darius Gaiden](/doc/darius-gaiden-map.png)

I guess the module is also technically named after some dead Persian guy who did some King stuff, I heard on Wikipedia, the free encyclopedia.




Signature Series Blank Plate
----------------------------

![Blank Plate](/doc/blank.png)

A complimentary 8hp blank plate and ♥-head screwdriver are provided with every Signature Series module purchase.




Installation
------------

Those modules are part of the [VCV plugin library](https://vcvrack.com/plugins.html). This is the easiest way to install them and keep them up to date. You can also do things the hard way and [build them yourself](https://vcvrack.com/manual/Building#building-rack-plugins) if you have a good reason to do so, for example, if you hate yourself, or if you enjoy building random C++ projects off github as a hobby.




Acknowledgements
----------------

Thanks to everyone who created open-source modules I could learn from. Thanks to Andrew Belt for creating VCV Rack and providing code suggestions. Thanks to my dog for no particular reason. Thanks to you for using my art.




Lawyer's corner
---------------

The code and graphics Aria Salvatrice VCV Rack modules are distributed under the Do What The Fuck You Want To Public License version 2. They come without any warranty and might recklessly endanger life and limb, the usual.

Regarding my signature/logo, there is no legal requirement, but a simple matter of courtesy: if you edit my code to use my modules as a base for your own altered modules, remove my signature from the faceplates, even if you think your changes are trivial, I don't want to take credit for something I didn't get a chance to vet.    
The easiest way to remove my signature is by blanking or replacing the graphic in the `res/components/signature.svg` file.




Contact
-------

Send me questions and dog gifs to <woof@aria.dog>. 

ttyl,

![Aria Salvatrice](/doc/signature.png)







<!-- SPOILER WARNING

THESE MODULES ARE IN DEVELOPMENT 

DO NOT READ

Pitch Bend: Big Bend and Bendlet
--------------------------------

![Big Bend and Bendlet](/doc/bend.png)

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



Pitch Bend: Bendlet
-------------------

![Big Bend and Bendlet](/doc/bend.png)

**Polyphonic pitchbend helper**. Add it between your MIDI keyboard and your oscillator to get some dope portamenti (it's polyphonic, so I get to use the fancy Italian plural, deal) or play it with your mouse: the knob quickly springs back in place, like on real hardware. 

This module provides a zero setup building block of a keyboard synth, and I hope you will also find horrific ways to misuse it for unintended purposes.

Bendlet conceals a few tricks in its right click menu, all of them self-explanatory.

The big knob only springs back if you control it with your mouse. To use this module with a physical pitchbend wheel, plug in a cable from a MIDI-CV's PW output to the module's PW input.    
Controlling the knob with MIDI automation in not recommennded.

The module is polyphonic, and applies the same settings to every note received. 

This is the first module in the pitch bend suite - a version with advanced quantizing features is in development.


-->
