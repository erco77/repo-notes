// vim: autoindent tabstop=8 shiftwidth=4 expandtab softtabstop=4

#include "Subs.H"
#include "Diff.H"

#include <string.h>     // strncmp
#include <stdio.h>      // fopen..
#include <errno.h>      // errno..
#include <string.h>     // strerror..

#include <string>
#include <vector>
#include <sstream>      // stringstream
#include <iostream>     // cout debugging
#include <fstream>

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
    if (LoadCommand_SUBS(string("git show -U10000 ") + commit_hash, lines, errmsg) < 0) {
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


// Save DiffLine class's notes to a file
int DiffLine::savenotes(FILE *fp, int indent, string& errmsg)
{
    if (fprintf(fp, "%*sline_num: %d\n", indent, "", line_num_) < 0) goto dl_write_err;
    for (size_t i=0; i<notes_.size(); i++) {
        if (fprintf(fp, "%*snotes: %s\n", indent, "", notes_[i].c_str()) < 0) goto dl_write_err;
    }
    return 0;

dl_write_err:
    errmsg = string("write error: ") + string(strerror(errno));
    return -1;
}

// Load notes from specified file
int DiffLine::loadnotes(ifstream &ifs, string& errmsg)
{
    string s;
    notes_.clear();
    while (getline(ifs, s)) {
        StripLeadingWhite_SUBS(s);             // strip leading whitespace
        if (s.find("line_num: " ) != string::npos) continue; // ignore
        if (s.find("notes: ") != string::npos) {
            notes_.push_back(s.substr(strlen("notes: ")));
            continue;
        }
        if (s == "}") return 0;                // DiffLine section ends? done
        errmsg = string("unknown command '") + s + string("'");
        return -1;
    }
    return 0;
}

// Save notes for a specified dlp
int Diff::savenotes(DiffLine *dlp,          // DiffLine we're saving
                    string& errmsg)         // if we return -1, this has the error message 
{
    // Create notes filename
    int indent = 0;
    const bool create     = true;           // ensures the directory hierarchy will exist
    int        line_num   = dlp->line_num();
    string notes_filename = NotesFilename_SUBS(commit_hash(), diff_index(), line_num, create);  // creates dirs

    // Create the notes file, save the notes
    cout << "DEBUG: fopen(" << notes_filename << ") for write" << endl;
    FILE *fp = fopen(notes_filename.c_str(), "w");
    if (fp == NULL) {
        errmsg = string("can't create file: ") + string(strerror(errno));
        goto diff_write_err;
    }
    // Save the parent Diff class's contents to a file so it's associated with the note.
    if (fprintf(fp, "%*sdiff_index:  %ld\n", indent, "",          diff_index()) < 0) goto diff_write_err;
    if (fprintf(fp, "%*scommit_hash: %s\n",  indent, "", commit_hash().c_str()) < 0) goto diff_write_err;
    if (fprintf(fp, "%*sfilename:    %s\n",  indent, "", filename().c_str()   ) < 0) goto diff_write_err;

    // Save the DiffLine class associated with this note
    if (fprintf(fp, "%*sdiffline {\n", indent, "") < 0) goto diff_write_err;
    indent += 4;
    if (dlp->savenotes(fp, indent, errmsg) < 0) goto diff_write_err;
    indent -= 4;
    if (fprintf(fp, "%*s}\n", indent, "") < 0) goto diff_write_err;
    fclose(fp);
    return 0;

diff_write_err:
    stringstream ss;
    ss << "Diff::savenotes(): '" << notes_filename << "': " << errmsg;
    errmsg = ss.str();
    if (fp) fclose(fp);
    return -1;
}

// Find line_num in diff_lines[] and return index, or -1 on error
int Diff::find_line_num(int line_num, string& errmsg)
{
    // TODO: Do we need a linear lookup? Or is line# the index#
    for (size_t i=0; i<diff_lines_.size(); i++)
        if (diff_lines_[i].line_num() == line_num)
            return i;
    // Not found? fail..
    stringstream ss;
    ss << "Diff file " << filename() << ": couldn't find line_num " << line_num << " in diff_lines[]";
    errmsg = ss.str();
    return -1;
}

// Load notes from specified file
int Diff::loadnotes(const string& filename, int diff_index, int line_num, string& errmsg)
{
    // Find index into diff_lines_[] for specified line#
    size_t dli = find_line_num(line_num, errmsg);
    if (dli < 0) return -1;
    DiffLine &dl = diff_lines_[dli];
    cout << "DEBUG: Applying " << filename << " to diff file "
         << this->filename() << ", line_num " << line_num << endl;

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
        if (s == "") continue;                        // ignore blank lines
        if (s.find("diff_index: " ) != string::npos) continue; // ignore
        if (s.find("commit_hash: ") != string::npos) continue; // ignore
        if (s.find("filename: "   ) != string::npos) continue; // ignore
        if (s.find("diffline {"   ) != string::npos) {
            if (dl.loadnotes(ifs, errmsg) < 0) {
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
    if (diffs[diff_index].loadnotes(filename, diff_index, line_num, errmsg) < 0) return -1;
    return 0;
}

// Load notes files
//     Assumes G_diffs[] already populated with all the diffs for this commit,
//     and we just load the notes files we find and apply them to G_diffs[].
//
int LoadCommitNotes(const string& commit_hash, vector<Diff> &diffs, string& errmsg)
{
    errmsg = "";
    bool create = true;
    vector<string> files;
    vector<string> warnings;
    // Find all notes files in ".repo-notes/<commit>/" dir
    string dirname = CommitDirname_SUBS(commit_hash, create);
    if (DescendDir_SUBS(dirname, files, warnings, errmsg) < 0) return -1;
    // Load all the notes files and apply them
    string emsg;
    for (size_t i=0; i<files.size(); i++) {
        cout << "--- Loading notes file: " << files[i] << endl;
        if (LoadNote(files[i], commit_hash, diffs, emsg) < 0) {
            // accumulate error messages (if any) and report them last
            errmsg += emsg + string("\n");
        }
    }
    return errmsg == "" ? 0 : -1;
}

// for (const auto &file : files) {
//     cout << file << endl;
// }
