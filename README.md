Aria Salvatrice Signature Series Synthesizer Modules
====================================================

![](https://img.shields.io/badge/Works%20on%20my%20computer-Yes-success?style=plastic)

Hiya hello hey hi what’s hip love, I’m Aria Salvatrice. I’m a French expat living on the Worldwide Web with my dog.

I make [Gay Baroque Technopop](https://soundcloud.com/ariasalvatrice) and [Pastoral Industrial](https://ariasalvatrice.bandcamp.com/) music happen, and made a buncha synth modules compatible with VCV Rack for my own use.    
They are the Aria Salvatrice Signature Series. They’re growing into an integrated system for performing aleatoric techno. Lotsa artists found them inspiring for many genres of music. You can grab them for free!

You should soon be able to use them in [Cardinal](https://github.com/DISTRHO/Cardinal). They are not supported in VCV Rack 2, due to [their behavior towards third-party contributors, and inclusion of women](https://aria.dog/barks/why-i-will-never-create-modules-for-vcv-rack-anymore/). Bug reports you cannot reproduce in a supported fork, such as Cardinal, will not be accepted. 

![Modules](/doc/collection.jpg)




Documentation
-------------

The documentation of the modules is only available [from my website](https://aria.dog/modules/). It's beginner-friendly and detailed - please check it out.



Compatibility, Download & Installation
--------------------------------------

Those modules currently work with any 1.x version of VCV Rack, on Linux, Windows, and OS X. Please contact VCV to obtain them

You can [build this plugin yourself](https://vcvrack.com/manual/Building#building-rack-plugins). If you build my plugin locally, you have to `make dep` before you `make dist`. 

The [Sonaremin](https://github.com/hexdump0815/sonaremin) project for Raspberry Pi bundles my modules in its distribution.    






<!-- Donations: currently not requested as the collection isn't actively developed
---------

If you wish to send me a tip for my modules, at this time, donations can only be accepted in the form of [pay-what-you-want album purchases](https://ariasalvatrice.bandcamp.com/releases). Feel free to send me a message along with the donation: <woof@aria.dog>!

I am looking into providing more convenient donation options in the future, since bandcamp takes their cut and requires you to make a purchase you might not be interested in. It is unfortunately difficult to find a payment processor that operates legally in France but respects the choice of individuals to go by a chosen name. Thank you for your understanding. -->




Other thingies
--------------

- [Changelog](CHANGELOG.md)
- [Design language of the modules](doc/design.md)




Acknowledgements & Namedrops
----------------------------

Thanks to everyone who created open-source modules I could learn from and make songs with.

Thanks to [Andrew Belt](https://vcvrack.com/) for creating VCV Rack and providing code suggestions.

Thanks to [Jerry Sievert](https://legitimatesounding.com/) and [cschol](https://github.com/cschol) for their help with [QuickJS](https://bellard.org/quickjs/) integration.

Thanks to [Silvio Kunaschk](https://github.com/qno/) for their help with continuous integration.

Thanks to [Squinky Labs](https://github.com/squinkylabs/SquinkyVCV), [Stoermelder](https://github.com/stoermelder/vcvrack-packone), [David O'Rourke](https://github.com/david-c14/SubmarineFree), and [Anthony Lexander Matos](https://github.com/anlexmatos) for technical advice.

Thanks to [Omri Cohen](https://www.youtube.com/channel/UCuWKHSHTHMV_nVSeNH4gYAg) for featuring my modules in livestreams.

Thanks to [Ken McAloon](https://whatsinaname.xyz/) for Latin language translations.

Thanks to [Sophie](https://twitter.com/DreamyDoric) for music theory advice.

Thanks to [Heavy Viper](https://rekkanogotoku.com/) for years of inspiring conversation about synths and music.

Thanks to [Mog](https://github.com/JustMog/Mog-VCV) for Mog.

Thanks to [my dog Ornstein](https://ornstein.neocities.org/) for being a good dog.

Thanks to my music-making LGBBQT internet shitposting gang for introducing me to VCV and modular synthesis as something that's not just for old guys with more disposable income than impetus to write fresh songs.

<big>**But most of all, thanks to YOU for using my art.**</big>




Lawyer's corner
---------------

Yeah, the licensing rules are a bit complicated. But the big idea is pretty simple: my modules are GPL3-or-later, my logo is copyrighted, the rest is less restrictive. If there's licensing incompatibilities (besides the intended virality of the GPL) lemme know and we'll figure out something.

Here goes the detailed breakdown:

### The output of my modules belongs to you

It should go without saying that no sane courtroom would ever humor the idea the output of my modules is original enough to be my copyright, even that of Arcane, which directly outputs random data obtained from a server under my control.

Since courtooms are rarely sane, **I explicitly relinquish any claim of intellectual property over the output you obtain by operating my modules**, not that I believe I ever had any. Any song you make with them is yours alone.


### Source code of individual modules

The code of the Aria Salvatrice Signature Series Synthesizer Modules is distributed under the [GNU General Public License v3.0 or later](https://spdx.org/licenses/GPL-3.0-or-later.html). The modules come without any warranty and might recklessly endanger life and limb, the usual.


### Re-usable libraries

Not that the code is very good, but some libraries I created for my own use are available under the less restrictive terms of the [Do What The Fuck You Want To Public License](https://spdx.org/licenses/WTFPL.html), to allow every creator to do what the fuck they want. The files in question are [`src/javascript.hpp`](src/javascript.hpp), [`src/lcd.hpp`](src/lcd.hpp), [`src/polyexternalscale.hpp`](src/polyexternalscale.hpp), [`src/portablesequence.hpp`](src/portablesequence.hpp), [`src/prng.hpp`](src/prng.hpp), [`src/quantizer.hpp`](src/quantizer.hpp), and [`src/widgets.hpp`](src/widgets.hpp).


### Faceplates 

 [CC-BY-SA-4.0](https://creativecommons.org/licenses/by-sa/4.0/), with the exception of my signature logo, which is copyrighted, and generally not directly baked into my faceplate SVG files. You may freely distribute your faceplate edits. If you enjoy making custom skins, bear in mind that some of the colors used in my modules are defined in the code, not the SVG files.


### Components (knobs, jacks, etc)

WTFPL graphics and code (in [`src/widgets.hpp`](src/widgets.hpp)). But if you re-use them, I request you do not entirely re-use my signature color scheme in your own modules. This request is not legally binding, as it'd make licensing complicated. Because I use a limited palette, it's easy to replace most colors in my SVG files using search and replace in a text editor.

My collection of modules and widgets does not use VCV's component library at all, and is thus unencumbered by its licensing restrictions.


### Signature / Logo

Copyrighted. It's mine.

**If you edit my code to use my modules as a base for your own altered modules, remove my signature from your faceplates**, even if you think your changes are trivial: I don't want to endorse and take credit for something I didn't vet or personally participate in.

If you are faithfully porting my code to a fork of VCV Rack, are compiling binaries for another platform, or are otherwise distributing my modules as I designed them, keep the signature. If unsure, just ask.

The easiest way to remove my signature from every module is by blanking or replacing the graphic in the [`res/components/signature.svg`](res/components/signature.svg) file, and removing it from the blank plate: [`res/faceplates/Blank.svg`](res/faceplates/Blank.svg).


### Project name

**Aria Salvatrice** is the name I go by as a person. Distributed forks of my code should not make my name part of their title. The VCV project, and software libraries that distribute my code, should not distribute a fork maintained by a different person under my name without my explicit permission. To do so would impersonate me.    
Distributed forks of my code should mention I'm the original author, but shouldn't use my name in a way that can be construed as implying my authorship of their fork, or my endorsement of their fork. 

**While the VCV project's policy [allows taking over inactive plugins in its library, including those that are named after their author](https://community.vcvrack.com/t/open-source-modules-not-in-the-library/11357), I am requesting for its [ethics guidelines](https://vcvrack.com/manual/PluginLicensing#vcv-plugin-ethics-guidelines) protecting the brand names of companies to be extended to my own name as a human.** I am also requesting the same out of any other project distributing my software: it is a long-standing tradition of free open-source software that forks should go by a different name, if only to avoid user confusion.

If you are faithfully porting my code to a fork of VCV Rack, are compiling binaries for another platform, or are otherwise distributing my modules as I designed them, but need to apply trivial compatibility patches to make my software work on your platform, you should keep the name, and the signature. The deciding factor is whether you are distributing my software _as I designed it_. If you alter it, you should rebrand it.

The name of the individual modules in my collection does not have to be changed. Whether to keep the name of the modules the same, change them, or name them a variation of the original name, is left to the forker's discrection.

Git forks of my code on sites such as github, created for example to experiment with my code, forks that are not directly distributed to end users but only seen by an audience of developers, are obviously not considered distributed forks, and are thus exempt from this request, as it is obvious to the intended audience that this is a fork, and no impersonation is intended. 


### Graphics for the **Arcane** module 

The **Arcane** module uses Tarot cards altered from [Yoav Ben-Dov's CBD Tarot](https://www.cbdtarot.com/). Its graphics are distributed under the [CC-BY-NC-SA](https://creativecommons.org/licenses/by-nc-sa/3.0/deed.en_US), to comply with the license of the graphics used. That includes the faceplate, as it uses a pattern taken from that tarot deck.

### Fonts used

- The LCD Font is an edited SVG export of the [Fixed_v01 font by Orgdot](http://www.orgdot.com/aliasfonts/index.htm), released under [a custom MIT-style license](res/components/lcd/Fixed_v01/LICENSE.txt).
- The segment display font is [DSEG](https://www.keshikan.net/fonts-e.html) by [Keshikan](https://www.keshikan.net/), released under the [OFL 1.1](res/dseg/LICENSE.txt). A TTF file from that family is distributed with the modules.
- Titles are typeset in [Francois One](https://fonts.google.com/specimen/Francois+One) by [Vernon Adams](http://sansoxygen.com/). This font is not redistributed with the modules. 
- Labels are typeset in [Nova](https://fontlibrary.org/en/font/nova) by [Wojciech Kalinowski](https://fontlibrary.org/en/member/wmk69). This font family is not redistributed with the modules. 


### Libraries used

- [QuickJS](https://bellard.org/quickjs/) by [Fabrice Bellard](https://bellard.org/) is used in some modules under the terms of the [MIT license](https://github.com/AriaSalvatrice/AriaVCVModules/tree/master/doc/LICENSE_QuickJS.txt)
- [TonalJS](https://github.com/tonaljs/tonal) by [danigb](https://github.com/danigb) is used in some modules under the terms of the [MIT license](https://github.com/AriaSalvatrice/AriaVCVModules/tree/master/doc/LICENSE_TonalJs.txt)


### Copyright assignment

**By sending me pull requests, you assign their copyright to me, allowing me, in perpetuity, to license your contributions however I see fit**.    
Right now, that means a mix of GPL-3.0-or-later and WTFPL, but I reserve the right to relicense it or re-use code in proprietary projects in the future.     
This is a personal project where I don't expect external contributions to be any more complex than small-scale bugfixes and feature additions, so I think that's reasonable. If you think that's unreasonable, don't contribute. You will be asked to acknowledge this policy the first time you send me a non-trivial pull request. See [`CONTRIBUTING.md`](CONTRIBUTING.md) for more information.




Contact
-------

You can send me comments on the [VCV Rack community forums](https://community.vcvrack.com/).    
You can send me bug reports and feature requests on [my GitHub project page](https://github.com/AriaSalvatrice/AriaVCVModules/issues).    
You can send me tips by [purchasing my albums](https://ariasalvatrice.bandcamp.com/).    
You can send me dog gifs to <woof@aria.dog>.

ttyl,

![Aria Salvatrice](/doc/signature.png)
