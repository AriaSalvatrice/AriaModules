Changelog
=========

The versioning follows this standard: the first number corresponds to the VCV rack version used. The second number is increased any time a new module is added. The third number is increased when something is added or fixed without adding any new module.

## UNRELEASED

- Nothing




## [1.4.2] - 2020-06-19

- [NEW FEATURE] Darius: Added experimental support for Portable sequences from the right-click menu.




## [1.4.1] - 2020-06-16

### Added

- [NEW FEATURE] Arcane, Atout: Added a polyphonic Scale output compatible with Darius' external scale input. The jack formerly labeled Scale is now labeled Melody. 

### Fixed

- [FIX] UnDuLaR: When restarting Rack, the locks are released.
- [FIX] UnDuLaR: Improved jumping by large offsets not having a sufficient overshoot buffer.

### Darius Update

- [NOTE] Darius: Many new features were added to Darius. Despite the scale of the revision, it is an update to the original module, rather than a separate Mk2 version. Most existing patches will not be broken, except those that rely on the external seed input. The width of the module remains the same.
- [DEPRECATED] Darius: The random number generation was improved for greater uniformity. This WILL break patches that relied on the Random Seed input to be deterministic. Sorry for the inconvenience.
- [NEW FEATURE] Darius: Added a LCD providing information about the module state.
- [NEW FEATURE] Darius: The output jacks of nodes that have 0% chance to be reached are no longer lit.
- [NEW FEATURE] Darius: Added an output range control to the CV output.
- [NEW FEATURE] Darius: Can now start the sequence on any step. The "Steps" knob is now known as the "Last step" knob.
- [NEW FEATURE] Darius: Added undo support for the two "Randomize" buttons.
- [NEW FEATURE] Darius: Added route and CV presets in the right-click menu.
- [NEW FEATURE] Darius: Added an optional quantizer, with support for external scales.
- [FIX] Darius: Fixed random seed not working with multiple instances of Darius.
- [FIX] Darius: Individual trigger/gate outputs now work with every input and the step button, not just with the forward input. This may break old patches.




## [1.4.0] - 2020-02-11

### Added

- [NEW MODULE] UnDuLaR: Scroll the rack via CV for live performance.

### Fixed

- [FIX] Arcane: The Fool card no longer crashes the rack. The oracle server will NOT draw the Fool for a week after release to give people time to upgrade.
- [FIX] Arcane, Atout: LCD loading message changed from "DOWNLOADING" to "WAIT ON D/L" on secondary instances, as only the first added attempts a download. 




## [1.3.0] - 2020-01-29

### Added

- [NEW MODULE] Arcane: Today's fortune ★
- [NEW MODULE] Atout: Today's fortune ★
- [NEW MODULE] Aleister: Today's fortune ★
- [NEW FEATURE] Darius: All trigger inputs now accept polyphonic input. Existing patches are very unlikely to be broken by this new feature. If they are, they can be fixed by splitting any polyphonic trigger input then sending only the first channel.
- [NEW FEATURE] Darius: Added a "RND" seed CV input, and a switch to decide to refresh it on "1st/all" step(s), to force a specific random seed to be used.
- [NEW FEATURE] Darius: Added a CV range selector: 0V~10V or -5V~5V. CV Knobs now default to the center on new instances for ease of use in -5V~5V mode.
- [NEW FEATURE] Darius: Added new trigger controls: Back, Up, and Down. The "Step" CV input is now called "Forward" for consistency.

### Changed

- [CHANGE] Darius: Moved jacks and buttons around a little to accomodate new features.
- [CHANGE] All existing modules: Unlit jacks and knobs are now blue instead of gray, following the established design language better.

### Fixed

- [FIX] Darius: Loading a patch saved in the middle of a sequence shows the full path traveled, instead of just the current step.
- [FIX] Darius: Initializing the device now resets it to the first node.
- [FIX] Splort, Smerge, Spleet, Swerge, Splirge: Increased the refresh rate of lights, as they were optimized too aggressively.




## [1.2.0] - 2020-01-17

### Added

- [NEW MODULE] Darius: Branching 8-step sequencer taking a random path through its nodes




## [1.1.0] - 2020-01-13

### Added

- [NEW MODULE] Splort: 16 channels polyphonic split with optional chainable sort mode
- [NEW MODULE] Smerge: 16 channels polyphonic merge with optional chainable sort mode
- [NEW MODULE] Spleet: Dual 4 channels / single 8 channels polyphonic split with optional sort mode
- [NEW MODULE] Swerge: Dual 4 channels / single 8 channels polyphonic merge with optional sort mode
- [NEW MODULE] Blank Plate: A complimentary blank plate and ♥-head screwdriver are provided with every Signature Series module purchase
- [NEW FEATURE] Splirge: Sort mode added.

### Changed

- [CHANGE] Splirge: New panel layout. The merge and split sections are swapped around to make it easier to understand the internal wiring. Existing patches will still work as before. 

### Fixed

- [FIX] Splirge: Fixed merging non-contiguous channels not resetting unplugged channels to 0v.




## [1.0.0] - 2019-12-13

### Added

- [NOTE] First public release, just a simple module to get started making them
- [NEW MODULE] Splirge - 3hp 4 channels split and merge
