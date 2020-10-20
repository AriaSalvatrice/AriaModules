Design language of the modules
==============================

This documents what I've done for my own reference. It's not a set of strict rules I plan to follow closely, and it doesn't define anything formally. It's just meant to improve consistency across the collection.

## Tools

- Inkscape
- Krita


## Fonts

- [Francois One](https://fonts.google.com/specimen/Francois+One) 13pt for titles
- [Nova](https://fontlibrary.org/en/font/nova) Flat 9pt, -0.50px letterspacing, 0.85 line-height for labels

Font sizes expressed in however the hell Inkscape chooses to interpret pt and px by default. 

Align the baselines first, then use Path > Object to Path to convert them ASAP - no need to keep a master version with editable labels, just remake those when necessary. It's a good idea to keep a text template outside the drawable boundariees of the file.

When labels call for a stroke to make them easier to distinguish from the background, the stroke is 0.5mm, and converted to a path.


## Colors

- Yellow: 					    #ffcc03ff
- Pink: 					    #fcaebbff
- Blues, from light to dark:
  - Lightest (Highlight, LCD):  #c1f0f2ff
  - Light (backgrounds):        #76bfbeff
  - Neutral (panel decoration): #61aeb0ff
  - Shade (bg gradient):        #469ca9ff
  - Dark (text and arrows):     #0e6977ff
  - Darkest (jacks):            #083d45ff

Additional gradients, from light to dark (the second color being the main). Use sparingly.

- Orange (Psychopump +/- buttons):  #ffcc03ff ~ #f28a40ff
- Red (Mute button):                #ff1c6eff ~ #974151ff
- Green (Solo button):              #a0ff2dff ~ #269d2cff

These shades of yellow and pink are almost indistinguishable to people with tritanopia (about 1% of the population), and the Red gradient is hard to tell apart from the bues for people with protanopia (about 1% of males), so they should never be the only design cue provided to convey information. When a widget can be lit both pink and yellow, the pink version should have a Dark notch or circle prevalent in the design. 


## Alignment

Keep things aligned across collections, but don't sweat precision. Go for something that looks good rather than mathematical accuracy.


## Jacks

Jacks are spaced by at least 8mm. 10mm for a small logical separation. They are preferably aligned to integer values. Inputs have a yellow rim, outputs a pink one. Outputs are framed in a dark roundrect iff it improves readability (3mm rounding radius).


## Illustrations

Vector illustrations should be simple cartoonesque line art or silhouettes, otherwise they break. Bitmaps might be an option for more complex illustrations. The background patterns should be subtle and not look too busy when zoomed out.


## Strokes

0.3mm and 0.5mm are frequently used.


## Bitmap tracing

Auto-tracing paths in inkscape with default settings on "Trace Bitmap" never looks right in VCV - lots of broken shapes.

For the tarot cards, I set the settings as follows, and barely edited them at all: 

- Mode: Multiple scans: Brightness steps, 2 scans, smooth, Stack scans, Remove background
- Options: Suppress speckles 5, smooth corners 0.2, optimize paths 0.05

Optimize paths to a low settings is the important one here. It's what tends to break things once imported in VCV.

