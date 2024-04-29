// vim: autoindent tabstop=8 shiftwidth=4 expandtab softtabstop=4

#include "Subs.H"
#include "Diff.H"

#include <string.h>     // strncmp
#include <iostream>

// Load all diffs for the specified hash.
//
// Returns:
//    0 on success, commits[] loaded
//   -1 on error (errmsg has reason)
//
int LoadDiffs(string& hash,
              vector<Diff> &diffs,
              string& errmsg)
{
    // Clear previous diffs first
    diffs.clear();

    // Load all diffs for this commit
    vector<string> lines;
    cout << "Loading diffs from hash " << hash << ": ";
    if (LoadCommand_SUBS(string("git show -U10000 ") + hash, lines, errmsg) < 0) {
        return -1;
    }
    cout << lines.size() << " loaded." << endl;

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
    Diff diff;
    diff.filename("Comments");
    char diff_filename[512];
    for (int i=0; i<(int)lines.size(); i++) {
        const char *s = lines[i].c_str();
        cout << "Working on: " << s << endl;
        // New diff?
        if (sscanf(s, "diff --git %*s %511s", diff_filename) == 1) {
            cout << "--- DIFF FILENAME: " << diff_filename << endl;
            // Save previous diff first
            diffs.push_back(diff);
            // Clear diff, set filename, skip leading "b/" prefix if any
            diff.clear();
            if (strncmp(diff_filename, "b/", 2)==0) {
                diff.filename(diff_filename+2);
            } else {
                diff.filename(diff_filename);
            }
        }
        // Add lines to diff
        diff.add_line(lines[i]);
    }
    // Append last diff
    if (diff.filename() != "") {
        diffs.push_back(diff);
    }
    return 0;
}

