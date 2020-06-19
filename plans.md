# Overview

In a move of radical open-sourceness, I will make public most of my future plans for this collection. I make no promise I will ever actually work on them. Ideas take time to mature and your input can help make them better. Feel free to use Github issues, the VCV forums, or e-mail to discuss these. 

There's no such thing as stealing ideas, but there is such a thing as duplication of effort. If you wish to adopt something you see here, a notification that you're working on something similar is expected from you. 

- I will not create modules that already exist in a form that satisfies me.
- I will not create modules I will not use to make and play music. I've come to call the genres I write in "Baroque Technopop" and "Pastoral Industrial". With VCV, I focus on live aleatoric techno. Music anchored in 4/4 time and Western tonality. I do not care much for ambient, drone, noise, "experimental", xenharmonic music, and other common uses of a modular. If my modules are useful for that purpose, that's only a side-effect.
- There is a ton I do not know. I barely know C++ or music theory. I don't know DSP at all. I want to focus on creating things that help me build skills transferable in other fields I'm interested in. I do not want to embark on learning dead ends that can't be folded back into future creative work.
- Everything module I make must synergize with my own existing modules to form a coherent system.
- I do not own a single hardware module and I have no plans to. I do not care whether it's economically and technically feasible to create hardware versions of my modules.
- I approach the modular as an artist, not as an electrical engineer.
- I will only create modules for as long as the activity holds my interest, and not a minute longer. 
- Professional decorum can fuck off. 



# Matrix mixer using computer vision & physical marbles

See VCV forums: https://community.vcvrack.com/t/diy-marbles-based-physical-matrix-mixer-controller-project/10132




# Srot: sort & rotate polyphonic channels

I need this for my split/merge/sort system to be complete, and for people to start understanding how to use it and pay attention to the potential of the LINK feature. 

The module takes and outputs a poly cable. It can sort it by value (ASC, DESC), by absolute value (ASC, DESC). It can limit output to a certain range of values. It can randomize channel order on trigger. It can output fewer channels than there is input, or be set to have more channels than the last plugged in. It makes use of the LINK feature. 




# Controllers that spring back

Think pitchbend wheel. A series of controllers that can spring back, and can be used as pitchbend/modulation, with all sorts of neat features.

- Meant to be operated with the mouse or touch, not from CV or MIDI. 
- Form factors offered: Wheel (pitchbend/modulation), XY Pad, Joystick (Travel constrained to a circle shape). 
- They can spring back, and you can tweak the tension of the spring in each direction.
- Latch feature: if you push a controller up to the edge, it locks in position, until you push the release button or grab the controller again.
- In pitchbend mode, you can set bend up and down range separately.
- You can quantize input, and you can tell the pitchbend to aim for _n_ scale degrees above/below within the current scale/chord.
- You can also quantize output for a stepped glissando.




# Quantizer / chord sequencer

A quantizer system with scene support, allowing you to create and use external scales (c.f. Darius), making use of the code I already have. Scene support to quickly change the tonality of the piece. You can use it to program chords and scales. 

You can click a button to open a text field, where you can write down a chord progression. It should be very liberal in what it accepts: weird-ass rare jazz chords, inversions, roman numeral notation.




# Soapstone

The module is a sequencer with the basics to drive a simple song: a set BPM and clock output, 4x16 step patterns, 2x16 step notes, 1x16 step CV, quantizer scale, quantizer key.    
You can only change a parameter every few seconds, and the BPM can only be changed by +5 and -5 increments. 

Your change is synchronized with every other instance of Soapstone in the entire world.     
You are told how many users are connected, but you do not know who they are. The others are not required to collaborate with you, so you better be ready to roll with whatever they want to do. 

You cannot set a private channel for collaboration. That's not the point. It's another social experiment like Arcane - and hopefuly one that will garner more interest. 

Probably gonna use websockets, and see if I can get away hosting it on a free hobby cloud plan thing (even at its peak popularity, I do not expect the service to ever have over 50 users). With websockets I can also allow you to change it via a webpage, allowing streamers to play with viewers without requiring VCV. 




# Rotating disk / vinyl turntable

Controlled by hand or via a physical infinite encoder. You can adjust stuff like motor torque, friction, start and stop speed. It outputs its state with various kinds of CV. You can use it as a turntable, wind-up clockwork, power crank, wheel of fortune, jog wheel...




# Gesture sequencer system 

The core idea of the Gesture Sequencer System: for live performance, simple mouse gestures work better than precise virtual knob tweaking. 

The system will:

- Fit my personal needs for expressive and fast live play.
- Not be meant for programming a complex song.
- Acknowledge digitally native UI, favoring interaction patterns that make sense with the mouse, instead of reproducing interactions that make sense in hardware, while remaining a design that could be physically built. 
- Be expandable with new features in the future.

It will be a multi-module system, based on the expander UI metaphor.

I am not sure yet how it will operate internally. Having a bidirectional expander bus across 4 modules is probably a terrible idea. Sharing a single data structure in memory is not thread-safe. I am thinking of allowing each module to read but not write the data of other's, and allow the user to change the channel of each module.

The system is comprised of the following classes of devices, operating in the following order:

Movement → Pattern → Sequence → Processor

To craft a sequencer, you add no more than one module of each class (you can skip the Pattern one).

## Movement

Can be as simple as a built-in clock making each lane progress at a set rate, or individual back/forward/random/walk etc controls per lane. Could also have UDLR controls, treating the sequencer non-linearly. The Movement module essentially moves pawns as if the Sequencer were a board game.

## Pattern

Pattern switcher & song programmer. Can be as simple as a few buttons to swap patterns, and as complex as a computer to program a whole song. Can queue patterns. Can randomize pattern progressions. Optional if you don't need multiple patterns. 

## Sequencer

A grid with 4, 8, 12, or 16 lanes, and up to 64 steps. Grids are not limited to being square. Each cell must have a concept for what it means to go up, down, left, or right. 

When a cell is enabled, clicking disables it.

When a cell is disabled, it accepts mouse gestures, as were introduced decades ago in the Opera browser. Four-directional mouse gestures are a fast and reliable input method, unless you make them so complicated they're Street Fighter combos.

The mouse gesture always start with a click, double-click, triple-click, or even, stay with me on this one: a quadruple click.

After that, the click is held and followed by gestures in the four directions. All the following gestures can be used: U, D, L, R, UD, DU, LR, RL. You can follow up a vertical gesture by a lateral one. Vertical and lateral states are mutually exclusives.

Example gesture: Triple click → Up → Down → Left.

## Processor

The processor gives meaning to the cells and provides outputs. Here's a possible example:

| Gesture              | 303-style output          | Probabilistic Drums | Chord arpeggiator (takes poly v/oct input)
|----------------------|---------------------------|---------------------|----------------------
| Single click         | Toggle step               | Toggle step         | Toggle step 
| Double click         | Set to accent             | Set to Accent       | Toggle step on output 2
| Triple click         | -                         | -                   | Toggle step on output 1 & 2
| Quadruple click      | -                         | -                   | -
| Drag Up              | Octave +1                 | 60% prob            | Octave +1
| Drag Down            | Octave -1                 | 40% prob            | Octave -1
| Drag Down then Up    | Octave +2                 | 80% prob            | Octave +2
| Drag Up then Down    | Octave -2                 | 20% prob            | Octave -2
| Drag Left            | Slide from previous step  | 2x trig             | -
| Drag Right           | Tie with next step        | 4x trig             | -
| Drag Right then Left | Tie and slide both        | Flam                | -
| Drag Left then Right | Tie and slide both        | Ratchet             | -



# Alien communication device
My very own take on the Turing machine. The spaceship of the alien princess leaves our solar system tomorrow. For now she graces earth with a song.

Something that is less concerned with being an elegant electronic circuit, or an elegant algorithm, and is more concerned with the musicality of its output, and the playfulness of its interaction. 

I want it to embrace repetitive, fractal, kaleidoscopic structures, without diving deep into a single-algorithm aesthetic. It should shed any pretense of algorithmic purity and hardcode stuff that just plain sounds good.

Keep the big knob. The big knob is great. When you turn it, you must hear it. And memory banks. 

Build your own relationship to the intrument. It should be labeled in an alien language. The documentation should be absolutely worthless. Its buttons should have an obvious effect, but it should be impossible to explain what that effect is.


# VVR - Voltage Voltage Rectification

Send it a clock, and triggers/gates to four directional Stepchart inputs, one bar in advance. 

Connect a Dance Dance Revolution pad to the Player input. Triggers are steps, gates are holds. 

If the player performs a step/hold perfectly, it gets passed to a specific group of outputs output. Less than perfectly, to another group. If they miss, to a third group. How well they are doing is expressed as a CV. 

This project sounds fun, but it'd be a lot of work. I'd be much more eager to implement it if people are interested in doing something neat with it - especially if there's artists eager to turn it into some sort of participatory installation art. 

And if someone's willing to help with that problem, we could use machine learning to parse the large corpus of existing Stepmania charts out there, and use it to generate stepcharts on the fly. 