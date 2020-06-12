namespace Quantizer {

enum ScalesEnum {
	CHROMATIC,
	MAJOR,
	MINOR
};

// The name of the scale from the ScalesEnum, with proper capitalization
std::string scaleDisplayName(int scale){
	switch(scale){
		case CHROMATIC: 	return "Chromatic"; break;
		case MAJOR:	 		return "Major"; break;
		case MINOR:	 		return "Minor"; break;
	}
	return "";
}

// The name of the scale from the ScalesEnum, fit to display on a LCD: 8 uppercase characters.
std::string scaleLcdName(int scale){
	switch(scale){ //              "--------"
		case CHROMATIC: 	return "CHROMA"; break;
		case MAJOR:	 		return "MAJOR"; break;
		case MINOR:	 		return "MINOR"; break;
	}
	return "";
}

// The individual scale degrees of the corresponding scale from the ScalesEnum, in the key of C
std::array<bool, 12> scaleDegrees(int scale){
	switch(scale){
		case CHROMATIC: {
			std::array<bool, 12> chromatic 		{true , true , true , true , true , true , true , true , true , true , true , true};
			return chromatic;
			break;
		}
		case MAJOR: {
			std::array<bool, 12> major 			{true , false, true , false, true , true , false, true , false, true , false, true};
			return major;
			break;
		}
		case MINOR: {
			std::array<bool, 12> major 			{true , false, true , true , false, true , false, true , true , false, true , false};
			return major;
			break;
		}
	}
	std::array<bool, 12> none 					{false, false, false, false, false, false, false, false, false, false, false, false};
	return none;
}

// Quantizes the voltage to the scale.
// Scale is a bool[12] starting on C. 12TET only.
float quantize(float voltage, std::array<bool, 12> scaleDegrees) {
	float octave = floorf(voltage);
	float voltageOnFirstOctave = voltage - octave;
	float currentComparison;
	float currentDistance;
	float closestNoteFound = 10.0;
	float closestNoteDistance = 10.0;
	
	// Iterate scale degrees and seek the closest match
	for (int sd = 0; sd < 12; sd++) {
		if (scaleDegrees[sd]) {
			currentComparison = sd / 12.f;
			currentDistance = fabs(voltageOnFirstOctave - currentComparison);
			if (currentDistance < closestNoteDistance) { 
				closestNoteFound = currentComparison;
				closestNoteDistance = currentDistance;
			}
		}
	}
	// Next, verify whether going up one octave would bring us even closer
	for (int sd = 0; sd < 12; sd++) {
		if (scaleDegrees[sd]) {
			currentComparison = sd / 12.f + 1.f;
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

} // Quantizer