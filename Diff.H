// vim: autoindent tabstop=8 shiftwidth=4 expandtab softtabstop=4

#ifndef DIFF_H
#define DIFF_H

#include <string>
#include <vector>

using namespace std;

//
// Diff - A single file's diffs
//
class Diff {
    string filename_;           // filename for this diff
    vector<string> diff_lines_; // lines of diff
    // TODO: Add vector<Note> here too? May be several for each diff
public:
    Diff()  { }
    ~Diff() { }

    // Set diff filename
    void filename(const char* val) { filename_ = val; }
    void filename(string& val)     { filename_ = val; }
    string filename() const        { return filename_; }

    // Add a diff line
    void add(const char* val)  { diff_lines_.push_back(string(val)); }
    void add(string &val)      { diff_lines_.push_back(val); }

    // Clear class
    void clear(void) { filename(""); diff_lines_.clear(); }
};

int LoadDiffs(string& hash, vector<Diff> &diffs, string& errmsg);

#endif
