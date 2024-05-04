#include <iostream>     // cout debugging
#include <fstream>
// Load all diffs for the specified commit_hash.
int LoadDiffs(const string& commit_hash, vector<Diff> &diffs, string& errmsg)
    //DEBUG cout << "Loading diffs from commit hash " << commit_hash << ": ";
    if (LoadCommand_SUBS(string("git show -U10000 ") + commit_hash, lines, errmsg) < 0) {
    diff.commit_hash(commit_hash);
            diffs.back().diff_index(diffs.size()-1);   // set diff_index now that we know it
            diff.diff_index(diffs.size());             // index will be 'next'
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

    string notes_filename = NotesFilename_SUBS(commit_hash(), diff_index(), line_num, create);  // creates dirs
    if (fprintf(fp, "%*sdiff_index:  %ld\n", indent, "",          diff_index()) < 0) goto diff_write_err;
    if (fprintf(fp, "%*scommit_hash: %s\n",  indent, "", commit_hash().c_str()) < 0) goto diff_write_err;
    if (fprintf(fp, "%*sfilename:    %s\n",  indent, "", filename().c_str()   ) < 0) goto diff_write_err;

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