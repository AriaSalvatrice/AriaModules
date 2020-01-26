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

To get started immediately, patch in a clock to the step forward input, randomize the CV, patch the main output to an oscillator, and listen what happens.

On each node:

- **CV Knob** (left): sets the CV for that step, (0V~10V, or -5V~5V, depending on the position of the **Unipolar/Bipolar** rocker switch).
- **Random route knob** (right): alters the probability to pick the top or the bottom node on the next step. Arrow pointing to the right means 50/50. 
- **Output**: when active, passes through the clock input (both gates and trigs work), or sends 10V continuously if no step input is plugged in.

You can randomize the CV and the routes separately. If you leave all the **Random route** knobs to the default, the pattern will naturally end up towards the center most of the time. Use this information wisely, or don't. 

Darius is simple and limited, by design. It is meant to be easy to learn and to use. Many of its apparent limitations can be bypassed with a bit of creative patching.

Darius is named after the eponymous arcade shoot-em-up game series, known for its surreal visuals, its fish-themed enemies, its [unique soundtracks](https://www.youtube.com/watch?v=6FEdlAL3bX0), its multi-display arcade cabinets, and for allowing the player to select their route through the game via a branching map. For the most authentic experience possible, set the pattern length to 7 and write your song in 7/8 time.

![Darius Gaiden](/doc/darius-gaiden-map.png)

I guess the module is also technically named after some dead Persian guy who did some King stuff, I heard on Wikipedia, the free encyclopedia.




Arcane, Atout & Aleister - Today's Fortune ★
--------------------------------------------

![Arcane](/doc/arcane.png)

I hope luck is on your side. Every day, you will share together the fortune I shall grant you. It is your task to interpret what my augury means to you.

Modules in the Arcane series are comprised almost only of output jacks, sending today's fortune as CV. **Those values are the same for every VCV Rack user**, no matter their platform, and you can't predict or influence them. I pronounce a new oracle every day at 12:00 AM UTC without fail.

I will draw the following cards, and impart the following knowledge:

- **_Arcane Majeur_** - a major arcana of the Tarot of Marseilles.
- **_Bâtons_** (Wands), **_Coupes_** (Cups), **_Deniers_** (Coins), **_Épées_** (Swords) - a different 16 step binary pattern for each suit of the Tarot.
- **Scale pattern** - The notes of a music scale comprised of 5, 6, 7, or 8 notes, often scales common in popular Western music, and sometimes less common ones. Which note is the root of the scale isn't specified, only which notes are part of the scale.
- **8 note pattern** - Every note from the selected scale, sorted in a random order. If the scale has fewer than 8 notes, up to 3 different notes will be repeated at the end, an octave higher.
- **BPM** - Integer ranging from 60 to 180 BPM, with values between 90 and 150 twice as likely.
- My **sincerest wish** of luck, love, health, and prosperity.

How to interpret my fortune is up to you and your friends. You can make use of a single output jack of the module, or all of them. You can craft an ephemeral song that will no longer exist tomorrow. You can create an unchanging patch that will grace you with a new song every day. There are no rules but the ones you choose to follow. 

All I can do is reveal some mundane obscura to get you started scrying the will of the jacks:

- **Quantizer** - A pair of polyphonic ports quantizing input to the nearest scale note.    
Try to combine it with the [Split and Merge series](#split-and-merge-splort-smerge-spleet-swerge-and-splirge), the ability to sort by voltage might prove useful. 
- **Scale notes** - Polyphonic ports sending every note of the scale on the fourth octave (0V~0.9166667V), in the order of the 8 note pattern.    
The left jack named "Scale" sends as many notes as there are in the scale (between 5 and 8). The second jack named "Padded" sends the entire 8 note pattern, with any repeated note being one octave higher than normal.     
If you combine them with [ML_modules sequential switch 8->1](https://github.com/martin-lueders/ML_modules/wiki/Sequential-Switches), they can become a melody.
- **_Arcane Majeur_** - The number of the arcana, divided by 10.    
Thus, _The Fool_ is 0V, _The Magician_ is 0.1V, and so on up to 2.1V for _The World_. Following the conventions of the Tarot of Marseilles, the 8th arcana is Justice, and the 11th is Strength, unlike the ordering popularized by the Rider-Waite-Smith deck, which swaps those two arcana around.    
You can multiply this signal by 4.76 with an offsetter such as [Bogaudio's](https://github.com/bogaudio/BogaudioModules#offset) to obtain a 0V~9.99V signal, if you wish.
- **BPM** - Follows the [VCV Standard](https://vcvrack.com/manual/VoltageStandards#pitch-and-frequencies) for clocks and LFOs where 0V stands for 120BPM.    
You can send that signal to a compatible clock module such as [Impromptu Modular's Clocked](https://marcboule.github.io/ImpromptuModular/#clocked) if you require a clock (for example, for swing, or other time signatures than 4/4).
- **Reset** and **Run** - They control all the outputs below them. Operate them manually using the button, or synchornize multiple devices by sending them a trigger from a single source, for example the [Little Utils Button](https://github.com/mgunyho/Little-Utils#button).
- **Gate/Ramp** on the **32nd note**, **16th note**, **8th note**, **4th note**, **Bar** - Sends, depending on the position of the **Gate/Ramp** switch, either a gate synchronized to the BPM at the corresponding interval in 4/4 time, or a 0V-10V ramp corresponding to that phase.    
While **Arcane** can serve as a master clock, the ramp can be used to drive the [ZZC Clock](https://zzc.github.io/en/clock-manipulation/clock/), if you'd prefer to use it.
- **_Bâtons_**, **_Coupes_**, **_Deniers_**, **_Épées_**. The four patterns, sent as gates on each 32nd note, 16th note, 8th note, 4th note, or bar.    
Don't limit yourself to drums, you can use them in many different ways! Want to visualize the patterns, or to use them as something different than a rhythm? Try out the **Aleister** expander.
- The **Pulse Width** knob lets you select how long the gates are, proportionally to their note length. It affects every gate output. Need more granular control over pulse widths? An external clock or a pulse generator such as [Submarine's](https://github.com/david-c14/SubmarineFree/blob/master/manual/PG.md) will help.

The module's **LCD** will show you the date of the fortune, the BPM, the name of the arcana, and the notes of the scale. However, it cannot unambiguously name the scale, since your fortune doesn't stipulate on which note the scale starts.

The **LCD** will also tell you whether today I am wishing you luck, love, health, or prosperity (displayed as MONEY due to the size of the LCD). You cannot access that information via CV, as it is only a personal wish that might not manifest, unlike my oracles, which are always accurate.

There are two available form factors of the same module:

- **Arcane**: the full 24hp version displays today's arcana, from Nicolas Conver's 1760 century Tarot of Marseilles.
- **Atout**: the smaller 9hp version doesn't display the arcana, but includes all the functionality and every jack from **Arcane**. To conserve space, the **Gate/Ramp** switch is at the bottom.

The third module, **Aleister**, gives you access to the **_Bâtons_**, **_Coupes_**, **_Deniers_**, and **_Épées_** binary patterns as series of 16 outputs sending continuously either 0V or 10V, rather than as a rhythmic pattern of gates. If you connect only the first output of a group, it will instead be a polyphonic cable outputting the entire group. **Aleister** takes 14hp of space. You can employ **Aleister**'s services as a standalone module, but when you placed directely the right of either **Arcane** or **Atout**, the module will act as an expander, lighting up the jacks in sync with the rightmost connected output of the corresponding pattern on the parent module. Try it out, you'll get what it does immediately.

If the module is active at the time a new fortune is drawn, the values will not change, but a notification a new fortune is available will appear on the **LCD**. Using the right-click menu, you can _Initialize_ the module to download the newest fortune. Remember to also reinitialize **Aleister** if in use. // FIXME - Reinit should force a download!

**The tarot deck used is the Conver – Ben-Dov (CBD) version**. The CBD deck was restored in 2008-2011 from the original 18th century deck by tarot expert [Dr. Yoav Ben-Dov's](https://www.cbdtarot.com/).    
In traditional Tarot of Marseilles imagery (not just in this specific deck), some figures are frequently depicted nude. While the style of the CBD deck is neither graphic nor intended to titillate, I took the liberty to slightly alter a few cards to remove details, as most users will download my plugin without being forewarned about contents a small minority would object to. I hope you will understand this decision.

**Upon activation, this module will connect to the internet**, and fetch today's numbers on a GitHub repository via HTTP. The fortunes are not generated locally to make it possible for all users to share the same numbers every day, while also making it impossible to cheat fate and predict the next oracle (which a local deterministic implementation would necessarily entail).

If the repository is unreachable, the module will will output 0V on all ports, except the quantizer, which will forward input as-is. [You can find more information about Arcane server and API, and their source code, on their GitHub repository](https://github.com/AriaSalvatrice/Arcane).

**There is no offline mode, and no way to load older fortunes, by design.** These are multiplayer-only oracles. But I won't prevent you from editing the .json files in the `AriaSalvatrice/Arcane/` directory of your VCV Rack user directory, if you really must. Every downloaded fortune is archived, and if a local file exists for today, it will not be checked against the server.     
(Quick tech remark about JSON API cache spoofing: such self-deception will never alter your true destiny, and it is unwise to cling to the past instead of living for a future you can yet change.)

Using this series of modules to their full extent requires a bit of creative patching, a bit of lateral thinking, but most importantly, friends to share your different interpretations of the same fortune with.    
If you treat my oracles as a mere random number generator, you will never gain any wisdom from them.




UnDuLaR Rack Scroller
---------------------

![UnDuLaR](/doc/undular.png)

Scroll your rack via CV! This is particularly useful for live performance: build a rack that is no wider (or no taller) than your screen at your preferred zoom level, and connect only the X or the Y control to a MIDI knob. 

- **UP/DOWN**: This pair of inputs take trigs, and scroll the rack up or down by 3U - the height of one module.
- **LEFT/RIGHT**: This pair of inputs take trigs, and scroll the rack up or down by 32hp - the width of one module, assuming the module of question happens to be exactly 32hp. 
- **X**: This input takes 0V~10V, and scrolls your rack horizontally, adapting to its current dimensions.
- **Y**: This input takes 0V~10V, and scrolls your rack vertically, adapting to its current dimensions.
- **Zoom**: This input takes 0V~10V, and zooms your rack in and out from 25% to 400%. Zooming in and out via CV may or may not be fast enough to be usable for you. You try it out. 

You can still scroll around normally when using the module. If you want the module to fully take over scrolling, there's a few dangerous right-click options that do that. It's useful to avoid auto-scrolling when you drag a cable to the edge of the screen. 

Be sure to attempts all sorts of idiotic party tricks you will regret such as connecting an oscillator to the controls, forcing you to use `Engine > Sample Rate > Pause` before the onset of the seizure.

Thanks to [Frank Buss' Shaker](https://github.com/FrankBuss/FrankBussRackPlugin) for demonstrating it's possible to scroll the rack via a module.




Signature Series Blank Plate
----------------------------

![Blank Plate](/doc/blank.png)

A complimentary 8hp blank plate and ♥-head screwdriver are included with every Signature Series module purchase.




Installation
------------

Those modules are part of the [VCV plugin library](https://vcvrack.com/plugins.html). This is the easiest way to install them and keep them up to date. You can also do things the hard way and [build them yourself](https://vcvrack.com/manual/Building#building-rack-plugins) if you have a good reason to do so, for example, if you hate yourself, or if you enjoy building random C++ projects off github as a hobby.




Acknowledgements & Credits
--------------------------

Thanks to everyone who created open-source modules I could learn from. Thanks to Andrew Belt for creating VCV Rack and providing code suggestions. Thanks to my dog for no particular reason. Thanks to you for using my art.

The modules use the following fonts:
- [Francois One](https://fonts.google.com/specimen/Francois+One) by [Vernon Adams](http://sansoxygen.com/) 
- [Nova](https://fontlibrary.org/en/font/nova) by [Wojciech Kalinowski](https://fontlibrary.org/en/member/wmk69) 
- [Fixed_v01](http://www.orgdot.com/aliasfonts/index.htm) by [Orgdot](http://www.orgdot.com/aliasfonts/index.htm)

The **Arcane** module uses Tarot cards from [Yoav Ben-Dov's CBD Tarot](https://www.cbdtarot.com/).



Lawyer's corner
---------------

The code of the Aria Salvatrice VCV Rack modules is distributed under the Do What The Fuck You Want To Public License version 2. They come without any warranty and might recklessly endanger life and limb, the usual.

The graphics are also distributed under the WTFPL, save for the following exceptions:

- My signature/logo remains copyrighted.
  - If you edit my code to use my modules as a base for your own altered modules, remove my signature from your faceplates, even if you think your changes are trivial: I don't want to endorse and take credit for something I didn't vet or personally participate in.
  - If you are faithfully porting my code to a fork of VCV Rack, are compiling binaries for another platform, or are otherwise faitfully distributing my modules as I designed them, you can leave in the signature. If unsure, just ask.
  - The easiest way to remove my signature from every module is by blanking or replacing the graphic in the [`res/components/signature.svg`](res/components/signature.svg) file, and removing it from the blank plate: [`res/Blank.svg`](res/Blank.svg).
- The files in `res/Arcane` are all distributed under the [CC-BY-NC-SA](https://creativecommons.org/licenses/by-nc-sa/3.0/deed.en_US) license, as they are a derivative work of the [CBD Tarot](https://www.cbdtarot.com/) released under that license.
- The font Fixed_v01 in `res/Fixed_v01` is distributed under a [custom permissive MIT-style custom license](FIXME). The license file is included with the distributed plugin to comply with its terms.

You may freely distribute alternate faceplates for my modules. You will find information about my design language and design process in [`doc/design.md`](doc/design.md).




Contact
-------

Send me questions and dog gifs to <woof@aria.dog>. 

ttyl,

![Aria Salvatrice](/doc/signature.png)
