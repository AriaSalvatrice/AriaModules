Changelog
=========

The versioning follows this standard: the first number corresponds to the VCV rack version used. The second number is increased any time a new module family is added. The third number is increased when something is added or fixed without adding a new module family.

## [1.8.1b] - 2021-12-02

### Licensing

- [CHANGE] This collection is relicensed from GPL3-only to GPL3-or-later, to allow inclusion in [Cardinal](https://github.com/DISTRHO/Cardinal).


## [1.8.1] - 2020-10-21

### Fixed

- [FIX] Psychopump: Fixed buttons 1 & 2 not showing up in case-sensitive filesystems.




## [1.8.0] - 2020-10-21

### New Modules

- [NEW] Psychopump: 8 triggered channels of CV presets and S&H with randomization. Pairs great with percussive sound sources to send your beats straight to Hades.

### Changed

- [CHANGE] Modulus Salomonis Regis, Modulellus Salomonis Regis, Modulissimus Salomonis Regis: Removed the visual link between Load and Reset on the faceplate, as this default behavior can be disabled by the user.
- [CHANGE] QQQQ: Altered the design of the keyboard button for consistency.
- [CHANGE] All modules with a LCD: Altered the SVG font to hint at the pixel grid better on HDPI displays. It _is_ supposed to look low-tech and pixelated.

### Fixed

- [FIX] Darius: Fixed slide knob updating slowly.




## [1.7.1] - 2020-10-11

### Added

- [NEW] All modules: Partial support for [Lights Off](https://library.vcvrack.com/ModularFungi/LightsOff). It is currently very difficult to provide high quality support for it, so most modules won't look perfect.

### Changed

- [OPTIMIZATION] Darius: No longer processes at audio rates. This greatly improves its performance. Audio rate processing had no practical uses and caused bugs. Find a better waveshaper! Or open a feature request if you really want that option restored.
- [CHANGE] Darius: When the first step is greater than 1, the path leading up to the first node is now dimmer, to make it easier to see which step is the first.
- [CHANGE] QQQQ, Quack, Q<: Poly External Scale output no longer forwards information about the tonic for now, as there are too many ambiguous cases, UI-wise. Better modules to program scales and chords, or possibly a revised QQQQ UI, will be added in the future. Because there are no modules consuming Poly External Scale data with a defined tonic yet, this is all very hypothetical anyway.
- [NEW FEATURE] Quale: The 1st channel of the chord received is now considered the tonic in Poly External Scales. This can be disasbled via right-click menu. No module makes use of this information yet.
- [CHANGE] Many small improvements to widgets, most of them not user-visible. The most noticeable is the removal of halos from LEDs, as they are slated for removal in VCV 2.0 anyway. The least noticeable is that I moved the pipe character on the LCD by 1 pixel to the right to make it align to the piano on Modulus Salomonis Regis.
- [CHANGE] Many small code improvements to most modules, most of them not user-visible, to keep the collection more maintainable in the future. 

### Fixed

- [FIX] QQQQ: Fix various scenarios where scenes are recalled incorrectly on init.
- [FIX] Grabby: The Grabby is no longer incorrectly labeled a Rotato.




## [1.7.0] - 2020-08-17

### New Modules

- [NEW] Pokies: 4 tiny buttons for automation or manual performance of CV parameters. Right-click options to change output values, and use in latch mode.
- [NEW] Grabby: A lil fader for automation or manual performance of CV parameters. Right-click options to offset and invert output. Support for Poly External Scales from Qqqq.
- [NEW] Rotatoes: 4 tiny knobs for automation or manual performance of CV parameters. Right-click options to offset and invert output. Support for Poly External Scales from Qqqq.




## [1.6.1]  - 2020-07-25

### Added

- [NEW] QQQQ: New right-click option to advance scene with trigs instead

### Changed

- [CHANGE] Arcane, Atout: The Poly External Scale output now sends 8V for enabled semitons, for future expansion of the Poly External Scale format.
- [CHANGE] Darius: The minimum voltage to toggle a semitone from a Poly External Scale is now 0.1V, for future expansion of the Poly External Scale format.
- [CHANGE] QQQQ, Quack, Q<, Quale: The minimum voltage to toggle a semitone from a Poly External Scale is now 0.1V, for future expansion of the Poly External Scale format.
- [CHANGE] QQQQ, Quack: The Poly External Scale output now sends 8V for enabled semitones, and 10V for the key if it is enabled on the piano display, for future expansion of the Poly External Scale format.
- [CHANGE] Modulus Salomonis Regis, Modulellus Salomonis Regis, Modulissimus Salomonis Regis : The minimum voltage to toggle a semitone from a Poly External Scale is now 0.1V, for future expansion of the Poly External Scale format.

### Fixed

- [FIX] Qqqq: Fixed reloading a patch with a scene CV cable patched in reloading the wrong scale if the signal received changes after reload. 




## [1.6.0] - 2020-07-21

### New Modules

- [NEW] Modulus Salomonis Regis: Self-modifying, self-patching sequencer. 8 nodes version.
- [NEW] Modulellus Salomonis Regis: Self-modifying, self-patching sequencer. 4 nodes version.
- [NEW] Modulissimus Salomonis Regis: Self-modifying, self-patching sequencer. 16 nodes version.

### Documentation

- [NEW] A new documentation site is available: <https://aria.dog/modules/>
- [REMOVED] The Github documentation was removed in favor of the new documentation site.

### Changed

- [CHANGE] Darius: New instances are now by default in quantizer mode, in the C Minor scale, with a limited CV range. Existing patches will not have their settings changed. 
- [CHANGE] Darius: 1ms wait on reset implemented, as per recommended best practice.

### Fixed

- [FIX] Qqqq: Fixed crash when importing chord progressions longer than 16 chords.
- [FIX] Qqqq: Improved clamping of extremely high/low input.




## [1.5.1] - 2020-07-10

### New modules

- [NEW] Quale: Convert chords to scales and scales to chords, using Quatherina's polyphonic scale representation.

### Changed

- [CHANGE] Darius: The position of the Key, Min, and Max knobs have been swapped for consistency across the module collection. This visual change will not break existing patches.
- [CHANGE] Qqqq, Quack: The position of the Key and Scale knobs have been swapped for consistency across the module collection. This visual change will not break existing patches.




## [1.5.0] - 2020-07-02

### New modules

- [NEW] Quatherina the Quantum Duck presents Quatherine's Quality Quad Quantizer (Qqqq): A quantizer with custom & external scales, 16 scenes, offset, sample and hold, transposition, and lead sheet chords parsing.
- [NEW] Quack: Smaller version of Qqqq, with piano buttons, and a single polyphonic quantizer column.
- [NEW] Q<: Tiny version of Qqqq, with a single polyphonic quantizer column.

### Changed

- [CHANGE] All modules: Small changes to the visual design of various widgets. Includes improvements in accessibility to users with tritanopia.
- [CHANGE] UnDuLaR: For the first 10 seconds after being initialized, UnDuLaR does nothing. This is a simpler way to remove a misconfigured instance than stopping the sample rate. 




## [1.4.2] - 2020-06-19

### Added 

- [NEW] Darius: Added experimental support for Portable sequences from the right-click menu.




## [1.4.1] - 2020-06-16

### Added

- [NEW] Arcane, Atout: Added a polyphonic Scale output compatible with Darius' external scale input. The jack formerly labeled Scale is now labeled Melody. 

### Fixed

- [FIX] UnDuLaR: When restarting Rack, the locks are released.
- [FIX] UnDuLaR: Improved jumping by large offsets not having a sufficient overshoot buffer.

### Darius Update

- [NOTE] Darius: Many new features were added to Darius. Despite the scale of the revision, it is an update to the original module, rather than a separate Mk2 version. Most existing patches will not be broken, except those that rely on the external seed input. The width of the module remains the same.
- [DEPRECATED] Darius: The random number generation was improved for greater uniformity. This WILL break patches that relied on the Random Seed input to be deterministic. Sorry for the inconvenience.
- [NEW] Darius: Added a LCD providing information about the module state.
- [NEW] Darius: The output jacks of nodes that have 0% chance to be reached are no longer lit.
- [NEW] Darius: Added an output range control to the CV output.
- [NEW] Darius: Can now start the sequence on any step. The "Steps" knob is now known as the "Last step" knob.
- [NEW] Darius: Added undo support for the two "Randomize" buttons.
- [NEW] Darius: Added route and CV presets in the right-click menu.
- [NEW] Darius: Added an optional quantizer, with support for external scales.
- [FIX] Darius: Fixed random seed not working with multiple instances of Darius.
- [FIX] Darius: Individual trigger/gate outputs now work with every input and the step button, not just with the forward input. This may break old patches.




## [1.4.0] - 2020-02-11

### Added

- [NEW] UnDuLaR: Scroll the rack via CV for live performance.

### Fixed

- [FIX] Arcane: The Fool card no longer crashes the rack. The oracle server will NOT draw the Fool for a week after release to give people time to upgrade.
- [FIX] Arcane, Atout: LCD loading message changed from "DOWNLOADING" to "WAIT ON D/L" on secondary instances, as only the first added attempts a download. 




## [1.3.0] - 2020-01-29

### New modules

- [NEW] Arcane: Today's fortune ★
- [NEW] Atout: Today's fortune ★
- [NEW] Aleister: Today's fortune ★

### Added

- [NEW] Darius: All trigger inputs now accept polyphonic input. Existing patches are very unlikely to be broken by this new feature. If they are, they can be fixed by splitting any polyphonic trigger input then sending only the first channel.
- [NEW] Darius: Added a "RND" seed CV input, and a switch to decide to refresh it on "1st/all" step(s), to force a specific random seed to be used.
- [NEW] Darius: Added a CV range selector: 0V~10V or -5V~5V. CV Knobs now default to the center on new instances for ease of use in -5V~5V mode.
- [NEW] Darius: Added new trigger controls: Back, Up, and Down. The "Step" CV input is now called "Forward" for consistency.

### Changed

- [CHANGE] Darius: Moved jacks and buttons around a little to accomodate new features.
- [CHANGE] All existing modules: Unlit jacks and knobs are now blue instead of gray, following the established design language better.

### Fixed

- [FIX] Darius: Loading a patch saved in the middle of a sequence shows the full path traveled, instead of just the current step.
- [FIX] Darius: Initializing the device now resets it to the first node.
- [FIX] Splort, Smerge, Spleet, Swerge, Splirge: Increased the refresh rate of lights, as they were optimized too aggressively.




## [1.2.0] - 2020-01-17

### New modules

- [NEW] Darius: Branching 8-step sequencer taking a random path through its nodes




## [1.1.0] - 2020-01-13

### New modules

- [NEW] Splort: 16 channels polyphonic split with optional chainable sort mode
- [NEW] Smerge: 16 channels polyphonic merge with optional chainable sort mode
- [NEW] Spleet: Dual 4 channels / single 8 channels polyphonic split with optional sort mode
- [NEW] Swerge: Dual 4 channels / single 8 channels polyphonic merge with optional sort mode
- [NEW] Blank Plate: A complimentary blank plate and ♥-head screwdriver are provided with every Signature Series module purchase

### Added

- [NEW] Splirge: Sort mode added.

### Changed

- [CHANGE] Splirge: New panel layout. The merge and split sections are swapped around to make it easier to understand the internal wiring. Existing patches will still work as before. 

### Fixed

- [FIX] Splirge: Fixed merging non-contiguous channels not resetting unplugged channels to 0v.




## [1.0.0] - 2019-12-13

### New modules

- [NEW] Splirge - 3hp 4 channels split and merge
