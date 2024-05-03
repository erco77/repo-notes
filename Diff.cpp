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
#include <iostream>

// Load all diffs for the specified hash.
//
// Returns:
//    0 on success, commits[] loaded
//   -1 on error (errmsg has reason)
//
int LoadDiffs(const string& hash,
              vector<Diff> &diffs,
              string& errmsg)
{
    // Clear previous diffs first
    diffs.clear();

    // Load all diffs for this commit
    vector<string> lines;
    //DEBUG cout << "Loading diffs from hash " << hash << ": ";
    if (LoadCommand_SUBS(string("git show -U10000 ") + hash, lines, errmsg) < 0) {
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
    diff.hash(hash);
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
            // Clear diff, set filename, skip leading "b/" prefix if any
            diff.clear();
            diff_line_num = 0;      // filename becomes line #0
            if (strncmp(diff_filename, "b/", 2)==0) {
                diff.filename(diff_filename+2);
            } else {
                diff.filename(diff_filename);
            }
        }
        // Add lines to diff
        diff.add_line(lines[i], ++diff_line_num);   // one based
    }
    // Append last diff
    if (diff.filename() != "") {
        diffs.push_back(diff);
    }
    return 0;
}


int DiffLine::savenotes(FILE *fp,
                        int indent,
                        string& errmsg) {
    if (fprintf(fp, "%*sline_num: %d\n", indent, "", line_num_) < 0) goto dl_write_err;
    for (size_t i=0; i<notes_.size(); i++) {
        if (fprintf(fp, "%*snotes: %s\n", indent, "", notes_[i].c_str()) < 0) goto dl_write_err;
    }
    return 0;

dl_write_err:
    errmsg = string("write error: ") + string(strerror(errno));
    return -1;
}

// Save notes for a specified dlp
int Diff::savenotes(int diff_index,         // our unique index#
                    DiffLine *dlp,          // DiffLine we're saving
                    string& errmsg)         // if we return -1, this has the error message 
{
    // Create notes filename
    int indent = 0;
    const bool create     = true;           // ensures the directory hierarchy will exist
    int    line_num       = dlp->line_num();
    string notes_filename = NotesFilename_SUBS(hash(), diff_index, line_num, create);  // creates dirs

    // Create the notes file, save the notes
    FILE *fp = fopen(notes_filename.c_str(), "w");
    if (!fp) {
        errmsg = string("can't create file: ") + string(strerror(errno));
        goto diff_write_err;
    }
    // Save the parent Diff class's contents to a file so it's associated with the note.
    if (fprintf(fp, "%*shash: \"%s\"\n",     indent, "",     hash().c_str()) < 0) goto diff_write_err;
    if (fprintf(fp, "%*sfilename: \"%s\"\n", indent, "", filename().c_str()) < 0) goto diff_write_err;

    // Save the DiffLine class associated with this note
    {
        if (fprintf(fp, "%*sdiffline {\n", indent, "") < 0) goto diff_write_err;
        indent += 4;
        if (dlp->savenotes(fp, indent, errmsg) < 0) goto diff_write_err;
        indent -= 4;
        if (fprintf(fp, "%*s}\n", indent, "") < 0) goto diff_write_err;
    }
    return 0;

diff_write_err:
    stringstream ss;
    ss << "Diff::savenotes(): '" << notes_filename << "': " << errmsg;
    errmsg = ss.str();
    if (fp) fclose(fp);
    return -1;
}
