Changelog
=========

## Planned for 1.3.0 or later, not implemented at all yet

- [PLANNED - NEW MODULE] UnDuLaR - Scroll and zoom the rack via CV input
- [PLANNED - NEW FEATURE] Darius: trigger inputs now accept polyphonic input. **Existing patches are very unlikely to be broken by this new feature. If they are, they can be fixed by splitting any polyphonic trigger input then sending only the first channel.**
- [PLANNED - NEW FEATURE] Darius: added a "Step Back" CV input, and moved the inputs a bit around the faceplate. 
- [PLANNED - NEW FEATURE] Darius: using the "Randomize" buttons is now stored in Undo history. 
- [PLANNED - NEW FEATURE] Darius: added a CV polarity selector: unipolar (0V~10V) or bipolar (-5V~5V). CV Knobs now default to the center on new instances. 
- [PLANNED - FIX] Darius: Initializing the device now resets it to the first node.


## Unreleased 1.3.0

### Added

- [NEW MODULE] Arcane - Today's fortune ★
- [NEW MODULE] Atout - Today's fortune ★
- [NEW MODULE] Aleister - Today's fortune ★

### Changed

- [CHANGE] All existing modules: Unlit jacks and knobs are now blue instead of gray, following the established design language better.
- [FIX] Splort, Smerge, Spleet, Swerge, Splirge: increased the refresh rate of lights, as they were optimized too aggressively.



## [1.2.0] - 2020-01-17

### Added

- [NEW MODULE] Darius - Branching 8-step sequencer taking a random path through its nodes



## [1.1.0] - 2020-01-13

### Added

- [NOTE] Now this collection is starting to have some useful modules! This plugin now provides a fully-featured Split and Merge series.
- [NEW MODULE] Splort - 16 channels polyphonic split with optional chainable sort mode
- [NEW MODULE] Smerge - 16 channels polyphonic merge with optional chainable sort mode
- [NEW MODULE] Spleet - Dual 4 channels / single 8 channels polyphonic split with optional sort mode
- [NEW MODULE] Swerge - Dual 4 channels / single 8 channels polyphonic merge with optional sort mode
- [NEW MODULE] Blank Plate - A complimentary blank plate and ♥-head screwdriver are provided with every Signature Series module purchase

### Changed

- [NEW FEATURE] Splirge: Sort mode added.
- [CHANGE] Splirge: New panel layout. The merge and split sections are swapped around to make it easier to understand the internal wiring. Existing patches will still work as before. 
- [FIX] Splirge: Fixed merging non-contiguous channels not resetting unplugged channels to 0v.



## [1.0.0] - 2019-12-13

### Added

- [NOTE] First public release, just a simple module to get started making them
- [NEW MODULE] Splirge - 3hp 4 channels split and merge