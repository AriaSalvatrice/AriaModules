#pragma once

namespace Quantizer {

// Modes are not supported
enum ScalesEnum {
	CHROMATIC,
	MAJOR,
	NATURAL_MINOR,
	MELODIC_MINOR,
	HARMONIC_MINOR,
	PENTATONIC_MAJOR,
	PENTATONIC_MINOR,
	BLUES_MAJOR,
	BLUES_MINOR,
	BEBOP_MAJOR,
	SPANISH_GYPSY,
	NUM_SCALES
};


// The name of the scale from the ScalesEnum, with proper capitalization
// Not used yet!
inline std::string scaleDisplayName(int scale){
	switch(scale){
		case CHROMATIC: 		return "Chromatic";
		case MAJOR:	 			return "Major";
		case NATURAL_MINOR:		return "Natural Minor";
		case MELODIC_MINOR:		return "Melodic Minor";
		case HARMONIC_MINOR:	return "Harmonic Minor";
		case PENTATONIC_MAJOR:	return "Pentatonic Major";
		case PENTATONIC_MINOR:	return "Pentatonic Minor";
		case BLUES_MAJOR:		return "Blues Major";
		case BLUES_MINOR:		return "Blues Minor";
		case BEBOP_MAJOR:		return "Bebop Major";
		case SPANISH_GYPSY:		return "Spanish Gypsy";
	}
	return "";
}


// The name of the scale from the ScalesEnum, fit to display on a LCD: 8 characters, uppercase or lowercase without descenders.
// First scale being chromatic, an exception can be made in implentations to fit the whole word by removing the key.
inline std::string scaleLcdName(int scale){
	switch(scale){
		case CHROMATIC: 		return "CHROMA. ";
		case MAJOR:	 			return "MAJOR   ";
		case NATURAL_MINOR:	 	return "n.MINOR ";
		case MELODIC_MINOR:		return "m.MINOR ";
		case HARMONIC_MINOR:	return "h.MINOR ";
		case PENTATONIC_MAJOR:	return "PENTA. M";
		case PENTATONIC_MINOR:	return "PENTA. m";
		case BLUES_MAJOR:		return "BLUES M ";
		case BLUES_MINOR:		return "BLUES m ";
		case BEBOP_MAJOR:		return "BEBOP M ";
		case SPANISH_GYPSY:		return "SP.GYPSY";
	}
	return "";
}


// The note/key name, two characters, sharp notation.
inline std::string noteLcdName(int scale){
	switch(scale){
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


// The individual notes of the corresponding scale from the ScalesEnum, in the key of C
inline std::array<bool, 12> validNotesInScale(int scale){
	switch(scale){
		case CHROMATIC: {
			std::array<bool, 12> chromatic 	{true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true};
			return chromatic;
		}
		case MAJOR: {
			std::array<bool, 12> major 		{true, false,  true, false,  true,  true, false,  true, false,  true, false,  true};
			return major;
		}
		case NATURAL_MINOR: {
			std::array<bool, 12> n_minor 	{true, false,  true,  true, false,  true, false,  true,  true, false,  true, false};
			return n_minor;
		}
		case MELODIC_MINOR: {
			std::array<bool, 12> n_minor 	{true, false,  true,  true, false,  true, false,  true, false,  true, false,  true};
			return n_minor;
		}
		case HARMONIC_MINOR: {
			std::array<bool, 12> n_minor 	{true, false,  true,  true, false,  true, false,  true,  true, false, false,  true};
			return n_minor;
		}
		case PENTATONIC_MAJOR: {
			std::array<bool, 12> n_minor 	{true, false,  true, false,  true, false, false,  true, false,  true, false, false};
			return n_minor;
		}
		case PENTATONIC_MINOR: {
			std::array<bool, 12> n_minor 	{true, false, false,  true, false,  true, false,  true, false, false,  true, false};
			return n_minor;
		}
		case BLUES_MAJOR: {
			std::array<bool, 12> n_minor 	{true, false,  true,  true,  true, false, false,  true, false,  true, false, false};
			return n_minor;
		}
		case BLUES_MINOR: {
			std::array<bool, 12> n_minor 	{true, false, false,  true, false,  true,  true,  true, false, false,  true, false};
			return n_minor;
		}
		case BEBOP_MAJOR: {
			std::array<bool, 12> n_minor 	{true, false,  true, false,  true,  true, false,  true,  true,  true, false,  true};
			return n_minor;
		}
		case SPANISH_GYPSY: {
			std::array<bool, 12> n_minor 	{true,  true, false, false,  true,  true, false,  true,  true, false,  true, false};
			return n_minor;
		}		
	}
	std::array<bool, 12> none 				{false, false, false, false, false, false, false, false, false, false, false, false};
	return none;
}

// The individual notes of the corresponding scale from the ScalesEnum, in the specified key
inline std::array<bool, 12> validNotesInScaleKey(int scale, int key){
	std::array<bool, 12> notes = validNotesInScale(scale);
	std::rotate(notes.rbegin(), notes.rbegin() + key, notes.rend());
	return notes;
}


// Quantizes the voltage to the scale, expressed as a bool[12] starting on C. 12TET only.
inline float quantize(float voltage, std::array<bool, 12> validNotes) {
	float octave = floorf(voltage);
	float voltageOnFirstOctave = voltage - octave;
	float currentComparison;
	float currentDistance;
	float closestNoteFound = 10.0;
	float closestNoteDistance = 10.0;
	
	// Iterate notes and seek the closest match
	for (int note = 0; note < 12; note++) {
		if (validNotes[note]) {
			currentComparison = note / 12.f;
			currentDistance = fabs(voltageOnFirstOctave - currentComparison);
			if (currentDistance < closestNoteDistance) { 
				closestNoteFound = currentComparison;
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
				closestNoteDistance = currentDistance;
			}
			break;
		}
	}	
	// Return best match, or pass output as-is if nothing found.
	if (closestNoteDistance < 10.0) {
		voltage = octave + closestNoteFound;
	}
	return clamp(voltage, -10.f, 10.f);
}

// Note name and octave
inline std::string noteName(float voltage) {
	voltage = voltage * 12.f + 60.f;
	int octave = (int) voltage / 12 - 1;
	int note = (int) voltage % 12;
	std::string noteName = noteLcdName(note);
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

} // Quantizer
