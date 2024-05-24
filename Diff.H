// vim: autoindent tabstop=8 shiftwidth=4 expandtab softtabstop=4

#ifndef DIFF_H
#define DIFF_H

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

#include "Subs.H"          // StringToLines_SUBS()..

using namespace std;

// A single line in the diffs
class DiffLine {
    size_t diff_index_;    // parent Diff's index in G_diffs[]
    int    line_num_;      // line number from top of diff file
    string line_str_;      // line contents
    vector<string> notes_; // user notes

public:
    DiffLine() {
        diff_index_ = -1;
        line_num_ = -1;
    }
    ~DiffLine() { }

    void   diff_index(size_t val)    { diff_index_ = val; }
    size_t diff_index(void) const    { return diff_index_; }

    void line_num(int val)           { line_num_ = val;  }
    int  line_num() const            { return line_num_; }

    void line_str(const char* val)   { line_str_ = val;  }
    void line_str(const string& val) { line_str_ = val;  }
    string line_str() const          { return line_str_; }

    // Example use of notes(int):
    //     for (size_t i=0; i<notes_size(); i++) {
    //         printf("%ld) %s\n", i, notes(i).c_str());
    //     }
    //
    void   notes(const string& val)  { notes_.clear(); StringToLines_SUBS(val, notes_); }
    void   notes(const char *val)    { notes(string(val)); }
    size_t notes_size(void) const    { return notes_.size(); }
    string notes(int index) const    { return notes_[index]; }
    string notes(void) const {
        size_t size = notes_.size();
        string out;
        for (size_t i=0; i<size; i++) {
            out += notes_[i];
            if (i < (size-1)) out += "\n";
        }
        return out;
    }
    int save_notes(ofstream& ofs, int indent, string& errmsg);
    int load_notes(ifstream &ifs, string& errmsg);
    void show_self();
};

//
// Diff - A single file's diffs
//
class Diff {
    size_t diff_index_;                 // G_diffs[] index
    string commit_hash_;                // commit hash for this diff
    string filename_;                   // filename for this diff
    vector<DiffLine> diff_lines_;       // lines of diff
public:
    Diff() { diff_index_ = -1; }
    ~Diff() { }

    void diff_index(size_t val)    { diff_index_ = val; }
    size_t  diff_index(void) const { return diff_index_; }

    void commit_hash(const char* val)   { commit_hash_ = val; }
    void commit_hash(const string& val) { commit_hash_ = val; }
    string commit_hash() const          { return commit_hash_; }

    // Set/get diff filename
    void filename(const char* val) { filename_ = val; }
    void filename(string& val)     { filename_ = val; }
    string filename() const        { return filename_; }

    // Add a diff line
    //    NOTE: Assumes our diff_index() is valid
    //
    void add_line(const string &val, int line_num) {
        DiffLine dl;
        dl.diff_index(diff_index());       // pass Diff's G_diffs[] index to DiffLine
        dl.line_str(val);
        dl.line_num(line_num);
        diff_lines_.push_back(dl);
    }

    // Get total diff lines
    size_t diff_lines(void) const { return diff_lines_.size(); }

    // Get a specific diff line (by its index, 0..lines())
    DiffLine& diff_line(size_t index) { return diff_lines_[index]; }

    // Clear class
    void clear(void) { filename(""); diff_lines_.clear(); }
    int save_notes(DiffLine *dlp, string& errmsg);
    int load_notes(const string& filename, int diff_index, int line_num, string& errmsg);
};

int LoadDiffs(const string& commit_hash, vector<Diff> &diffs, string& errmsg);
int LoadCommitNotes(const string& commit_hash, vector<Diff> &diffs, string& errmsg);

#endif
