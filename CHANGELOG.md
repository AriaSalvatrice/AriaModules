Changelog
=========

The versioning follows this standard: the first number corresponds to the VCV rack version used. The second number is increased any time a new module is added. The third number is increased when something is added or fixed without adding any new module.


## UNRELEASED

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
