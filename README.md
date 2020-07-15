Aria Salvatrice Signature Series Synthesizer Modules
====================================================

Hiya hello hey hi. These here are a few cool and nice modules what for [VCV Rack](https://vcvrack.com/). You can download them from [its built-in library](library.vcvrack.com/).

I am some persuasion of multimedia artist interested in generative processes, creating [music](https://ariasalvatrice.bandcamp.com/releases) in genres I've come to call "_Gay Baroque Technopop_" and "_Pastoral Industrial_". All these synthesizer modules were designed for my own use first. With this collection, I aim to create an integrated, coherent system of opinionated, unique modules, primarily concerned with the live performance of aleatoric techno. Despite their focus, these modules are versatile, and integrate well with the wider VCV ecosystem. Many artists enjoyed using my modules in all sorts of music, and I hope you will like them too.

This collection of modules is free software. If you wish to send me a tip for my modules, at this time, donations can only be accepted in the form of [pay-what-you-want album purchases](https://ariasalvatrice.bandcamp.com/releases). I am looking into providing additional donation options in the future.

----

Modules documentation
---------------------

- [Split and Merge: Splort, Smerge, Spleet, Swerge, and Splirge](#split-and-merge-splort-smerge-spleet-swerge-and-splirge)
- [Quatherina the Quantum Duck presents Quatherina's Quality Quad Quantizer](#quatherina-the-quantum-duck-presents-quatherinas-quality-quad-quantizer)
- [Quatherina's Quale](#quatherinas-quale)
- [Darius Branching Step Sequencer](#darius-branching-step-sequencer)
- [Arcane, Atout & Aleister - Today's Fortune ★](#arcane-atout--aleister---todays-fortune-)
- [UnDuLaR Rack Scroller](#undular-rack-scroller)
- [Signature Series Blank Plate](#signature-series-blank-plate)


Other thingies
--------------

- [Changelog](CHANGELOG.md)
- [Installation](#installation)
- [Acknowledgements & Credits](#acknowledgements--credits)
- [Lawyer's corner](#lawyers-corner)
- [Contact](#contact)
- [Design language of the modules](doc/design.md)
- [Future plans for the collection](doc/plans.md)
- [How to contribute](CONTRIBUTING.md)
- [Beta / pre-release thread on VCV forums](https://community.vcvrack.com/t/arias-cool-and-nice-thread-of-barely-working-betas-and-bug-squashing/8208)



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




Quatherina the Quantum Duck presents Quatherina's Quality Quad Quantizer
------------------------------------------------------------------------

![QQQQ](/doc/QQQQ.png)

Quatherina the Quantum Duck is best known for her aventures in the Quartz Quasar, in the Quaint Quarry, in the Quarrelsome Quahaug Quagmire, in the Quarantined Quaestor Quadripoint, and in Canada (the canonicity of that last adventure being a point of debate amongst Quatherina's fans that I wish to remain neutral about).    
To finance her adventuring lifestyle, she has graciously accepted payment (in the form of an undisclosed tonnage of Quaker Oats) to lend her name and likeness in endorsement of my synthesizer modules.

**Quatherina the Quantum Duck presents Quatherina's Quality Quad Quantizer** is a quantizer: it coerces any input signal into the closest pitch of the scale, and outputs it as a V/Oct signal. It only works with Twelve-tone Equal Temperament, the usual Western tuning system. It provides four separate quantizer lines, each with a built-in scaler and offsetter to change the range of the signal, the ability to transpose the signal, and sample & hold. You can save scales and chords in 16 slots.


### Form factors

There are three different form factors available of the same underlying module:

- **Quatherina's Quality Quad Quantizer** (or **QQQQ**): The full 20hp version, featuring piano buttons, a helpful LCD, 4 polyphonic quantizer columns, 16 scenes memory, and a high definition portrait of Quatherina.
- **Quack**: This smaller 7hp version only includes piano buttons, scale/key knobs, and a single polyphonic quantizer column from **QQQQ**.
- **Q<**: 3hp. External input. Tiny.

Those modules are all expanders of each other: the left one sends the scale to the right one, which forwards it to its own right. So if you only require two quantizer lines and no sequencer, you can use **Quack** on the left and **Q<** on the right, to use only half the size of **QQQQ**. If expander behavior is not desired, you can leave a gap between the instances.

In this documentation, we're focusing on **QQQQ**: once you understand it, you'll immediately understand how to operate the smaller versions.


### Setting the scale

The scale of the device can be set from multiple sources, and **the last source to make a change always has the last word what's the scale**.     
The possible sources of change are:

- Pressing the Piano keys
- Turning the Scale/Key knobs
- Receiving a scale from the device immediately to the left
- Receiving a scale from the external input
- Changing the scene
- Importing a chord progression or portable sequence with the keyboard button

All the quantizer columns on a module follow the same scale. And while in this documentation I call them scales for simplicity, **QQQQ** also works with chords (albeit, without knowledge of their intended voicing), and can be a fantasatic arpeggiator. 

The **Piano keys** always show you the current scale. Keys lit in yellow are part of the scale, while unlit keys are disallowed. You can click on the keys to change which notes are allowed. When you click the **visualize** button of a quantizer line, the notes currently playing on that line are lit pink (with a small dot in the center for accessibility).

In the **LCD Area** at the top of the device, the **Scale** and **Key** knobs select a preset scale, and its key.     
The available scales were curated for ease of use and instant satisfaction rather than for comprehensiveness, avoiding duplication in the form of scales that can be expressed as a mode of another scale (for your convenience, an exception is made for Natural Minor and Pentatonic Minor). The following scales, along with my qualified opinion about them, are available:

- **Chromatic** - All the notes, all the time! It's a great deal if you like every note.
- **Major** - The white keys of the piano! I'm not really sure why we need anything else, but hey, your call if you want something daring and spicy.
- **Natural Minor** - It is the exact same thing as the major scale, but it has a different "tonic" making it more suitable for being sad. Yup, just a scam to sell you the same stuff as something new.
- **Melodic Minor** - You can use it to write melodies! Before this scale was invented, music had no melodies.
- **Harmonic Minor** - The fancy, high-culture, bourgeois minor scale, used by refined individuals. That shit slaps.
- **Pentatonic Major** - It's the guitar scale! And it has only 5 notes! Yet guitars have 6 strings. How does it even work? Theory peeps must have felt really stupid after they came up with that one.
- **Pentatonic Minor** - Same hustle as with natural minor: it's the exact same notes as major, but their order shuffled around a bit, repackaged with an edgy sad brand.
- **Whole Tone** - This one has structure! It is for fans of symmetry and kaleidoscopes and fractals and putting a fibonacci spiral overlay on a renaissance painting. 
- **Blues Major** - It's for when you got the blues, but aren't that sad about it.
- **Blues Minor** - That one is for special occasions.
- **Dominant Diminished** - That bad boy makes Honked up Hot Jazz .
- **Bebop Major** - This one has a "blue note", like those albums with the cool fonts on the cover.
- **Bebop Minor** - This one is for fans of animé. You're welcome.
- **Double Harmonic** - It's so harmonic, cultures from all over the world claim this one as theirs. Maybe some day France will come up with a triple harmonic one.
- **Eight Tone Spanish** - When I tried to find more useful info about it, I found a forum post from 2003, where a poster said indignantly that scales "[are] not a democracy, but a hierarchy".
- **Hirajōshi** - It's all you need to rip a sick shamisen solo. Anything more is for poseur kids.
- **In Sen** - That one was originally used for Japanese wind chimes. Instantly become the most original dark ambient modular artist around by feeding it to [Rings and Clouds](https://vcvrack.com/AudibleInstruments) for 3 hours.

The **External Scale** input and output can encode a scale as a 12-channel polyphonic cable, where enabled notes have a continuous 10V signal (anything above 0V works), and disabled notes, 0V. This way of expressing a scale is supported by my other modules, such as **Arcane** and **Darius**. Try out hacking this data bus with my splitters and mergers!

After patching in an unchanging **External Scale**, if you edit the settings from another sources (e.g., turning the scale knob), you can unplug and replug the **External Scale** jack to reload it.


### Scene memory

You can save up to 16 scales in **QQQQ**. The slots are the top right of the module:

- **Scene slot**: Those 16 buttons allow you to save 16 different scales. Navigating to a scene slot loads that scale, and changes to the scale from any source are saved to the active scene. You can right-click on those buttons to copy and paste scales. Mapping the scene slots via MIDI doesn't work well with every mapping module, but you can use the **Scene input** jack if necessary.
- **Scene input**: Navigates the scenes via CV. Useful with a step sequencer! Accepts 0V~10V by default. When plugged in, the buttons can't be operated manually anymore.

The scenes only save the scales: the positions of the knobs are global.


### Keyboard / Clipboard I/O

With the **Keyboard** button, you can write down or paste chords from the clipboard, and have them transformed to **QQQQ** scenes! You can also use it to copy and paste scales as portable sequences. The three following modes are available:

- **Lead sheet notation**: Chord symbols, separated with spaces, commas, or hyphens. It's rather liberal in what it accepts, but kinda chokes on slash chords and crazy jazz combos. Still, it should get you close. For example, this is a valid input: `G am B, E/G# Bb - A,Fsus2  Csus2 D(add9)`
- **Roman numeral notation**: Roman numerals, separated with spaces, commas, or hyphens. The tonic will be set to the current position of the **Key** knob. To denote minor chords, rather than use lowercase, you need an explicit `m` after the numeral: `III im7 VIsus4` rather than `III i7 VIsus4`. It's a lot more fiddly than lead sheet notation and will only recognize rather basic progressions. 
- **Portable Sequence**: Simultaneous notes from a [**Portable Sequence**](https://github.com/squinkylabs/SquinkyVCV/blob/master/docs/clipboard-format.md) are interpreted as a scale. You can also export them as such. Note that when exporting and re-importing a portable sequence, empty scene slots will be skipped. How to make this feature useful is left as an exercise to **Portable Sequences** implementers.

Text input is always imported starting from the first scene. And remember: **QQQQ** only thinks in scales, not in chords. After you import a chord progression, **QQQQ** doesn't care about the voicing of the chord. It's all folded back to a single octave. 

Under the hood, **QQQQ** makes use of [Tonal.js](https://github.com/tonaljs/tonal)' knowledge of chords. Yup, this module runs javascript! Same [QuickJS](https://bellard.org/quickjs/) engine as [VCV Prototype](https://vcvrack.com/Prototype). Of course, the number crunching is efficient C++, javascript usage is limited to what it does best - fuzzy text parsing.

Below the two text input buttons, you will find a high-definition **Likeness of Quatherina**, increasing the appeal of the module through co-branding.

**KNOWN ISSUE**: **QQQQ** crashes when the chord input is over 120 characters long. I am looking into fixing this issue.


### Quantizer columns

There's four of them, and the signal flows through them from top to bottom. Essentially, between input and output, you'll find the sort of small utilities you're likely to want to use with a quantizer.

- **CV Input**: Each input jack is forwarded to the jack to its right if it's unplugged. Useful to process the same signal in different ways. It is not forwarded across expanders. Inputs are polyphonic.
- **% Scale**: Attenuates, inverts, or amplifies the input, from -100% to 300%.
- **Offset**: Adds or removes a fixed voltage to the input, from -10V to 10V.
- **Transpose**: Transposes the signal according to one of the three rules, set by the transpose mode button under the knob.
- **Transpose mode**: Selects one of the three transposition rules for the knob directly above it. Each transposition rule results in output that remains in harmony. The rules are:
    - **Unlit** - _Octaves Mode_: Quantizes the signal first, then transposes it up or down by full octaves.
    - **Yellow** - _Semitones Mode_: Transposes the signal up or down by a few semitones first, then quantizes it. 
    - **Pink** - _Scale degrees Mode_: Quantizes the signal first, then transposes the signal by a specific amount of scale degrees.
- **Sample & Hold / Track & Hold toggle**: Swaps between the two modes for the input directly below it. Unlit is Sample & Hold, lit is Track & Hold.
- **Sample & Hold / Track & Hold input**: When plugged in, instead of operating continuously, the quantizer will Sample & Hold or Track & Hold the signal. When you're quantizing a noisy source of modulation, it's useful to send it the same gate you'd send to the envelope that will play the note in question. The Sample & Hold is not polyphonic: a monophonic gate can drive a polyphonic column. And remember, signals move from top to bottom through the columns: the Sample & Hold only happens after processing everything above it.
- **Visualize**: Shows the quantized output on the piano display.
- **Output** - The quantized signal! Polyphony is defined by the **CV Input**.

### Performance

While there are smaller and simpler quantizers than **QQQQ** available in the VCV library, you will find that **QQQQ** is nonetheless faster than many options with fewer features, thanks to its aggressive sub-sampling and culling of unused outputs: 

- **Sub-sampling**: For almost every single use case, there is no reason for a quantizer to operate at audio rates (which most VCV modules do). Every 32 samples is plenty - that's still faster than once per millisecond at 44,100Hz, making artifacts inaudible.
- **Culling**: When the output of a column is unplugged and its visualization disabled, the column in question does not process input, and thus consumes almost no CPU. Unnecessary calculations are also skipped when Sample & Hold is used.

**Protip for cool kids only:** Did you know that there are a few people out there who do not use Quantizers, and instead choose to spend years to learn something called "Music Theory"? Those people do all their music by reading books and thinking really hard about maths! It's true - check out your local library for more information.




Quatherina's Quale
------------------

![Quale](/doc/quale.png)

Quatherina the Quantum Duck doesn't experience the existence of multiple octaves, and quavers at those who purport to. She makes no distinction between chords and scales. It's all voltage to her. 

With some qualms, she humors your fancies, offering **Quale**: a little 3hp module to convert scales (as expressed in **QQQQ**'s polyphonic data bus format) to chords, and the other way around.

When you patch the **Scale Input**, it's expressed on the **Chord Output** as a polyphonic chord on the fourth octave (0V to 0.91667V).

When you patch in a polyphonic V/Oct signal on the **Chord Input**, it's folded to a single octave and expressed on the **Scale Output** in the **QQQQ** format, compatible with the rest of my collection.

**Quale** can be used as an expander with the entire **Qqqq** family: when **Quale** is placed to the left of **QQQQ**, its **Scale Output** is forwarded to **QQQQ**. When **Quale** is placed to the right of **QQQQ**, the scale from **QQQQ** is expressed as a chord on **Quale**'s **Chord Output**.

**Protip for cool kids only:** Quatherina has recently taken to calling people who claim to subjectively experience the existence of multiple octaves Q-Zombies.



UnDuLaR Rack Scroller
---------------------

![UnDuLaR](/doc/undular.png)

Scroll your rack via CV! UnDuLaR adjusts to the size of your rack without any setup. You can also automate cable tension and opacity.

This is particularly useful for live performance with MIDI gear. While you can bind every control, **I recommend using only one movement input**. Build a rack that is no wider (or no taller) than your screen at your preferred zoom level, bind to MIDI either a trig input pair or a scroll input, then lock the unused axis with its button. You do not want the mental overhead of navigating two dimensions while performing.

- **UP**, **DOWN**, **LEFT**, **RIGHT**: Take trigs, and jump around the rack in the corresponding direction, wrapping around when it reaches the end.     
The default vertical jump is 3U - the height of exactly one module.    
The default horizontal jump is 32hp - the width of exactly one module, so long as the module in question happens to be exactly 32hp.
- **Step X**, **Step Y**: Adjust the distance to jump on each trigger.
- **Scroll X**, **Scroll Y**: Take 0V~10V, and smoothly scrolls your rack horizontally or vertically. You might want to invert the input if it feels more natural for you to operate your MIDI controller in the reverse direction.
- **Padding**: Overshoots the scrolling a little by the specified hp.
- **Zoom**: Takes 0V~10V, and zooms your rack in and out from 25% to 400%. **Zooming in and out in VCV Rack is neither fast nor reliable enough to ever attempt during live performance.** Expect broken graphics, CPU usage spikes, and audio crackles. This is due to VCV's graphics engine, there is nothing I can do to improve the performance.
- **Alpha** (Opacity): Takes 0V~10V, and adjusts the opacity of cables.
- **Tnsn.** (Tension): Takes 0V~10V, and adjust the tension of cables.
- **Lock X/Lock Y**: By pressing those dangerous buttons, UnDuLaR fully takes over scrolling on that axis, preventing scrolling from the scrollbars, scrollwheels, arrow keys, middle click, or from dragging cables at the edges of the screen. It will be a little bit jittery, I can't help it. Those locks will only work if the three corresponding cable inputs for that axis are unplugged.

As a safety measure, UnDuLar does nothing for the 10 first seconds after initialization. If you find yourself unable to scroll back to UnDuLaR, save your patch (yup, seriously), then reload it. You now have 10 seconds to find the Undular module and unplug it. 

Known issue: zooming in and out after locking X or Y won't bring you where you expect, and might allow you to get out of bounds.    
Known solution: don't do that.

You can only use a single instance of UnDuLaR in your patch, as multiple instances would just fight each other for control. Additional instances past the first one do nothing.

**Make sure you triple check the module works to your expectations every single time you're about to take it on the stage**, especially after a Rack update! I'll hear no complaints about botched performances, the module is provided as-is, with **zero guarantee** it will work with your setup, and zero guarantee that it will continue to work in the future. The only guarantee I offer is that zooming will cause you no end of trouble.

Thanks to [Frank Buss' Shaker](https://github.com/FrankBuss/FrankBussRackPlugin) for demonstrating it's possible to do this stuff from a module!

**Protip for cool kids only:** Never hesitate to attempt all sorts of imbecilic party tricks you will regret immediately, such as connecting an oscillator to the controls, forcing you to use `Engine > Sample Rate > Pause` before the onset of the seizure.




Signature Series Blank Plate
----------------------------

![Blank Plate](/doc/blank.png)

A complimentary 8hp blank plate and ♥-head screwdriver are included with every Signature Series module purchase.

**Protip for cool kids only:** Having one or two blank plates in your patch is fine, but if you find yourself having too many of them, it's generally considered a red flag you're doing something wrong. The accepted best practice in the virtual modular community is to make your patch as gratuitously complicated as possible until you reach at least 95% CPU usage. 




Installation
------------

Those modules are part of the [VCV plugin library](https://vcvrack.com/plugins.html). This is the easiest way to install them and keep them up to date. You can also do things the hard way and [build them yourself](https://vcvrack.com/manual/Building#building-rack-plugins) if you have a good reason to do so, for example, if you hate yourself, or if you enjoy building random C++ projects off github as a hobby.

If you build my plugin locally, you have to `make dep` before you `make dist`. 




Acknowledgements & Credits & Other Assorted Namedrops
-----------------------------------------------------

Thanks to everyone who created open-source modules I could learn from and make songs with.

Thanks to Andrew Belt for creating [VCV Rack](https://vcvrack.com/) and providing code suggestions.

Thanks to my music-making LGBBQT internet shitposting gang for introducing me to VCV and modular synthesis as something that's not just for old guys with more disposable income than impetus to write fresh songs.

Thanks to [Jerry Sievert](https://legitimatesounding.com/) and [cschol](https://github.com/cschol) for their help with [QuickJS](https://github.com/tonaljs/tonal) integration.

Thanks to [Squinky Labs](https://github.com/squinkylabs/SquinkyVCV), [Stoermelder](https://github.com/stoermelder/vcvrack-packone), [David O'Rourke](https://github.com/david-c14/SubmarineFree) for technical advice.

Thanks to [Omri Cohen](https://www.youtube.com/channel/UCuWKHSHTHMV_nVSeNH4gYAg) for covering my modules.

Thanks to [Sophie Leetmaa](https://../azure-pipelines.ymlsophieleetmaa.com) and [Ken McAloon](https://whatsinaname.xyz/) for latin language translations.

Thanks to a different [Sophie](https://twitter.com/DreamyDoric) for music theory advice.

Thanks to [Heavy Viper](https://rekkanogotoku.com/) for years of inspiring conversation about synths and music.

Thanks to [Mog](https://github.com/JustMog/Mog-VCV) for Mog.

Thanks to [my dog Ornstein](https://ornstein.neocities.org/) for being a good dog.

**But most of all, thanks to YOU for using my art.**

The modules use the following fonts:
- [Francois One](https://fonts.google.com/specimen/Francois+One) by [Vernon Adams](http://sansoxygen.com/)
- [Nova](https://fontlibrary.org/en/font/nova) by [Wojciech Kalinowski](https://fontlibrary.org/en/member/wmk69)
- [Fixed_v01](http://www.orgdot.com/aliasfonts/index.htm) by [Orgdot](http://www.orgdot.com/aliasfonts/index.htm)

The **Arcane** module uses Tarot cards altered from [Yoav Ben-Dov's CBD Tarot](https://www.cbdtarot.com/).

The **UnDuLaR** module background uses the traditional yagasuri kimono pattern as provided by [ULOCO, UOTOMIZU](https://forallcreators.com/yagasuri-background/).

[QuickJS](https://github.com/tonaljs/tonal) is used in some modules under the terms of the [MIT license](doc/LICENSE_TonalJs.txt)

[TonalJS](https://bellard.org/quickjs/) is used in some modules under the terms of the [MIT license](doc/LICENSE_QuickJS.txt)



Lawyer's corner
---------------

Yeah, the rules are a bit complicated. If they cause licensing incompatibilities (besides the intended virality of the GPL) lemme know. Here goes the breakdown:

### The output of my modules belongs to you

It should go without saying that no sane courtroom would ever humor the idea the output of my modules is original enough to be my copyright -- even that of Arcane, which directly outputs random data obtained from a server under my control.

Since courtooms are rarely sane, **I explicitly relinquish any claim of intellectual propery over the output of my modules**, not that I believe I ever had any. Any song you make with them is yours alone.

### Source code of individual modules

The code of the Aria Salvatrice Signature Series Synthesizer Modules is distributed under the [GNU General Public License v3.0 only](https://spdx.org/licenses/GPL-3.0-only.html). The modules come without any warranty and might recklessly endanger life and limb, the usual.

### Re-usable .hpp libraries

Not that the code is very good, but some libraries I created for my own use are available under the less restrictive terms of the [Do What The Fuck You Want To Public License](https://spdx.org/licenses/WTFPL.html), to allow every VCV creator to do what the fuck they want. The files in question are [`src/components.hpp`](src/components.hpp), [`src/javascript.hpp`](src/javascript.hpp), [`src/lcd.hpp`](src/lcd.hpp), [`src/portablesequence.hpp`](src/portablesequence.hpp), [`src/prng.hpp`](src/prng.hpp), and [`src/quantizer.hpp`](src/quantizer.hpp).

### Faceplates 

 [CC-BY-SA-4.0](https://creativecommons.org/licenses/by-sa/4.0/), with the exception of my signature logo, which is copyrighted, and generally not directly baked into my faceplate SVG files. You may freely distribute your faceplate edits to the VCV community.

### Components (knobs, jacks, etc)

CC-BY-SA-4.0. If you re-use them, I request you do not entirely re-use my signature color scheme in your own modules, but this request is not legally binding. Because I use a limited palette, it's easy to replace most colors in my SVG files using search and replace in a text editor.

### Signature / Logo

Copyrighted. It's mine.

**If you edit my code to use my modules as a base for your own altered modules, remove my signature from your faceplates**, even if you think your changes are trivial: I don't want to endorse and take credit for something I didn't vet or personally participate in.

If you are faithfully porting my code to a fork of VCV Rack, are compiling binaries for another platform, or are otherwise distributing my modules as I designed them, keep the signature. If unsure, just ask.

The easiest way to remove my signature from every module is by blanking or replacing the graphic in the [`res/components/signature.svg`](res/components/signature.svg) file, and removing it from the blank plate: [`res/faceplates/Blank.svg`](res/faceplates/Blank.svg).

### Graphics for the **Arcane** module 

[CC-BY-NC-SA](https://creativecommons.org/licenses/by-nc-sa/3.0/deed.en_US), as they are a derivative work of the [CBD Tarot](https://www.cbdtarot.com/) released under that license. That includes the faceplate, as it uses a pattern taken from that tarot deck.

### LCD Font

SVG export of the [Fixed_v01 font by Orgdot](http://www.orgdot.com/aliasfonts/index.htm), released under [a custom MIT-style license](res/components/lcd/Fixed_v01/LICENSE.txt).

### Copyright assignment

**By sending me pull requests, you assign their copyright to me, allowing me, in perpetuity, to license your contributions however I see fit**.    
Right now, that means a mix of GPL-3.0-only and WTFPL, but I reserve the right to relicense it or re-use code in proprietary projects in the future.     
This is a personal project where I don't expect external contributions to be any more complex than small-scale bugfixes and feature additions, so I think that's reasonable. If you think that's unreasonable, don't contribute. You will be asked to acknowledge this policy the first time you send me a pull request. See [`CONTRIBUTING.md`](CONTRIBUTING.md) for more information.


Contact
-------

You can send me comments on the [VCV Rack community forums](https://community.vcvrack.com/).    
You can send me bug reports and feature requests on [my GitHub project page](https://github.com/AriaSalvatrice/AriaVCVModules/issues).    
You can send me tips by [purchasing my albums](https://ariasalvatrice.bandcamp.com/).    
You can send me dog gifs to <woof@aria.dog>.

ttyl,

![Aria Salvatrice](/doc/signature.png)
