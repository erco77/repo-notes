// vim: autoindent tabstop=8 shiftwidth=4 expandtab softtabstop=4

#include <string.h>     // strncmp
// Load all diffs for the specified hash.
//
              vector<Diff> &diffs,
              string& errmsg)
    // Clear previous diffs first
    diffs.clear();

    //    |diff --git a/Makefile b/Makefile             <-- START EACH NEW DIFF HERE
    //    |+++ b/Makefile                               <-- WE NEED THESE FOR filenames[], without the "b/"
    diff.filename("Comments");
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