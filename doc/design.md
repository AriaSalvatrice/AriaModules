Design language of the modules
==============================

This documents what I've done so far, mostly for my own reference. It's not a set of strict rules I plan to follow closely, it's just meant to improve consistency across the collection.

## Tools

- Inkscape
- Krita

## Fonts

- Francois One 13pt for titles
- Nova Flat 9pt with -0.50px letterspacing for labels

Font sizes expressed in however the hell Inkscape chooses to interpret pt and px by default. 

Align the baselines first, then use Path > Object to Path to convert them ASAP - no need to keep a master version with editable labels, just remake those when necessary.

## Colors

- Yellow: 					    #ffcc03ff
- Pink: 					    #fcaebbff
- Blues, from light to dark:
  - Lightest (not used yet):    #c1f0f2ff
  - Light (backgrounds):        #76bfbeff
  - Neutral (panel decoration): #61aeb0ff
  - Shade (bg gradient):        #469ca9ff
  - Dark (text and arrows):     #0e6977ff
  - Darkest (jacks):            #083d45ff

## Alignment

Things are aligned to a 0.5mm grid when possible.

## Jacks

Jacks are spaced by at least 8mm. 10mm for a small logical separation. They are preferably aligned to integer values. Inputs have a yellow rim, outputs a pink one, and are framed in a dark roundrect (3mm rounding radius).

## Hand painting

Labels are never hand painted. Arrows are always hand painted. Illustrations should be simple line art. 

## Process

1. Rough design and component placement in Inkscape. 
2. Only edit placements in code from now on - the SVG placement is just for drafting. Because frankly, having to write in values by hand and recompile things is much less of a pain than having to deal with Inkscape more than necessary.
3. Inkscape will crash a few times because haha stop complaining it's open-source.
4. Hand paint the background and arrows in Krita. Place notches in the corners of the frame for easy alignment. Trace it in Inkscape on default settings and resize it.