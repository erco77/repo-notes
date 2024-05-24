// vim: autoindent tabstop=8 shiftwidth=4 expandtab softtabstop=4

#include <string.h>     // strncmp
#include <stdio.h>
#include <errno.h>      // errno..
#include <string.h>     // strerror..

#include <string>
#include <vector>
#include <sstream>      // stringstream
#include <iostream>     // cout debugging
#include <fstream>
#include <iomanip>      // setw

using namespace std;

#include "Subs.H"
#include "Diff.H"
#include "MainWindow.H" // tty->printf()..
#include "version.h"    // VERSION, GITVERSION

// Load all diffs for the specified commit_hash.
//
// Returns:
//    0 on success, commits[] loaded
//   -1 on error (errmsg has reason)
//
int LoadDiffs(const string& commit_hash, vector<Diff> &diffs, string& errmsg)
{
    // Clear previous diffs first
    diffs.clear();
    // Load all diffs for this commit
    vector<string> lines;
    //DEBUG cout << "Loading diffs from commit hash " << commit_hash << ": ";
    //if (LoadCommand_SUBS(string("git show -U10000 ") + commit_hash, lines, errmsg) < 0) {
    if (LoadCommand_SUBS(string("git show ") + commit_hash, lines, errmsg) < 0) {
        return -1;
    }
    //DEBUG cout << lines.size() << " loaded." << endl;

    // Walk lines of output, look for:
    //     1. "diff --git" lines, start new Diff
    //     2. Load all lines into each Diff
    // Example:
    //     ________________________________________________________
    //    |commit 210e73df650ba23011d197a27e6cae5b8b2c9bbe
    //    |Author: Greg Ercolano <erco@seriss.com>
    //    |Date:   Wed Apr 24 22:10:52 2024 -0700
    //    |
    //    |    Initial commit: initial development
    //    |
    //    |diff --git a/Makefile b/Makefile             <-- START EACH NEW DIFF HERE
    //    |new file mode 100644  ^^^^^^^^^^ --> We want this as the filename of the diff, without the "b/"
    //    |index 0000000..e692784
    //    |--- /dev/null
    //    |+++ b/Makefile                               <-- WE NEED THESE FOR filenames[], without the "b/"
    //    |@@ -0,0 +1,8 @@
    //    |+run: repo-notes
    //    |+       ./repo-notes
    //    |..
    //
    int diff_line_num = 0;
    Diff diff;
    diff.commit_hash(commit_hash);
    diff.filename("Comments");
    char diff_filename[512];
    for (int i=0; i<(int)lines.size(); i++) {
        const char *s = lines[i].c_str();
        //DEBUG cout << "Working on: " << s << endl;
        // New diff?
        if (sscanf(s, "diff --git %*s %511s", diff_filename) == 1) {
            //DEBUG cout << "--- DIFF FILENAME: " << diff_filename << endl;
            // Save previous diff first
            diffs.push_back(diff);
            diffs.back().diff_index(diffs.size()-1);   // set diff_index now that we know it
            // Clear diff, set filename, skip leading "b/" prefix if any
            diff.clear();
            diff.diff_index(diffs.size());             // index will be 'next'
            diff_line_num = 0;                         // filename becomes line #0
            if (strncmp(diff_filename, "b/", 2)==0) {
                diff.filename(diff_filename+2);
            } else {
                diff.filename(diff_filename);
            }
        }
        // Add lines to diff
        diff.add_line(lines[i], ++diff_line_num);      // one based
    }
    // Append last diff
    if (diff.filename() != "") {
        diffs.push_back(diff);
    }
    return 0;
}

// Put "<filename>: write error: <system_errmsg>" into errmsg, return -1
int FileWriteError(const string& filename, string& errmsg)
{
    errmsg = filename + string(": write error: ") + string(strerror(errno));
    return -1;
}

// Save DiffLine class's notes to a file
int DiffLine::save_notes(ofstream& ofs, int indent, string& errmsg)
{
    ofs << setw(indent) << "" << "line_num: " << line_num() << "\n";
    for (size_t i=0; i<notes_.size(); i++)
        ofs << setw(indent) << "" << "   notes: " << notes(i) << "\n";
    return ofs.fail() ? -1 : 0;
}

// Load notes from specified file
int DiffLine::load_notes(ifstream &ifs, string& errmsg)
{
    string s;
    notes_.clear();
    while (getline(ifs, s)) {
        StripLeadingWhite_SUBS(s);          // strip leading whitespace
        if (s[0] == '#') continue;          // skip comment lines
        if (s.find("line_num: " ) != string::npos) continue; // ignore
        if (s.find("notes: ") != string::npos) {
            notes_.push_back(s.substr(strlen("notes: ")));
            continue;
        }
        if (s == "}") return 0;             // DiffLine section ends? done
        errmsg = string("unknown command '") + s + string("'");
        return -1;
    }
    return 0;
}

// Show self (debugging)
void DiffLine::show_self() {
    cout << "DiffLine:\n"
         << "    diff_index=" << diff_index_ << "\n"
         << "      line_num=" << line_num_   << "\n"
         << "      line_str=" << line_str_   << "\n";
    for (size_t t=0; t<notes_.size(); t++) {
        cout << "     notes[" << setw(2) << t << "]=" << notes_[t] << "\n";
    }
}


// Save notes for a specified dlp
int Diff::save_notes(DiffLine *dlp,         // DiffLine we're saving
                     string& errmsg)        // if we return -1, this has the error message
{
    // Create notes filename
    int indent = 0;
    const bool create   = true;             // ensures the directory hierarchy will exist
    int        line_num = dlp->line_num();
    string     filename = NotesFilename_SUBS(commit_hash(), diff_index(), line_num, create);  // creates dirs

    // Notes empty? Remove file (if any)
    string notes = dlp->notes();
    StripLeadingWhite_SUBS(notes);
    if (notes == "") {
        if (IsFile_SUBS(filename))          // remove existing notes file (if any)
            DeleteFile_SUBS(filename);
        return 0;                           // done
    }
    // Create the notes file, save the notes
    ofstream ofs(filename);
    if (!ofs) return FileWriteError(filename, errmsg);
    // Save the parent Diff class's contents to a file so it's associated with the note.
    ofs << "# repo-notes VERSION="  << VERSION << ", GIT=" << GITVERSION << "\n"
        << setw(indent) << "" << "diff_index:  " << diff_index()     << "\n"
        << setw(indent) << "" << "commit_hash: " << commit_hash()    << "\n"
        << setw(indent) << "" << "filename:    " << this->filename() << "\n"
        << setw(indent) << "" << "diffline {\n";
    if (ofs.fail()) return FileWriteError(filename, errmsg);
    indent += 4;
    // Save the DiffLine class associated with this note
    if (dlp->save_notes(ofs, indent, errmsg) < 0)
        return FileWriteError(filename, errmsg);
    indent -= 4;
    ofs << setw(indent) << "" << "}\n";
    if (ofs.fail()) return FileWriteError(filename, errmsg);
    ofs.close();
    return 0;
}

// Load notes from specified file
int Diff::load_notes(const string& filename, int diff_index, int line_num, string& errmsg)
{
    DiffLine &dl = diff_lines_[line_num-1];   // index is (line_num-1)
    // Open notes file
    ifstream ifs(filename);
    if (!ifs) {
        errmsg += filename + string(": ") + strerror(errno);
        return -1;
    }

    // Parse all lines in file
    //     Note: most of the Diff commands in this context are informational,
    //           so we just ignore them. We just want to load the DiffLine's notes.
    string s;
    while (getline(ifs, s)) {
        StripLeadingWhite_SUBS(s);                    // strip leading whitespace
        if (s[0] == '#') continue;                    // skip comment lines
        if (s == "") continue;                        // ignore blank lines
        if (s.find("diff_index: " ) != string::npos) continue; // ignore
        if (s.find("commit_hash: ") != string::npos) continue; // ignore
        if (s.find("filename: "   ) != string::npos) continue; // ignore
        if (s.find("diffline {"   ) != string::npos) {
            if (dl.load_notes(ifs, errmsg) < 0) {
                string prefix = filename + string(": ");
                errmsg.insert(0, prefix);
                return -1;
            }
            continue;
        }
        errmsg = filename + string(": unknown command '") + s + string("'");
        return -1;
    }
    return 0;
}

// Return the diff_index from the notes filename
// Example: .repo-notes/commits/f5a4b4b/notes-3-4.txt
//                              commit        | |
//                                            | line#
//                                            |
//                                            diff_index
int GetDiffIndexAndLineNum(const string& filename,
                           int& diff_index,
                           int& line_num,
                           string& errmsg)
{
    int i = filename.find("/notes-");
    if (i >= 0) {
        string notes = filename.substr(i+1);    // get "notes-##-##.txt"
        if (sscanf(notes.c_str(), "notes-%d-%d.txt", &diff_index, &line_num) == 2) {
            return 0;       // success
        }
    }
    errmsg += string("In filename '") + filename
            + string("': can't parse diff_index + line_num integers");
    return -1;
}

// Load a single note file, apply to diffs[] array
int LoadNote(const string& filename, const string& commit_hash, vector<Diff> &diffs, string& errmsg)
{
    // Parse diffs[] index# and diff line# from filename
    int diff_index = -1;
    int line_num   = -1;
    if (GetDiffIndexAndLineNum(filename, diff_index, line_num, errmsg) < 0) return -1;

    // Diff instance loads notes file
    if (diffs[diff_index].load_notes(filename, diff_index, line_num, errmsg) < 0) return -1;
    return 0;
}

// Load notes files
//     Assumes G_diffs[] already populated with all the diffs for this commit,
//     and we just load the notes files we find and apply them to G_diffs[].
//
int LoadCommitNotes(const string& commit_hash, vector<Diff> &diffs, string& errmsg)
{
    errmsg = "";
    bool create = false;    // don't create commit dirs
    vector<string> files;
    vector<string> warnings;
    // Find all notes files in ".repo-notes/<commit>/" dir
    string dirname = CommitDirname_SUBS(commit_hash, create);
    if (!IsDir_SUBS(dirname)) return 0;     // no commit dir? skip
    if (DescendDir_SUBS(dirname, files, warnings, errmsg) < 0) return -1;
    // Load all the notes files and apply them
    string emsg;
    for (size_t i=0; i<files.size(); i++) {
        tty->printf(ANSI_INFO2 "Loading notes file: %s\n" ANSI_NOR, files[i].c_str());
        if (LoadNote(files[i], commit_hash, diffs, emsg) < 0) {
            errmsg += emsg + string("\n");    // accumulate error messages (if any) and report them last
        }
    }
    return errmsg == "" ? 0 : -1;
}
