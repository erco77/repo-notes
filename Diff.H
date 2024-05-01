// vim: autoindent tabstop=8 shiftwidth=4 expandtab softtabstop=4

#ifndef DIFF_H
#define DIFF_H

#include <string>
#include <vector>

using namespace std;

// A single line in the diffs
class DiffLine {
    string filename_;    // XXX: could be a ptr? ref? ptr back to parent Diff?
    int    line_num_;    // line number from top of diff file
    string line_str_;    // line contents
    string notes_;       // user notes

public:
    DiffLine()  { line_num_ = -1; }
    ~DiffLine() { }

    // Set/get diff filename
    void filename(const char* val)   { filename_ = val; }
    void filename(const string& val) { filename_ = val; }
    string filename() const          { return filename_; }

    void line_num(int val)           { line_num_ = val;  }
    int  line_num() const            { return line_num_; }

    void line_str(const char* val)   { line_str_ = val;  }
    void line_str(const string& val) { line_str_ = val;  }
    string line_str() const          { return line_str_; }

    void notes(const string& val)    { notes_ = val; }
    void notes(const char* val)      { notes_ = val; }
    string notes() const             { return notes_; }
};

//
// Diff - A single file's diffs
//
class Diff {
    string filename_;                   // filename for this diff
    vector<DiffLine> diff_lines_;       // lines of diff
public:
    Diff()  { }
    ~Diff() { }

    // Set/get diff filename
    void filename(const char* val) { filename_ = val; }
    void filename(string& val)     { filename_ = val; }
    string filename() const        { return filename_; }

    // Add a diff line
    void add_line(const char* val, int line_num) {
        DiffLine dl;
        dl.filename(filename());        // XXX: needless copy
        dl.line_str(val);
        dl.line_num(line_num);
        diff_lines_.push_back(dl);
    }

    // Add a diff line
    void add_line(string &val, int line_num) {
        DiffLine dl;
        dl.filename(filename());        // XXX: needless copy
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
};

int LoadDiffs(string& hash, vector<Diff> &diffs, string& errmsg);

#endif
