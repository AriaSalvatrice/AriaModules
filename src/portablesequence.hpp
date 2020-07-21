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
#include "plugin.hpp"

// Implements the Portable Sequences interchange format: 
// https://github.com/squinkylabs/SquinkyVCV/blob/master/docs/clipboard-format.md
//
// This library has not been battle tested yet, beware before reusing.
// NOTE - I do not understand jansson memory management stuff well. There might be leaks.
// 
namespace PortableSequence{

struct Note {
    // Required. 1.f per quarter note.
    float start = 0.f;
    // Required. V/Oct. Can be negative.
    float pitch = 0.f;
    // Required. 1.f per quarter note.
    float length = 0.f;
    // Optional. 0.f-10.f, not exported if out of range.
    float velocity = -1.f;
    // Optional. 0.f-1.f, not exported if out of range.
    float playProbability = -1.f;

    bool hasVelocity() {
        return (velocity >= 0.f);
    }

    bool hasPlayProbability() {
        return (playProbability >= 0.f);
    }

    // Clamps all of this note's values to their legal range.
    // Call explicitly to ensure correct output.
    void clampValues() {
        start = clamp(start, 0.f, INFINITY);
        pitch = clamp(pitch, -10.f, 10.f);
        length = clamp(length, 0.f, INFINITY);
        if (velocity >= 0.f) velocity = clamp(velocity, 0.f, 10.f);
        if (playProbability >= 0.f) playProbability = clamp(playProbability, 0.f, 1.f);
    }

    json_t* toJson() {
        json_t *rootJ = json_object();
        json_object_set(rootJ, "type", json_string("note"));
        json_object_set(rootJ, "start", json_real(start));
        json_object_set(rootJ, "pitch", json_real(pitch));
        json_object_set(rootJ, "length", json_real(length));
        if (velocity >= 0.f) json_object_set(rootJ, "velocity", json_real(velocity));
        if (playProbability >= 0.f) json_object_set(rootJ, "playProbability", json_real(playProbability));
        return rootJ;
    }

    // No fromJson for notes - doesn't seem to have any use case

}; // Note

// Contains one or more PortableSequence::Note
struct Sequence {
    float length = 0.f;
    std::vector<PortableSequence::Note> notes;

    // Added to the end
    void addNote(const PortableSequence::Note &note) {
        notes.push_back(note);
    }

    // Clamps all of this sequence's values to their legal range.
    void clampValues(){
        for(std::size_t i = 0; i < notes.size(); i++) notes[i].clampValues();
    }

    // Sorts by note start.
    void sort(){
        std::sort(notes.begin(), notes.end(), [](PortableSequence::Note a, PortableSequence::Note b){
            return a.start < b.start;
        });
    }

    // Length is best set explicitly, but can be calculated if missing
    // FIXME: Might be broken - check
    void calculateLength(){
        for(std::size_t i = 0; i < notes.size() - 1; i++)
            length = ((notes[i].start + notes[i].length) > length) ? (notes[i].start + notes[i].length) : length;
    }

    json_t* toJson() {
        json_t *rootJ = json_object();
        json_t *vcvrackSequenceJ  = json_object();
        json_t *notesJ = json_array();
        for(std::size_t i = 0; i < notes.size(); i++) json_array_append(notesJ, notes[i].toJson());
        json_object_set(vcvrackSequenceJ, "length", json_real((float) length));
        json_object_set(vcvrackSequenceJ, "notes", notesJ);
        json_object_set(rootJ, "vcvrack-sequence", vcvrackSequenceJ);
        return rootJ;
    }

    // Returns true if import successful.
    // Does not validate data in great detail - so clamp it after import.
    bool fromJson(const char* clipboard) {
        json_error_t error;
        json_t* rootJ = json_loads(clipboard, 0, &error);
        if (!rootJ) {
            WARN("Portable Sequence: Could not parse clipboard as JSON");
            return false;
        }
        json_t* vcvrackSequenceJ = json_object_get(rootJ, "vcvrack-sequence");
        if (!vcvrackSequenceJ) {
            WARN("Portable Sequence: No vcvrack-sequence data found");
            return false; 
        }
        json_t* notesJ = json_object_get(vcvrackSequenceJ, "notes");
        if (!notesJ) {
            WARN("Portable Sequence: No notes data found");
            return false; 
        }
        for(std::size_t i = 0; i < json_array_size(notesJ); i++) {
            json_t* noteJ = json_array_get(notesJ, i);
            PortableSequence::Note note;
            note.start = json_real_value(json_object_get(noteJ, "start"));
            note.pitch = json_real_value(json_object_get(noteJ, "pitch"));
            note.length = json_real_value(json_object_get(noteJ, "length"));
            json_t* velocityJ = json_object_get(noteJ, "velocity");
            json_t* playProbabilityJ = json_object_get(noteJ, "playProbability");
            note.velocity = (velocityJ) ? json_real_value(velocityJ) : -1.f;
            note.playProbability = (playProbabilityJ) ? json_real_value(playProbabilityJ) : -1.f;
            addNote(note);
        }
        json_t* lengthJ = json_object_get(vcvrackSequenceJ, "length");
        if (!lengthJ) {
            WARN("Portable Sequence: No global length found. It will be automatically calculated instead.");
            calculateLength();
        } else {
            length = json_real_value(lengthJ);
        }
        return true;
    }

    // Copies the portable sequence to the user's clipboard
    // Does not clamp or sort first - do so before explicitly if desired.
    void toClipboard() {
        json_t* sequenceJ = this->toJson();
        char* sequenceC = json_dumps(sequenceJ, JSON_INDENT(2) | JSON_REAL_PRECISION(9));
        glfwSetClipboardString(APP->window->win, sequenceC);
        free(sequenceC);
        json_decref(sequenceJ);
    }

    // Returns true if import successful
    bool fromClipboard() {
        const char* clipboard = glfwGetClipboardString(APP->window->win);
        return (clipboard == NULL) ? false : fromJson(clipboard);
    }

}; // Sequence

} // PortableSequence
