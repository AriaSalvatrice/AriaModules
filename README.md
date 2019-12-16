Aria Salvatrice's VCV Rack Modules
==================================

Hiya hello hey hi. These here are a buncha cool modules what for [VCV Rack](https://vcvrack.com/). By buncha I mean there's one so far, but maybe one day there might be like over three of them. Yay.


Modules (pluralized so I don't have to change the readme once I make more stuff)
--------------------------------------------------------------------------------

<!--
>>> THESE MODULES ARE IN DEVELOPMENT <<<

### Big Bend and Bendlet ###

![Big Bend and Bendlet](/doc/big-bend.png)

**Polyphonic quantizing pitchbend helper**. Add it between your MIDI keyboard and your oscillator to get some dope glissandi and portamenti (it's polyphonic, so I get to use the fancy Italian plural, deal) or play it with your mouse: the knob quickly springs back in place, like on real hardware. 

These modules provide a zero setup building block present on most keyboard synths, and I hope you will also find horrific ways to misuse them for unintended purposes.

Both modules offer the same features. Bendlet conceals them all in the right click menu, while Big Bend exposes them all, and lets you control many via CV. If virtual rack space usage isn't a concern, you should pick Big Bend. 

The big knob only springs back if you control it with your mouse. To use this module with a physical pitchbend wheel with its own spring, plug in a cable to the PW input. Controlling the knob with MIDI automation makes it spring back once it stops receiving input: to obtain smooth results with a device such as [stoermelder ReMOVE Lite](https://github.com/stoermelder/vcvrack-packone/blob/v1/docs/ReMove.md), set it to the highest sampling rate you can.

The modules are polyphonic, and apply the same settings to every note received. Split the channels and use multiple instances for interesting effects!

Neato party tricks you simply must attempt with these modules:

- **Quantized bend**: make pitchbend expressed in scale degrees rather than semitones. Always end up in tune when you push the bend all way up and down!
- **Quantized output**: quantize the input so it matches a scale, quantize the output to get a  glissando with discrete steps, or do both if you do not know fear. (Bonus: you can use the module purely as a quantizer, if you like its color more than the 26 other quantizers available.)
- **Huge range**. Up to 120 semitones. Do I sound like I'm joking? Well, I'm not joking, punk. I'm giving you a whole 120 semitones pitchbend. I'm that overly literal genie giving you exactly what you ask for without regard for the consequences. With great range comes great irresponsibility. 

The quantizer is employing sacred "Twelve tone equal temperament" maths: it sounds the way pretty much every instrument is already tuned.    
Literati of the forbidden notes are asked to either use their own tools, or to hit me with a pull request implementing their mystical xenharmonic hermeticism properly. 
-->

### Splirge ###

![Splirge](/doc/splirge.png)

Splirge is cute little **four channel polyphonic split and merge** that slots anywhere you can spare 3hp. When you don't need all 16 channels, Splirge will save you 7hp over using VCV Fundamental's Split and Merge.

The word "Splirge" seems to have been coined by the late Håkan Müller as part of [a Reason Rack Extension](https://www.reasonstudios.com/shop/rack-extension/mxsplirger-cv-flexible-split-merge/), and I think it's a cool word.

<!-- 
THIS FEATURE IS IN DEVELOPMENT
Secret cheat code for cool kids only: there's a right-click option to sort the channels by voltage, lowest to highest. Need to do that on 16 channels? Check out **Splort & Smerge** instead. 
-->


<!--
THESE MODULES ARE IN DEVELOPMENT

### Splort & Smerge ###
**Polyphonic split and merge, neatly sorted by voltage.** You can order channels from highest to lowest, lowest to highest, or swap them around randomly. Meant to be used for chords, but I trust you to have better ideas how to use them.

In random mode, channels are swapped around, and swapped again when you send a trig to the input conveniently named "trig". It uses the same amount of channels as the highest number in use. Use a dummy cable to use all 16 channels.

Only need 4 channels sorted lowest to highest? Try out the tiny kawaii **Splirge** module instead. 
-->




Lawyer's corner
---------------
The Aria Salvatrice VCV Rack modules are distributed under the Do What The Fuck You Want To Public License version 2. It comes without any warranty, the usual.

If you fork my modules for some bizarre reason, remove my signature from the faceplates, I don't want my signature on your work. You can easily do it by blanking or altering the `res/components/signature.svg` file.    
If you could also be nice enough to alter the color scheme that'd rule, but if you can't be bothered, eh. 



Contact
-------
Send me questions and dog gifs to <woof@aria.dog>. 

ttyl,

![Aria Salvatrice](/doc/signature.png)
