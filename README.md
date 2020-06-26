Aria Salvatrice Signature Series Synthesizer Modules
====================================================

Hiya hello hey hi. These here are a few cool and nice modules what for [VCV Rack](https://vcvrack.com/). You can download them from [its built-in library](library.vcvrack.com/).

I am some sort of multimedia artist interested in generative processes, creating [music](https://ariasalvatrice.bandcamp.com/releases) in genres I've come to call "Gay Baroque Technopop" and "Pastoral Industrial". All these synthesizer modules were designed for my own use first. I will not create yet another ADSR or VCO unless they do something unique. With this collection, I aim to create an integrated, coherent system of opinionated modules focused on the live performance of aleatoric techno. Nonetheless, they are versatile, and do their best to play nice with the wider module ecosystem. Many people enjoyed using my modules in all sorts of music, and I hope you will like them too.

FIXME: **If you wish to show your support for my synthesizer modules, tips are accepted:**


----

Modules documentation
---------------------

- [Split and Merge: Splort, Smerge, Spleet, Swerge, and Splirge](#split-and-merge-splort-smerge-spleet-swerge-and-splirge)
- [Quatherina the Quantum Duck presents Quatherina's Quality Quad Quantizer](#quatherina-the-quantum-duck-presents-quatherinas-quality-quad-quantizer)
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

There are three different form factors available of the same underlying module:

- **Quatherina's Quality Quad Quantizer**  (or **QQQQ**): The full 20hp version, featuring piano buttons, 4 polyphonic quantizer columns, and 16 memory slots.
- TODO: **Quack**: This smaller 7hp version only include piano buttons, and a single polyphonic quantizer column from QQQQ.
- TODO: **Q<**: 3hp. 1 column. External input. Tiny.

Those modules are all TODO: expanders of each other: the left one sets the scale to the one on the right, which forwards it. For example, if you only require two quantizer lines and no sequencer, you can use **Quack** on the left and **Q<** on the right, to use only half the size of **QQQQ**. If expander behavior is not desired, you can leave a gap between the instances, or TODO: disable expander features from the right-click menu.

The scale of the device can be set from multiple sources, and **the last source to make a change always has the last word what's the scale**. All the quantizer lines on a module follow the same scale. And while in this documentation, I call them scales for simplicity, **QQQQ** also works with chords (albeit, without knowledge of their voicing), and can be a fantasatic arpeggiator. 

Let's look at the controls of **QQQQ** in detail. Once you understand them, you'll also understand what the smaller form factors do.

- **Piano keys**: They always show you the current scale. Keys lit in yellow are parts of the scale, while unlit keys are disallowed. You can also click on the keys to change which notes are allowed. TODO: When you click the **visualize** button of a quantizer line, the notes currently playing on that line are lit pink.

In the TODO:**LCD Area** at the top of the device:

- **Scale** and **Key**: Selects a preset scale, and its key. The available scales were curated for ease of use and instant satisfaction rather than for comprehensiveness, avoiding duplication in the form of scales that can be expressed as modes of another (for your convenience, an exception is made for Natural Minor and Pentatonic Minor). The following scales, along with my qualified opinion about them, are available:
    - **Chromatic** - All the notes, all the time! It's a great deal if you like every note.
    - **Major** - The white keys of the piano! I'm not really sure why we need anything else, but hey, your call if you want something exotic.
    - **Natural Minor** - It is the exact same thing as the major scale, but it has a different "tonic" making it more suitable for being sad.
    - **Melodic Minor** - You can use it to write melodies! Before this scale was invented, music had no melodies.
    - **Harmonic Minor** - This is the fancy, high-culture, bourgeois minor scale, used by refined individuals. That shit slaps.
    - **Pentatonic Major** - It's the guitar scale! It has only 5 notes. Yet guitars have 6 strings. How does it even work? Theory makes my head hurt.
    - **Pentatonic Minor** - Same scam as with natural minor: it's the exact same notes as major, just with their order shuffled around a bit.
    - **Whole Tone** - This one has structure! It is for fans of symmetry and kaleidoscopes and fractals and putting a fibonacci spiral overlay on a renaissance painting. 
    - **Blues Major** - It's for when you got the blues, but aren't that sad about it.
    - **Blues Minor** - That is for special occasions.
    - **Dominant Diminished** - That bad boy makes Honked up Hot Jazz .
    - **Bebop Major** - This one has a "blue note", like those albums with the cool typography on the cover.
    - **Bebop Minor** - This one is for fans of animé. You're welcome.
    - **Double Harmonic** - It's so harmonic, cultures from all over the world claim this one as theirs!
    - **Eight Tone Spanish** - When I tried to find more useful info about it, I found a forum post from 2003, where a posted said indignantly that scales "[are] not a democracy, but a hierarchy".
    - **Hirajōshi** - It's all you need to rip a sick shamisen solo. Anything more is for poseur kids.
    - **In Sen** - That one was originally used for Japanese wind chimes. Instantly become the most original dark ambient modular artist around by feeding it to [Rings and Clouds](https://vcvrack.com/AudibleInstruments) for 3 hours.
- **External Scale**: This input and output express the scale current as a 12-channel polyphonic cable, where enabled notes have a continuous 10V signal, and disabled notes, 0V. This way of expressing a scale is supported by my other modules, such as **Arcane** and **Darius**. Try it out with my splitters and mergers! 

To the top right of the module:

- TODO:**Scene slot**: Those 16 buttons allow you to save 16 different scales. Navigating to a slot loads it, and changing the scale while a slot is active saves the scale to that slot. You can right-click on those buttons to copy and paste scales. You can map those buttons via MIDI!
- TODO:**Scene input**: Navigates the scenes via CV. Useful with a step sequencer! Accepts 0V~10V. When plugged in, buttons can't be operated manually anymore.

To the right, below the **Scene slots**:

- TODO:**Keyboard** and **Paste** input. They allow you to write down or paste input from the clipboard, and have it transformed to chords. The three following notations are supported (which you're using is automatically detected):
    - **Lead sheet notation**: Chord symbols, separated with spaces, commas, or hyphens. It's pretty liberal in what it accepts! For example, this is a valid input: `G am C B, E/G# Bb A,Fsus2  Csus2`
    - **Roman numeral notation**: FIXME: 
    - **Portable Sequence**: Simultaneous notes from a [portable sequence](https://github.com/squinkylabs/SquinkyVCV/blob/master/docs/clipboard-format.md) are interpreted as a new scale.

If your input isn't recognized, nothing happens, that simple. And remember that **QQQQ** only thinks in scales, not in chords. After you import a chord progression, **QQQQ** doesn't care about the voicing of the chord. It's all folded back to a single octave. 

Next, to the bottom-right of the module:

- **Likeness of Quatherina**: Increases the appeal of the module through co-branding.

To the center of the module are the four quantizer columns, through which signal flows from top to bottom.

- TODO:**CV Input**: Each input jack is forwarded to the columns to its right, if you want to process the same signal in different ways. Inputs are polyphonic.
- TODO:**% Scale**: Attenuates, amplifies, or inverts the input.
- TODO:**Offset**: Adds or remove a fixed voltage to the input.
- TODO:**Transpose**: Transposes the signal according to one of the three rules, set by the transpose mode button under the knob.
- TODO:**Transpose mode**: Selects one of the three transposition rules for the knob directly above it. Each transposition rules results in output that remains in harmony. The rules are:
    - **Unlit** - _Octaves Mode_: Quantizes the signal first, then transposes it up or down by full octaves.
    - **Yellow** - _Semitones Mode_: Transposes the signal up or down by a few semitones first, then quantizes it. 
    - TODO:**Pink** - _Scale degrees Mode_: Quantizes the signal first, then transposes the signal by a specific amount of scale degrees.
- TODO:**Sample & Hold / Track & Hold toggle**: Swaps between the two modes for the input directly below it. Unlit is Sample & Hold, lit is Track & Hold.
- TODO:**Sample & Hold / Track & Hold input**: When plugged in, instead of operating continuously, the quantizer will Sample & Hold or Track & Hold the signal. When you're quantizing a noisy source of modulation, it's useful to send it the same gate you'd send to the envelope that will play the note in question. If the **CV input** is polyphonic, send it the same amount of channels.
- TODO:**Output** - The quantized signal! Polyphony is defined by the **CV Input**. You only pay the CPU cost of the features you use: if left unplugged, unused columns avoid doing unecessary math.


**Protip for cool kids only:** Did you know that there are a few people out there who do not use Quantizers, and instead choose to spend years to learn something called "Music Theory"? Those people do all their music by thinking really hard about maths! It's true - check out your library for more information.




Darius Branching Step Sequencer
-------------------------------

![Darius](/doc/darius.gif)

Are you still looking for the perfect sequencer, the one that allows you to effortlessly express any musical idea? 

Sucks. 

Here comes yet another gimmick module challenging you to integrate its way of thinking to your song instead.

**Darius** is a 8-step sequencer where each node branches into two possible paths, creating repeating patterns that start similarly and resolve differently. Takes a whole 32hp of space to fit all its knobs. 

To get started immediately: patch in a clock to the **Forward⯈** input, randomize the CV, enable the **quantizer**, patch the **CV output** to an oscillator, and listen what happens. The module's **LCD** will help you figure out what's going on when you grope at the auspicuous-looking array of knobs.

On each node:

- **CV Knob** (left): sets the CV for that step.
- **Random route knob** (right): alters the probability to pick the top or the bottom node on the next step when stepping **Forward⯈**. If the knob's arrow points to the right, that means 50/50. (Because of floating point math imprecision, sometimes the probabilities displayed on the LCD are off by 0.1%)
- **Gate Output**: when the node is active, passes through the gate or step inputs received on any of the directional inputs, or sends 10V continuously if no step input is plugged in.

On the top-left of the module:

- **⯇Back**, **Up⯅**, **Down⯆**, **Forward⯈**: Takes a step in the corresponding direction when it receives a gate or a trigger. Stepping **Up⯅** or **Down⯆** advances a step in the corresponding direction, it's used to force the module to take a certain route, but can't be used to move within the same step. Going **⯇Back** remembers the path you've taken, and does nothing if you're already on the first step. You can also step **Forward⯈** manually, by using the button, which works even if the module isn't running. 
- **Step**: This manual input works even if the module isn't running. 
- **Run**: Starts and stops accepting directional inputs.
- **Reset**: Go back to the first node. 

Those inputs are generally connected to the corresponding output of a clock, but anything that sends gates or triggers will work. Triggers are accepted on any polyphonic channel, which is useful for creative self-patching via a poly merge module.

On the top of the module:

- **Step range**: Choose on which step to start and end the pattern, from 1 to 8 steps in total.
- **Randomize**: Those two buttons operate on the CV and the routes separately. If you don't like the results, you can use `Edit > Undo`. 

On the bottom-left:

- **CV/Quantize**: Whether to output precise CV (best for modulation), or quantized V/OCT CV (Twelve-tone Equal Temperament only - the usual Western tuning system).
- **-5V\~5V/0V\~10V**: In CV mode, selects if the knobs output voltage from 0V to 10V, or -5V to 5V. In quantized mode, removes 1 octave from the output. 
- **Min** and **Max**: Limit the CV output range. The words are only suggestions, if the **Min** is larger than the **Max**, it just flips in which direction the **CV** knobs operate.
- **Scale** and **Key**: Select which notes to quantize to when in Quantize mode. The available scales are the same as in the **QQQQ** modules. 
- **External scale**: Accepts the scale in the format **QQQQ** sends. You can use Darius as an arpeggiator by sending it a chord rather than a full scale!
- **Slide**: The fun knob.
- **Global Gate**: Passes through the gate or step inputs received on any of the directional inputs, or sends a short trig if operated via the manual Step button. It's useful if are controlling Darius using more than one directional input.
- **CV**: The main output. 

On the bottom-right, next to the signature, are two inputs used to fix the random seed, if you want Darius to be a bit more deterministic.

- **Random**: Use this to fix the random seed! When the input is not patched, or when it's receiving 0V, **Darius** flips the coin randomly. But when it's receiving a seed, the coin flips become deterministic - it will take the same route every time. Try out alternating, every bar, sending it an arbitrary fixed voltage such as 4.58V then 0V, to create call-and-response phrases where the first part is always the same.
- **1st/All**: Decides whether to store the random seed and plan out the route when on the first node, or whether to use a fresh random seed and flip the coin at moment to decide the node forward. In **1st** mode, going back and forth repeatedly results in the same path (unless you alter the routes), until the first node is reached from step 8 (it won't refresh it if you reach the first node from stepping back). In effect, it acts as a sample and hold for the **Random** input at the exact moment the first node is _left_.

Via the **right-click menu**, you can load various presets for the CV and routes, and copy/paste [Portable Sequences](https://github.com/squinkylabs/SquinkyVCV/blob/master/docs/clipboard-format.md). Portable Sequences support is experimental, and its behavior might change in the near future. When you **Copy** a sequence, you copy one random possible path Darius could take (or the current path it will take if there is an external **Random** seed). When you **Paste** a sequence, you paste its first 8 notes to each step, rather than each node: each node of a same step will receive the same note, for you to use as a new point of departure. After pasting, be sure to set the **Min** and **Max** knobs to the maximum range to obtain accurate data.

If you send triggers to the directional inputs at the exact same time, only one will be accepted. The default priority is: 
**Forward⯈**, **Up⯅**, **Down⯆**, **⯇Back**. If you want a different priority, you can patch logic gates with modules such as [Count Modula's](https://github.com/countmodula/VCVRackPlugins) to do that. If the priorities aren't working as expected, do not forget that every single cable a signal travels through adds at least one sample of delay, so your triggers might not be actually simultaneous.

Darius has a simple panel, easily understood features, and is impossible to truly tame - by design. It is meant to be easy to learn, surprising to use, and fun to master. It can be used as a melody sequencer, an arpeggiator, a source of modulation, a drum sequencer, even as a worthless waveshaper. Many of its apparent limitations can be overcome with a bit of creative patching. It works wonderfully with [stoermelder PackOne](https://github.com/stoermelder/vcvrack-packone)'s [CV-MAP](https://github.com/stoermelder/vcvrack-packone/blob/v1/docs/CVMap.md) and [8FACE](https://github.com/stoermelder/vcvrack-packone/blob/v1/docs/EightFace.md) to give you CV control over the knobs and add multiple preset banks.

Darius is named after the eponymous arcade shoot-em-up game series, known for its surreal visuals, its fish-themed enemies, its [unique soundtracks](https://www.youtube.com/watch?v=6FEdlAL3bX0), its multi-display arcade cabinets, and for allowing the player to select their route through the game via a branching map.

![Darius Gaiden](/doc/darius-gaiden-map.png)

I guess the module is also technically named after some dead Persian guy who did some King stuff, I heard on Wikipedia, the free encyclopedia.

**Protip for cool kids only:** The plural of Darius is Darii.




Arcane, Atout & Aleister - Today's Fortune ★
--------------------------------------------

![Arcane](/doc/arcane.png)

I hope luck is on your side. Every day, you will share together the fortune I shall grant you. It is your task to interpret what my augury means to you.

Modules in the Arcane series are comprised almost only of output jacks, sending today's fortune as CV. **Those values are the same for every Arcane user**, and nobody can predict or influence them. I pronounce a new oracle every day at 12:00 AM UTC without fail.

To get started immediately: use the topmost **1/16** output to drive the clock input of a step sequencer, send that sequencer's output to Arcane's quantizer input, send the quantizer output to an oscillator, and listen what happens. Next, try other jacks as a clock. 

Optional video introduction, showing a song made with Arcane:

[![Arcane video](/doc/arcane-video.png)](https://youtu.be/g66gyHkzz0E)

You can [download the demo patch from Patchstorage](https://patchstorage.com/arcane-demo-patch/).


Every day, I will draw the following cards, and impart the following knowledge:

- **_Arcane Majeur_**: a major arcana of the Tarot of Marseilles.
- **_Bâtons_** (Wands), **_Coupes_** (Cups), **_Deniers_** (Coins), **_Épées_** (Swords): a different 16 step binary pattern for each suit of the Tarot.
- **Scale pattern**: The notes of a music scale comprised of 5, 6, 7, or 8 notes, often scales common in popular Western music, and sometimes less common ones. Which note is the root of the scale isn't specified, only which notes are part of the scale.
- **8 note pattern**: Every note from the selected scale, sorted in a random order. If the scale has fewer than 8 notes, up to 3 different notes will be repeated at the end, an octave higher.
- **BPM**: Integer ranging from 60 to 180 BPM, with values between 90 and 150 twice as likely.
- My **sincerest wish** of luck, love, health, and prosperity.

How to interpret my fortune is up to you and your friends. You can make use of a single output jack of the module, or all of them. You can craft an ephemeral song that will no longer exist tomorrow. You can create an unchanging patch that will grace you with a new song every day. There are no rules but the ones you choose to follow. 

All I can do is reveal some mundane obscura to get you started scrying the will of the jacks:

- **Quantizer**: A pair of polyphonic ports quantizing input to the nearest scale note.    
Try to combine it with the [Split and Merge series](#split-and-merge-splort-smerge-spleet-swerge-and-splirge), the ability to sort by voltage might prove useful. 
- **Melody**: Polyphonic ports sending every note of the scale on the fourth octave (0V~0.9166667V), in the order of the 8 note pattern. It sends as many notes as there are in the scale (between 5 and 8).
- **Padded**: Like **Melody**, sends the entire 8 note pattern, with any repeated note being one octave higher than normal.
If you combine them with [ML_modules sequential switch 8->1](https://github.com/martin-lueders/ML_modules/wiki/Sequential-Switches), they can become a melody.
- **External Scale**: Sends the scale in a format my other modules use: as a 12-channel polyphonic cable, where enabled notes have a continuous 10V signal, and disabled notes, 0V. Try it with **Darius**!
- **_Arcane Majeur_**: The number of the arcana, divided by 10.    
Thus, _The Fool_ is 0V, _The Magician_ is 0.1V, and so on up to 2.1V for _The World_. Following the conventions of the Tarot of Marseilles, the 8th arcana is Justice, and the 11th is Strength, unlike the ordering popularized by the Rider-Waite-Smith deck, which swaps those two arcana around.    
You can multiply this signal by 4.76 with an offsetter such as [Bogaudio's](https://github.com/bogaudio/BogaudioModules#offset) to obtain a 0V~9.99V signal, if you wish.
- **BPM**: Follows the [VCV Standard](https://vcvrack.com/manual/VoltageStandards#pitch-and-frequencies) for clocks and LFOs where 0V stands for 120BPM.    
You can send that signal to a compatible clock module such as [Impromptu Modular's Clocked](https://marcboule.github.io/ImpromptuModular/#clocked) if you require a clock (for example, for swing, or other time signatures than 4/4).
- **Reset** and **Run**: They control all the outputs below them. Operate them manually using the button, or synchronize multiple devices by sending them a trigger from a single source, for example the [Little Utils Button](https://github.com/mgunyho/Little-Utils#button).
- **Gate/Ramp** on the **32nd note**, **16th note**, **8th note**, **4th note**, **Bar**: Sends, depending on the position of the **Gate/Ramp** switch, either a gate synchronized to the BPM at the corresponding interval in 4/4 time, or a 0V-10V ramp corresponding to that phase.    
While **Arcane** can serve as a master clock, the ramp can be used to drive the [ZZC Clock](https://zzc.github.io/en/clock-manipulation/clock/), if you'd prefer to use it.
- **_Bâtons_**, **_Coupes_**, **_Deniers_**, **_Épées_**: The four patterns, sent as gates on each 32nd note, 16th note, 8th note, 4th note, or bar.    
Don't limit yourself to drums, you can use them in many different ways! Want to visualize the patterns, or to use them as something different than a rhythm? Try out the **Aleister** expander.
- **Pulse Width**: Selects how long the gates are, proportionally to their note length. It affects every gate output. Need more granular control over pulse widths? An external clock or a pulse generator such as [Submarine's](https://github.com/david-c14/SubmarineFree/blob/master/manual/PG.md) will help.

The module's **LCD** will show you the date of the fortune, the BPM, the name of the arcana, and the notes of the scale. However, it cannot unambiguously name the scale, since your fortune doesn't stipulate on which note the scale starts.

The **LCD** will also tell you whether today I am wishing you luck, love, health, or prosperity (displayed as MONEY due to the size of the LCD). You cannot access that information via CV, as it is only a personal wish that might not manifest, unlike my oracles, which are always accurate.

There are two available form factors of the same module:

- **Arcane**: the full 24hp version displays today's arcana, from Nicolas Conver's 1760 Tarot of Marseilles.
- **Atout**: the smaller 9hp version doesn't display the arcana, but includes all the functionality and every jack from **Arcane**. To conserve space, the **Gate/Ramp** switch is at the bottom.

The third module, **Aleister**, gives you access to the **_Bâtons_**, **_Coupes_**, **_Deniers_**, and **_Épées_** binary patterns as series of 16 outputs sending continuously either 0V or 10V, rather than as a rhythmic pattern of gates. If you connect only the first output of a group, it will instead be a polyphonic cable outputting the entire group. **Aleister** takes 14hp of space. You can employ **Aleister**'s services as a standalone module, but when placed directely the right of either **Arcane** or **Atout**, the module will act as an expander, lighting up the jacks in sync with the rightmost connected output of the corresponding pattern on the parent module. Try it out, you'll understand what it does immediately.

If the module is active at the time a new fortune is drawn, the values will not change, but a notification a new fortune is available will appear on the **LCD**. Using the right-click menu, you can _Initialize_ the module to download the newest fortune. Remember to also initialize **Aleister** if in use (whether standalone or in expander mode).

Using this series of modules to their full extent requires a bit of creative patching, a bit of lateral thinking, but most importantly, friends to share your different interpretations of the same fortune with.

**The tarot deck used is the Conver – Ben-Dov (CBD) version**. The CBD deck was restored in 2008-2011 from the original 18th century deck by tarot expert [Dr. Yoav Ben-Dov's](https://www.cbdtarot.com/). It was chosen for its traditional significance, the quality of its imagery, and the simplicity of its broad lines being well suited to VCV's limited vector rendering engine.    
While there is great variety in decks across many different Tarot traditions, Tarot of Marseilles imagery does not vary significantly across decks. The exact same subjects are drawn from the same angle, leaving very little room for the artist to re-interpret the themes. A few figures are generally depicted nude, and while the style of the CBD deck is obviously neither graphic nor intended to titillate, I took the liberty to slightly alter a few cards to remove details, in particular the Devil's codpiece, as most users will download my plugin without being forewarned about contents a small minority would find upsetting. I hope you will understand my decision. 

**Upon activation, this module will connect to the internet**, and fetch today's numbers on a GitHub repository via HTTP. The fortunes are not generated locally to make it possible for all users to share the same numbers every day, while also making it impossible to cheat fate and predict the next oracle (which a local deterministic implementation would necessarily entail).

If the repository is unreachable, the module will will output 0V on all ports, except the quantizer, which will forward input as-is. [You can find more information about Arcane server and API, and their source code, on their GitHub repository](https://github.com/AriaSalvatrice/Arcane).

**There is no offline mode, and no built-in way to load older fortunes, by design.** These are multiplayer-only oracles. But I won't prevent you from editing the .json files in the `AriaSalvatrice/Arcane/` directory of your VCV Rack user directory, if you really must. Every downloaded fortune is archived locally, a full archive is available from GitHub, and if a local file exists for today, it will not be checked against the server.     
(Quick tech remark about JSON API cache spoofing: such self-deception will never alter your true destiny, and it is unwise to cling to the past instead of living for a future you can yet change.)

It should go without saying that no sane courtroom would ever humor the idea the output of Arcane is original enough to be my copyright. Since courtooms are rarely sane, **I explicitly relinquish any claim of intellectual propery over the output of Arcane**, not that I believe I ever had any. Any song you make with it is yours alone.

**Protip for cool kids only:** If you treat my oracles as a mere random number generator, you will never gain any wisdom from them.




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
- **Lock X/Lock Y**: By pressing those dangerous buttons, UnDuLaR fully takes over scrolling on that axis, preventing scrolling from the scrollbars, scrollwheels, arrow keys, middle click, or from dragging cables at the edges of the screen. It will be a little bit jittery, I can't help it. Those locks will only work if the three corresponding cable inputs for that axis are unplugged. If you find yourself unable to scroll back to UnDuLaR, use this menu option temporarily to re-enable scrolling: `Engine > Sample Rate > Pause`.

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




Acknowledgements & Credits & Other Assorted Namedrops
-----------------------------------------------------

Thanks to everyone who created open-source modules I could learn from and make songs with. Thanks to Andrew Belt for creating VCV Rack and providing code suggestions. Thanks to my music-making LGBT internet shitposting gang for introducing me to VCV and modular synthesis as something that's not just for old guys with more disposable income than impetus to write fresh songs. 
Thanks to [Sophie](https://twitter.com/DreamyDoric) for music theory advice. Thanks to [Heavy Viper](https://rekkanogotoku.com/) for years of inspiring conversation about synths and music. Thanks to [Mog](https://github.com/JustMog/Mog-VCV) for Mog. Thanks to all VCV creators for their assistance, in particular [Squinky Labs](https://github.com/squinkylabs/SquinkyVCV), [Stoermelder](https://github.com/stoermelder/vcvrack-packone), [David O'Rourke](https://github.com/david-c14/SubmarineFree), [Omri Cohen](https://www.youtube.com/channel/UCuWKHSHTHMV_nVSeNH4gYAg), and [Jerry Sievert](https://legitimatesounding.com/). Thanks to [my dog Ornstein](https://ornstein.neocities.org/) for being a good dog.

**But most of all, thanks to YOU for using my art.**

The modules use the following fonts:
- [Francois One](https://fonts.google.com/specimen/Francois+One) by [Vernon Adams](http://sansoxygen.com/) 
- [Nova](https://fontlibrary.org/en/font/nova) by [Wojciech Kalinowski](https://fontlibrary.org/en/member/wmk69) 
- [Fixed_v01](http://www.orgdot.com/aliasfonts/index.htm) by [Orgdot](http://www.orgdot.com/aliasfonts/index.htm)

The **Arcane** module uses Tarot cards altered from [Yoav Ben-Dov's CBD Tarot](https://www.cbdtarot.com/).

The **UnDuLaR** module background uses the traditional yagasuri kimono pattern as provided by [ULOCO, UOTOMIZU](https://forallcreators.com/yagasuri-background/).



Lawyer's corner
---------------

The code of the Aria Salvatrice VCV Rack modules is distributed under the Do What The Fuck You Want To Public License version 2. They come without any warranty and might recklessly endanger life and limb, the usual.

The graphics are also distributed under the WTFPL, save for the following exceptions:

- My signature/logo remains copyrighted.
  - If you edit my code to use my modules as a base for your own altered modules, remove my signature from your faceplates, even if you think your changes are trivial: I don't want to endorse and take credit for something I didn't vet or personally participate in.
  - If you are faithfully porting my code to a fork of VCV Rack, are compiling binaries for another platform, or are otherwise faithfully distributing my modules as I designed them, keep the signature. If unsure, just ask.
  - The easiest way to remove my signature from every module is by blanking or replacing the graphic in the [`res/components/signature.svg`](res/components/signature.svg) file, and removing it from the blank plate: [`res/Blank.svg`](res/Blank.svg).
- The files in `res/Arcane` are all distributed under the [CC-BY-NC-SA](https://creativecommons.org/licenses/by-nc-sa/3.0/deed.en_US) license, as they are a derivative work of the [CBD Tarot](https://www.cbdtarot.com/) released under that license.
- The files in `res/components/lcd/Fixed_v01` were exported from the [Fixed_v01 font by Orgdot](http://www.orgdot.com/aliasfonts/index.htm), which is released under [a custom MIT-style license](res/components/lcd/Fixed_v01/LICENSE.txt), included in the source and in binary distributions.

You may freely distribute alternate faceplates for my modules. You will find information about my design language and design process in [`doc/design.md`](doc/design.md).




Contact
-------

You can send me comments on the [VCV Rack community forums](https://community.vcvrack.com/).    
You can send me bug reports and feature requests on [my GitHub project page](https://github.com/AriaSalvatrice/AriaVCVModules/issues).    
You can send me dontations to FIXME: .    
You can send me dog gifs to <woof@aria.dog>. 

ttyl,

![Aria Salvatrice](/doc/signature.png)
