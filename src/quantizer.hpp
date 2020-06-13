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
		case CHROMATIC: 		return "Chromatic"; break;
		case MAJOR:	 			return "Major"; break;
		case NATURAL_MINOR:		return "Natural Minor"; break;
		case MELODIC_MINOR:		return "Melodic Minor"; break;
		case HARMONIC_MINOR:	return "Harmonic Minor"; break;
		case PENTATONIC_MAJOR:	return "Pentatonic Major"; break;
		case PENTATONIC_MINOR:	return "Pentatonic Minor"; break;
		case BLUES_MAJOR:		return "Blues Major"; break;
		case BLUES_MINOR:		return "Blues Minor"; break;
		case BEBOP_MAJOR:		return "Bebop Major"; break;
		case SPANISH_GYPSY:		return "Spanish Gypsy"; break;
	}
	return "";
}


// The name of the scale from the ScalesEnum, fit to display on a LCD: 8 characters, uppercase or lowercase without descenders.
// First scale being chromatic, an exception can be made in implentations to fit the whole word by removing the key.
inline std::string scaleLcdName(int scale){
	switch(scale){
		case CHROMATIC: 		return "CHROMA. "; break;
		case MAJOR:	 			return "MAJOR   "; break;
		case NATURAL_MINOR:	 	return "n.MINOR "; break;
		case MELODIC_MINOR:		return "m.MINOR "; break;
		case HARMONIC_MINOR:	return "h.MINOR "; break;
		case PENTATONIC_MAJOR:	return "PENTA. M"; break;
		case PENTATONIC_MINOR:	return "PENTA. m"; break;
		case BLUES_MAJOR:		return "BLUES M "; break;
		case BLUES_MINOR:		return "BLUES m "; break;
		case BEBOP_MAJOR:		return "BEBOP M "; break;
		case SPANISH_GYPSY:		return "SP.GYPSY"; break;
	}
	return "";
}


// The note/key name, two characters, sharp notation.
inline std::string noteLcdName(int scale){
	switch(scale){
		case 0:  return "C "; break;
		case 1:  return "C#"; break;
		case 2:  return "D "; break;
		case 3:  return "D#"; break;
		case 4:  return "E "; break;
		case 5:  return "F "; break;
		case 6:  return "F#"; break;
		case 7:  return "G "; break;
		case 8:  return "G#"; break;
		case 9:  return "A "; break;
		case 10: return "A#"; break;
		case 11: return "B "; break;
	}
	return "";
}


// The individual notes of the corresponding scale from the ScalesEnum, in the key of C
inline std::array<bool, 12> validNotesInScale(int scale){
	switch(scale){
		case CHROMATIC: {
			std::array<bool, 12> chromatic 	{true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true};
			return chromatic;
			break;
		}
		case MAJOR: {
			std::array<bool, 12> major 		{true, false,  true, false,  true,  true, false,  true, false,  true, false,  true};
			return major;
			break;
		}
		case NATURAL_MINOR: {
			std::array<bool, 12> n_minor 	{true, false,  true,  true, false,  true, false,  true,  true, false,  true, false};
			return n_minor;
			break;
		}
		case MELODIC_MINOR: {
			std::array<bool, 12> n_minor 	{true, false,  true,  true, false,  true, false,  true, false,  true, false,  true};
			return n_minor;
			break;
		}
		case HARMONIC_MINOR: {
			std::array<bool, 12> n_minor 	{true, false,  true,  true, false,  true, false,  true,  true, false, false,  true};
			return n_minor;
			break;
		}
		case PENTATONIC_MAJOR: {
			std::array<bool, 12> n_minor 	{true, false,  true, false,  true, false, false,  true, false,  true, false, false};
			return n_minor;
			break;
		}
		case PENTATONIC_MINOR: {
			std::array<bool, 12> n_minor 	{true, false, false,  true, false,  true, false,  true, false, false,  true, false};
			return n_minor;
			break;
		}
		case BLUES_MAJOR: {
			std::array<bool, 12> n_minor 	{true, false,  true,  true,  true, false, false,  true, false,  true, false, false};
			return n_minor;
			break;
		}
		case BLUES_MINOR: {
			std::array<bool, 12> n_minor 	{true, false, false,  true, false,  true,  true,  true, false, false,  true, false};
			return n_minor;
			break;
		}
		case BEBOP_MAJOR: {
			std::array<bool, 12> n_minor 	{true, false,  true, false,  true,  true, false,  true,  true,  true, false,  true};
			return n_minor;
			break;
		}
		case SPANISH_GYPSY: {
			std::array<bool, 12> n_minor 	{true,  true, false, false,  true,  true, false,  true,  true, false,  true, false};
			return n_minor;
			break;
		}		
	}
	std::array<bool, 12> none 				{false, false, false, false, false, false, false, false, false, false, false, false};
	return none;
}

// FIXME - this rotation is wrong
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
