/*             DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
                    Version 2, December 2004

 Copyright (C) 2004 Sam Hocevar <sam@hocevar.net>

 Everyone is permitted to copy and distribute verbatim or modified
 copies of this license document, and changing it is allowed as long
 as the name is changed.

            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION

  0. You just DO WHAT THE FUCK YOU WANT TO.
*/
#pragma once

namespace Quantizer {

// A little offset to fudge against rounding errors.
// Otherwise, problem arise, for example quantizing a signal that's already a valid semitone 
// might give inconsistent results depending on the octave. FLoating point math is stupid.
const float FUDGEOFFSET = 0.001f;


// Except for Major/natural minor & pentatonic scales, I avoided scales that are modes of another.
// I wanted a curation limited to interesting instant satisfaction presets that
// work well with generative patterns and sound good to average modern western ears.
// This list is now set in stone forever to avoid breaking patches. It should never be changed.
enum ScalesEnum {
    CHROMATIC,
    MAJOR,
    NATURAL_MINOR,
    MELODIC_MINOR,
    HARMONIC_MINOR,
    PENTATONIC_MAJOR,
    PENTATONIC_MINOR,
    WHOLE_TONE,
    BLUES_MAJOR,
    BLUES_MINOR,
    DOMINANT_DIMINISHED,
    BEBOP_MAJOR,
    BEBOP_MINOR,
    DOUBLE_HARMONIC,
    EIGHT_TONE_SPANISH,
    HIRAJOSHI,
    IN_SEN,
    NUM_SCALES
};


// The name of the scale from the ScalesEnum, with proper capitalization
inline std::string scaleDisplayName(const int& scale){
    switch(scale){
        case CHROMATIC: 			return "Chromatic";
        case MAJOR:	 				return "Major";
        case NATURAL_MINOR:			return "Natural Minor";
        case MELODIC_MINOR:			return "Melodic Minor";
        case HARMONIC_MINOR:		return "Harmonic Minor";
        case PENTATONIC_MAJOR:		return "Pentatonic Major";
        case PENTATONIC_MINOR:		return "Pentatonic Minor";
        case WHOLE_TONE:			return "Whole Tone";
        case BLUES_MAJOR:			return "Blues Major";
        case BLUES_MINOR:			return "Blues Minor";
        case DOMINANT_DIMINISHED:	return "Dominant Diminished";
        case BEBOP_MAJOR:			return "Bebop Major";
        case BEBOP_MINOR:			return "Bebop Minor";
        case DOUBLE_HARMONIC:		return "Double Harmonic";
        case EIGHT_TONE_SPANISH:	return "Eight Tone Spanish";
        case HIRAJOSHI:				return "Hirajōshi";
        case IN_SEN:				return "In Sen";
    }
    return "";
}


// The name of the scale from the ScalesEnum, fit to display on a LCD: 8 characters, uppercase or lowercase without descenders.
// First scale being chromatic, an exception can be made in implentations to fit the whole word by removing the key.
// When synonyms exist, names are generally chosen to fit on the LCD.
inline std::string scaleLcdName(const int& scale){
    switch(scale){
        case CHROMATIC: 			return "CHROMA. ";
        case MAJOR:	 				return "MAJOR   ";
        case NATURAL_MINOR:	 		return "n.MINOR ";
        case MELODIC_MINOR:			return "m.MINOR ";
        case HARMONIC_MINOR:		return "h.MINOR ";
        case PENTATONIC_MAJOR:		return "PENTA. M";
        case PENTATONIC_MINOR:		return "PENTA. m";
        case WHOLE_TONE:			return "WHOLE T.";
        case BLUES_MAJOR:			return "BLUES M ";
        case BLUES_MINOR:			return "BLUES m ";
        case DOMINANT_DIMINISHED:	return "DOM. dim";
        case BEBOP_MAJOR:			return "BEBOP M ";
        case BEBOP_MINOR:			return "BEBOP m ";
        case DOUBLE_HARMONIC:		return "DbHARMO.";
        case EIGHT_TONE_SPANISH:	return "8SPANISH";
        case HIRAJOSHI:				return "HIRAJO. ";
        case IN_SEN:				return "IN SEN  ";
    }
    return "";
}


// The individual notes of the corresponding scale from the ScalesEnum, in the key of C
inline std::array<bool, 12> validNotesInScale(const int& scale){
    switch(scale){
        case CHROMATIC: {
            std::array<bool, 12> s {true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true};
            return s;
        }
        case MAJOR: {
            std::array<bool, 12> s {true, false,  true, false,  true,  true, false,  true, false,  true, false,  true};
            return s;
        }
        case NATURAL_MINOR: {
            std::array<bool, 12> s {true, false,  true,  true, false,  true, false,  true,  true, false,  true, false};
            return s;
        }
        case MELODIC_MINOR: {
            std::array<bool, 12> s {true, false,  true,  true, false,  true, false,  true, false,  true, false,  true};
            return s;
        }
        case HARMONIC_MINOR: {
            std::array<bool, 12> s {true, false,  true,  true, false,  true, false,  true,  true, false, false,  true};
            return s;
        }
        case PENTATONIC_MAJOR: {
            std::array<bool, 12> s {true, false,  true, false,  true, false, false,  true, false,  true, false, false};
            return s;
        }
        case PENTATONIC_MINOR: {
            std::array<bool, 12> s {true, false, false,  true, false,  true, false,  true, false, false,  true, false};
            return s;
        }
        case WHOLE_TONE: {
            std::array<bool, 12> s {true, false,  true, false,  true, false,  true, false,  true, false,  true, false};
            return s;
        }
        case BLUES_MAJOR: {
            std::array<bool, 12> s {true, false,  true,  true,  true, false, false,  true, false,  true, false, false};
            return s;
        }
        case BLUES_MINOR: {
            std::array<bool, 12> s {true, false, false,  true, false,  true,  true,  true, false, false,  true, false};
            return s;
        }
        case DOMINANT_DIMINISHED: {
            std::array<bool, 12> s {true,  true, false,  true,  true, false,  true,  true, false,  true,  true, false};
            return s;
        }
        case BEBOP_MAJOR: {
            std::array<bool, 12> s {true, false,  true, false,  true,  true, false,  true,  true,  true, false,  true};
            return s;
        }
        case BEBOP_MINOR: {
            std::array<bool, 12> s {true, false,  true,  true,  true,  true, false,  true, false,  true,  true, false};
            return s;
        }
        case DOUBLE_HARMONIC: {
            std::array<bool, 12> s {true,  true, false, false,  true,  true, false,  true,  true, false, false,  true};
            return s;
        }
        case EIGHT_TONE_SPANISH: {
            std::array<bool, 12> s {true,  true, false,  true,  true,  true,  true, false,  true, false,  true, false};
            return s;
        }
        case HIRAJOSHI: {
            std::array<bool, 12> s {true,  true, false, false, false,  true,  true, false, false, false,  true, false};
            return s;
        }
        case IN_SEN: {
            std::array<bool, 12> s {true,  true, false, false, false,  true, false,  true, false, false,  true, false};
            return s;
        }
    }
    std::array<bool, 12> none     {false, false, false, false, false, false, false, false, false, false, false, false};
    return none;
}


// The note/key name, two characters, sharp notation.
inline std::string keyLcdName(const int& key){
    switch(key){
        case 0:  return "C ";
        case 1:  return "C#";
        case 2:  return "D ";
        case 3:  return "D#";
        case 4:  return "E ";
        case 5:  return "F ";
        case 6:  return "F#";
        case 7:  return "G ";
        case 8:  return "G#";
        case 9:  return "A ";
        case 10: return "A#";
        case 11: return "B ";
    }
    return "";
}

// The note/key name, two characters, sharp notation.
// ! is an empty space as large as a normal character on the segment display font
// The font I use has no # symbol so I use * instead.
inline std::string keySegmentName(const int& key){
    switch(key){
        case 0:  return "C!";
        case 1:  return "C*";
        case 2:  return "D!";
        case 3:  return "D*";
        case 4:  return "E!";
        case 5:  return "F!";
        case 6:  return "F*";
        case 7:  return "G!";
        case 8:  return "G*";
        case 9:  return "A!";
        case 10: return "A*";
        case 11: return "B!";
    }
    return "";
}

// The individual notes of the corresponding scale from the ScalesEnum, in the specified key
inline std::array<bool, 12> validNotesInScaleKey(const int& scale, const int& key){
    std::array<bool, 12> notes = validNotesInScale(scale);
    std::rotate(notes.rbegin(), notes.rbegin() + key, notes.rend());
    return notes;
}


// Quantizes the voltage to the scale, expressed as a bool[12] starting on C. 12TET only.
// After quantizing, can optionally transpose up or down by scale degrees
inline float quantize(float voltage, const std::array<bool, 12>& validNotes, int transposeSd = 0) {

    voltage = voltage + FUDGEOFFSET;

    float octave = floorf(voltage);
    float voltageOnFirstOctave = voltage - octave;
    float currentComparison;
    float currentDistance;
    float closestNoteFound = 10.0;
    float closestNoteDistance = 10.0;
    int closestNoteFoundNum = 0;
    
    // Iterate notes and seek the closest match
    for (int note = 0; note < 12; note++) {
        if (validNotes[note]) {
            currentComparison = note / 12.f;
            currentDistance = fabs(voltageOnFirstOctave - currentComparison);
            if (currentDistance < closestNoteDistance) { 
                closestNoteFound = currentComparison;
                closestNoteFoundNum = note;
                closestNoteDistance = currentDistance;
            }
        }
    }
    // Next, verify whether going up one octave would bring us even closer
    for (int note = 0; note < 12; note++) {
        if (validNotes[note]) {
            currentComparison = note / 12.f + 1.f;
            currentDistance = fabs(voltageOnFirstOctave - currentComparison);
            if (currentDistance < closestNoteDistance) { 
                closestNoteFound = currentComparison;
                closestNoteFoundNum = note;
                closestNoteDistance = currentDistance;
            }
            break;
        }
    }

    if (closestNoteDistance < 10.0) {
        // We found a match
        voltage = octave + closestNoteFound;
        // Transpose it by scale degrees if requested
        if (transposeSd != 0) {
            // Keep it to a sane range just in case
            transposeSd = clamp(transposeSd, -120, 120); 
            if (transposeSd > 0) {
                // Add scale degrees
                for (int i = 0; i < transposeSd;) {
                    voltage += 1.f / 12.f;
                    closestNoteFoundNum++;
                    if (closestNoteFoundNum == 12) closestNoteFoundNum = 0;
                    // Because we found a match earlier we know there are some validNotes,
                    // so the loop should never get stuck.
                    if (validNotes[closestNoteFoundNum]) i++;
                }
            } else {
                // Subtract scale degrees
                for (int i = 0; i < abs(transposeSd);) {
                    voltage -= 1.f / 12.f;
                    closestNoteFoundNum--;
                    if (closestNoteFoundNum == -1) closestNoteFoundNum = 11;
                    if (validNotes[closestNoteFoundNum]) i++;
                }
            }
        }
    } else {
        // Pass output as-is when no match found (happens when there's no valid notes)
    }
    return clamp(voltage, -10.f, 10.f);
}

// C3 = 0, C#5 = 1, D8 = 2, etc.
inline size_t quantizeToPositionInOctave(float voltage, const std::array<bool, 12>& validNotes) {
    voltage = quantize(voltage, validNotes);
    voltage = voltage * 12.f + 60.f;
    return (int) voltage % 12;
}

// Note name and octave, for display on Lcd
inline std::string noteOctaveLcdName(float voltage) {
    voltage = voltage * 12.f + 60.f;
    int octave = (int) voltage / 12 - 1;
    int note = (int) voltage % 12;
    std::string noteName = keyLcdName(note);
    noteName.append(std::to_string(octave));
    return noteName;
}

// Note name and octave, for display on segment display fonts (spaces are ! symbols)
inline std::string noteOctaveSegmentName(float voltage) {
    voltage = voltage * 12.f + 60.f;
    int octave = (int) voltage / 12 - 1;
    int note = (int) voltage % 12;
    std::string noteName = keySegmentName(note);
    noteName.append(std::to_string(octave));
    return noteName;
}

// Which note to light on the LCD
inline std::array<bool, 12> pianoDisplay(float voltage) {
    std::array<bool, 12> notes;
    voltage = voltage * 12.f + 60.f;
    int note = (int) voltage % 12;
    for (int i = 0; i < 12; i++)
        notes[i] = ( i == note ) ? true : false;
    return notes;
}

inline size_t scaleDegreeCountInScale(std::array<bool, 12> scale) {
    size_t count = 0;
    for (size_t i = 0; i < 12; i++) {
        if (scale[i]) count++;
    }
    return count;
}

} // namespace Quantizer
