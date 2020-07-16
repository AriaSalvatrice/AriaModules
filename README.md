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

It should go without saying that no sane courtroom would ever humor the idea the output of my modules is original enough to be my copyright, even that of Arcane, which directly outputs random data obtained from a server under my control.

Since courtooms are rarely sane, **I explicitly relinquish any claim of intellectual propery over the output you obtain by operating my modules**, not that I believe I ever had any. Any song you make with them is yours alone.


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
