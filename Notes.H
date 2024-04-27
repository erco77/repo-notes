#ifndef NOTES_H
#define NOTES_H

#include <string>
#include <vector>

using namespace std;

// Note
//    A single note for a specific commit/filename/line offset
//
class Note {
    string commit_;             // commit hash for this note
    string filename_;           // filename this note is assigned to
    int    line_;               // line# into CommitFileDiff for note to be inserted
    string note_;               // the note's text

public:
    Note() {
        line_ = -1;
    }
    void commit(const string& val)   { commit_   = val; }
    void filename(const string& val) { filename_ = val; }
    void line(int val)               { line_     = val; }
    void note(const string& val)     { note_     = val; }
};

// CommitNotes
//    All notes for a particular commit
//
class CommitNotes {
    string       commit_;          // commit hash for this array of notes
    vector<Note> notes_;           // all notes for this commit
public:
    CommitNotes() { }
    ~CommitNotes() { }
    void commit(const string& val) { commit_ = val; }
    void add_note(Note& note)      { notes_.push_back(note); }
    int  total_notes(void) const   { return notes_.size(); }
    Note& get_note(int index)      { return notes_[index]; }
};


#endif
