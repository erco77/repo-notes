#include <stdio.h>      // fopen..
#include <errno.h>      // errno..
#include <string.h>     // strerror..

#include <string>
#include <vector>
#include <sstream>      // stringstream
int LoadDiffs(const string& hash, vector<Diff> &diffs, string& errmsg)
    diff.hash(hash);
            diffs.back().diff_index(diffs.size());     // set diff_index now that we know it
            diff.diff_index(diffs.size()+1);           // index will be 'next'  // I YAM HERE!!
            diff_line_num = 0;                         // filename becomes line #0
        diff.add_line(lines[i], ++diff_line_num);      // one based


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

// Save notes for a specified dlp
int Diff::savenotes(DiffLine *dlp,          // DiffLine we're saving
                    string& errmsg)         // if we return -1, this has the error message 
{
    // Create notes filename
    int indent = 0;
    const bool create     = true;           // ensures the directory hierarchy will exist
    int        line_num   = dlp->line_num();
    string notes_filename = NotesFilename_SUBS(hash(), diff_index(), line_num, create);  // creates dirs

    // Create the notes file, save the notes
    cout << "DEBUG: fopen(" << notes_filename << ") for write" << endl;
    FILE *fp = fopen(notes_filename.c_str(), "w");
    if (fp == NULL) {
        errmsg = string("can't create file: ") + string(strerror(errno));
        goto diff_write_err;
    }
    // Save the parent Diff class's contents to a file so it's associated with the note.
    if (fprintf(fp, "%*sdiff_index: %ld\n", indent, "",       diff_index()) < 0) goto diff_write_err;
    if (fprintf(fp, "%*shash:       %s\n",  indent, "",     hash().c_str()) < 0) goto diff_write_err;
    if (fprintf(fp, "%*sfilename:   %s\n",  indent, "", filename().c_str()) < 0) goto diff_write_err;

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