// vim: autoindent tabstop=8 shiftwidth=4 expandtab softtabstop=4

// Make notes on a repo's diffs

#include <sys/stat.h>   // mkdir(), etc
#include <sys/types.h>
#include <stdio.h>      // popen
#include <string.h>     // strchr
#include <unistd.h>
#include <fcntl.h>

#include <string>       // std::string
#include <vector>       // std::vector
#include <iostream>     // cout, cerr
#include <sstream>

using namespace std;

// Fltk
#include <FL/Fl.H>
#include <FL/fl_message.H>
#include <FL/Fl_Menu_Button.H>
#include "MainWindow.H"
#include "EditNotes.H"

// Back end
#include "Subs.H"       // subs to support this project
#include "Notes.H"      // our notes
#include "Commit.H"     // commits from git
#include "Diff.H"       // single commit diff
#include "version.h"

// Globals
vector<Diff> G_diffs;   // all diffs for current project
                        // TODO: notes should be in here too

EditNotesDialog *G_editnotes = 0;

// Rebuild the diffs_browser contents from a diffs[] array
void UpdateDiffsBrowser(vector<Diff>& diffs)
{
    int save = diffs_browser->value();
    diffs_browser->clear();

    // For each diff, add FILENAME and DIFF LINES to browser..
    for (size_t di=0; di<diffs.size(); di++ ) { // diff index
        Diff& diff = diffs[di];

        // Add diff filename
        string filename;
        const char *filename_fmt = "@N"      // Use fl_inactive_color() to draw the text
                                   "@B47"    // Fill the background with fl_color(n)
                                   "@b"      // Use a bold font (adds FL_BOLD to font)
                                   "@l"      // Use a LARGE (24 point) font
                                   "@.";     // Print rest of line, don't look for more '@' signs
        const char *grn_fmt   = "@C60@.";    // green foreground text
        const char *red_fmt   = "@C1@.";     // red foreground text
        const char *nor_fmt   = "@.";        // normal text (no color)
        const char *notes_fmt = "@B16@S8@."; // lt blue bg, small text
        filename = string(filename_fmt) + diff.filename();
        diffs_browser->add(filename.c_str());

        // Add diff lines
        for (size_t dli=0; dli<diff.diff_lines(); dli++) {  // vector<DiffLine> index
            DiffLine &dl = diff.diff_line(dli);
            string line_str = dl.line_str();
            switch (line_str[0]) {
                case '+': line_str = string(grn_fmt) + line_str; break;
                case '-': line_str = string(red_fmt) + line_str; break;
                default:  line_str = string(nor_fmt) + line_str; break;
            }
            diffs_browser->add(line_str.c_str(), (void*)&dl);

            // Check to see if this DiffLine has notes.
            //     If it does, add the notes /below/ the diff line
            //     and set the bgcolor to something to make it easily identifyable.
            //
            if (dl.notes_size() == 0) continue; // no notes? skip
            // Add all lines of the user's notes
            for (size_t i=0; i<dl.notes_size(); i++) {
                string out = string(notes_fmt)  // add notes prefix '@' formatting
                           + string("   ")      // indent
                           + dl.notes(i);       // content
                // add formatted line to browser, associated with this dl (DiffLine)
                diffs_browser->add(out.c_str(), (void*)&dl);    // diffs_browser's userdata() is DiffLine
            }
        }
    }
    diffs_browser->value(save);
}

// Run 'git log' and put its output in the left browser
void UpdateGitLogBrowser(vector<Commit>& commits)
{
    git_log_browser->clear();
    for (size_t i=0; i<commits.size(); i++ ) {
        git_log_browser->add(commits[i].oneline().c_str());
    }
    // Pick the first item
    if (commits.size() > 0) {
        git_log_browser->select(1);     // one based!
        git_log_browser->do_callback();
    }
}

// Update the filename browser with files from commit 'commit_hash'
void UpdateFilenameBrowser(const string& commit_hash, vector<Diff>& diffs)
{
    // Load diffs for this commit
    string errmsg;
    diffs.clear();      // clear any previous first
    if (LoadDiffs(commit_hash, diffs, errmsg) < 0) {
        fl_alert("ERROR: %s" , errmsg.c_str());
        exit(1);
    }

//DEBUG    printf("Loaded %ld diffs:\n", diffs.size());
//DEBUG    for (size_t t=0; t<diffs.size(); t++) {
//DEBUG        Diff &diff = diffs[t];
//DEBUG        for (size_t r=0; r<diff.diff_lines(); r++) {
//DEBUG            DiffLine &dl = diff.diff_line(r);
//DEBUG        }
//DEBUG    }

    // Load notes for this commit
    if (LoadCommitNotes(commit_hash, G_diffs, errmsg) < 0) {
        tty->printf(ANSI_RED "%s" ANSI_NOR "\n", errmsg.c_str());
        fl_alert("ERROR: can't load notes for commit %s: %s",
                 commit_hash.c_str(), errmsg.c_str());
    }
    // Update browser with filenames from loaded diffs[]
    filename_browser->clear();
    for (size_t i=0; i<diffs.size(); i++ ) {
        Diff &diff = diffs[i];
        string filename = string("@.") + diff.filename();
        filename_browser->add(filename.c_str(), (void*)&diff);  // filename browser's userdata() is Diff ptr
    }
    // Pick the first item
    if (diffs.size() > 0) {
        filename_browser->select(1);    // one based!
        filename_browser->do_callback();
    }
}

// Someone clicked on a new commit line
void GitLogBrowser_CB(Fl_Widget*, void*)
{
    int index          = git_log_browser->value();
    string commit_hash = (index > 0) ? git_log_browser->text(index) : "";
    //DEBUG cout << "log browser picked: '" << commit_hash << "'" << endl;
    // Parse commit hash
    int si = commit_hash.find(' ');
    if (si <= 0) return;        // nothing picked
    commit_hash.resize(si);
    // Update filename browser to show this commit
    UpdateFilenameBrowser(commit_hash, G_diffs);
    UpdateDiffsBrowser(G_diffs);
}

// Someone clicked on the filename browser
void FilenameBrowser_CB(Fl_Widget*, void*)
{
    int index = filename_browser->value();
    for (int t=1; t<=diffs_browser->size(); t++ ) {    // index 1 based..!
        DiffLine *dl = (DiffLine*)diffs_browser->data(t);
        if (!dl) { continue; }
        // Found line? Scroll diffs_browser to this line
        if (dl->diff_index() == (size_t)index-1) {
            diffs_browser->topline(t-1);            // -1: shows the "Filename" line
            return;
        }
    }
}

void YouCantEdit()
{
    fl_alert("You can't add notes to this..\n"
             "Pick one of the diff lines instead.");
}

// When user hits 'Save' in EditNotesDialog
void SaveNotes_CB(Fl_Widget *w, void *userdata)
{
    DiffLine *dlp = (DiffLine*)userdata;
    dlp->notes(G_editnotes->notes());       // get new notes
    G_editnotes->hide();                    // hide dialog
    // Save the new notes
    string errmsg;
    size_t diff_index = dlp->diff_index();
    if (G_diffs[diff_index].save_notes(dlp, errmsg) < 0) {
        fl_alert("ERROR: SaveNotes_CB(): Diff index %ld: %s", diff_index, errmsg.c_str());
    }
    UpdateDiffsBrowser(G_diffs);            // update changes
}

// Return "Edit Notes" dialog window's title
string GetEditNotesWindowTitle(DiffLine *dlp)
{
    stringstream ss;
    int diff_index = dlp->diff_index();
    ss << "File: " << G_diffs[diff_index].filename() << ", Line " << dlp->line_num();
    return ss.str();
}

void AddNotes_CB(Fl_Widget*, void *userdata)
{
    DiffLine *dlp = (DiffLine*)userdata;
    if (!dlp) return YouCantEdit();
    G_editnotes->title(GetEditNotesWindowTitle(dlp));     // set dialog window's title
    G_editnotes->notes(dlp->notes());                     // load editor with previous notes
    G_editnotes->save_callback(SaveNotes_CB, (void*)dlp); // save button cb
    G_editnotes->show();                                  // show dialog
}

// Someone left or right clicked a line in diffs_browser
void DiffsBrowser_CB(Fl_Widget *w, void *d)
{
    int index = diffs_browser->value();
    DiffLine *dl = (DiffLine*)diffs_browser->data(index);
//DEBUG    dl->show_self();

    // Double clicked item? Edit, done
    if (Fl::event_clicks() > 0)
        { AddNotes_CB(0, dl); return; }

    // Right clicked on item? Show context menu
    if (Fl::event_button() == 3) {
        // Get 'Diff' instance as userdata for item clicked on
        int index = diffs_browser->value();
        DiffLine *dlp = (DiffLine*)diffs_browser->data(index);
        if (!dlp) return YouCantEdit();
        // Dynamically create menu, pop it up
        Fl_Menu_Button menu(Fl::event_x(), Fl::event_y(), 80, 1);
        menu.add("Add\\/edit notes..", 0, AddNotes_CB, (void*)dlp);
        menu.popup();
    }
}

int main()
{
    // Create a custom color used for 'notes'
    Fl::set_color(16, 0xe0, 0xe6, 0xf2);        // Set Fl_Color(16) to light blue
    make_window("repo-notes");
    menubar->add("Help/Version: " VERSION " " GITVERSION);
    mainwin->end();
    mainwin->show();

    G_editnotes = new EditNotesDialog();

    // Configure diff_browser's popup menu
    diffs_browser->callback(DiffsBrowser_CB);

    // Configure callbacks
    git_log_browser->when(FL_WHEN_CHANGED);
    git_log_browser->callback(GitLogBrowser_CB);
    filename_browser->when(FL_WHEN_CHANGED);
    filename_browser->callback(FilenameBrowser_CB);

    // Load all commits for current project
    vector<Commit> commits;
    string errmsg;
    if (LoadCommits(commits, errmsg) < 0) {
        fl_alert("ERROR: %s" , errmsg.c_str());
        exit(1);
    }
    UpdateGitLogBrowser(commits);

    return Fl::run();
}
