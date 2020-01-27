float quantize(float voltage, bool scaleDegrees[12]) {
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

