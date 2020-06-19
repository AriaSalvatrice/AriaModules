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

};

// Contains one or more PortableSequence::Note
struct Sequence {
    float length;
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

    // Can also set it explicitly instead FIXME
    void calculateLength(){
        int lastStartingNote = 0;
        for(std::size_t i = 0; i < notes.size() - 1; i++)
            lastStartingNote = (notes[i].start > notes[i+1].start) ? i : i+1;
        length = notes[lastStartingNote].start + notes[lastStartingNote].length;
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

    // Copies the portable sequence to the user's clipboard
    // Does not clamp or sort first - do it explicitly if desired.
    void toClipboard() {
        json_t* sequenceJ = this->toJson();
        char* sequenceC = json_dumps(sequenceJ, JSON_INDENT(2) | JSON_REAL_PRECISION(9));
        glfwSetClipboardString(APP->window->win, sequenceC);
        free(sequenceC);
        json_decref(sequenceJ);
    }

};


} // PortableSequence