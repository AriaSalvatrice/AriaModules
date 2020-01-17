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


## Design process in 12 easy steps

1. Rough design and component placement on paper. 
2. Just do the background and title in Inkscape.
3. Do all the widget placement in code. Frankly, having to write in values by hand and recompile things is much less of a pain than having to deal with Inkscape more than necessary.
4. Inkscape will crash because stop complaining and being entitled it's open-source.
5. Once the widget placement is finalized, take a screencap and overlay it in Inkscape as a guide.
6. Place the labels and roundrects in Inkscape.
7. Inkscape will crash (this is normal).
8. Hand paint the background and arrows in Krita, export layers separately as PNG.
9. Don't forget to remove the guide layer to avoid bloating the file size.
10. Import the layers in Inkscape and trace them at default settings.
11. Inkscape will crash (this is normal).
12. Try <https://jakearchibald.github.io/svgomg/> if things break for no apparent reason. Sometimes it fixes them. Sometimes it makes them worse. 
