Aria Salvatrice Signature Series Synthesizer Modules
====================================================

![](https://img.shields.io/badge/Works%20on%20my%20computer-Yes-success?style=plastic)

Hiya hello hey hi what’s hip love, I’m Aria Salvatrice. I’m a French expat living on the Worldwide Web with my dog.

I make [Gay Baroque Technopop](https://soundcloud.com/ariasalvatrice) and [Pastoral Industrial](https://ariasalvatrice.bandcamp.com/) music happen, and made a buncha synth modules compatible with VCV Rack for my own use.    
They are the Aria Salvatrice Signature Series. They’re growing into an integrated system for performing aleatoric techno. Lotsa artists found them inspiring for many genres of music. You can grab them for free!

![Modules](/doc/collection.jpg)



Documentation
-------------

The documentation of the modules is only available [from my website](https://aria.dog/modules/). It's beginner-friendly and detailed - please check it out.



Compatibility, Download & Installation
--------------------------------------

Those modules should work with any 1.x version of VCV Rack. They are part of VCV's onlinne library: by [subscribing to my plugin](https://library.vcvrack.com/AriaSalvatrice/), you will receive every stable update a few days after it's released. Installing my modules from the VCV library the recommended way.

Forks and ports of VCV Rack such as the [Sonaremin](https://github.com/hexdump0815/sonaremin) might bundle my modules in their preferred way. 

No efforts will be made to support forks based on the 0.x series such as [miRack](https://mirack.app/modules/) - they will probably never be supported.

The only downloads you will find here are [automatic development builds](https://github.com/AriaSalvatrice/AriaVCVModules/releases/tag/AzureCI), which are never guaranteed to work properly.

You can also do things the hard way and [build the plugin yourself](https://vcvrack.com/manual/Building#building-rack-plugins) if you have a good reason to do so, for example, if you hate yourself, or if you enjoy building random C++ projects off github as a hobby.

If you build my plugin locally, you have to `make dep` before you `make dist`. 



Other thingies
--------------

- [Changelog](CHANGELOG.md)
- [Design language of the modules](doc/design.md)
- [Future plans for the collection](https://aria.dog/modules/plans/)
- [How to contribute](CONTRIBUTING.md)
- [Beta / pre-release thread on VCV forums](https://community.vcvrack.com/t/arias-cool-and-nice-thread-of-barely-working-betas-and-bug-squashing/8208)



Acknowledgements & Credits & Other Assorted Namedrops
-----------------------------------------------------

Thanks to everyone who created open-source modules I could learn from and make songs with.

Thanks to [Andrew Belt](https://vcvrack.com/) for creating VCV Rack and providing code suggestions.

Thanks to [Jerry Sievert](https://legitimatesounding.com/) and [cschol](https://github.com/cschol) for their help with [QuickJS](https://github.com/tonaljs/tonal) integration.

Thanks to [Squinky Labs](https://github.com/squinkylabs/SquinkyVCV), [Stoermelder](https://github.com/stoermelder/vcvrack-packone), [David O'Rourke](https://github.com/david-c14/SubmarineFree) for technical advice.

Thanks to [Ken McAloon](https://whatsinaname.xyz/) for Latin language translations.

Thanks to [Sophie](https://twitter.com/DreamyDoric) for music theory advice.

Thanks to [Heavy Viper](https://rekkanogotoku.com/) for years of inspiring conversation about synths and music.

Thanks to [Mog](https://github.com/JustMog/Mog-VCV) for Mog.

Thanks to [my dog Ornstein](https://ornstein.neocities.org/) for being a good dog.

Thanks to my music-making LGBBQT internet shitposting gang for introducing me to VCV and modular synthesis as something that's not just for old guys with more disposable income than impetus to write fresh songs.


<big>But most of all, thanks to YOU for using my art.</big>

The modules use the following fonts:
- [Francois One](https://fonts.google.com/specimen/Francois+One) by [Vernon Adams](http://sansoxygen.com/) for titles.
- [Nova](https://fontlibrary.org/en/font/nova) by [Wojciech Kalinowski](https://fontlibrary.org/en/member/wmk69) for faceplages.
- [Fixed_v01](http://www.orgdot.com/aliasfonts/index.htm) by [Orgdot](http://www.orgdot.com/aliasfonts/index.htm) for LCDs.
- [DSEG](https://www.keshikan.net/fonts-e.html) by [Keshikan](https://www.keshikan.net/) for segment displays.


The **Arcane** module uses Tarot cards altered from [Yoav Ben-Dov's CBD Tarot](https://www.cbdtarot.com/).

The **UnDuLaR** module background uses the traditional yagasuri kimono pattern as provided by [ULOCO, UOTOMIZU](https://forallcreators.com/yagasuri-background/).

[QuickJS](https://github.com/tonaljs/tonal) is used in some modules under the terms of the [MIT license](https://github.com/AriaSalvatrice/AriaVCVModules/tree/master/doc/LICENSE_TonalJs.txt)

[TonalJS](https://bellard.org/quickjs/) is used in some modules under the terms of the [MIT license](https://github.com/AriaSalvatrice/AriaVCVModules/tree/master/doc/LICENSE_QuickJS.txt)



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

### Segment Display Font

[DSEG](https://www.keshikan.net/fonts-e.html) by [Keshikan](https://www.keshikan.net/), released under the [OFL 1.1](res/dseg/LICENSE.txt).


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
