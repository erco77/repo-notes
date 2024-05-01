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

// Globals
vector<Diff> G_diffs;   // all diffs for current project
                        // TODO: notes should be in here too

EditNotesDialog *G_editnotes = 0;

void UpdateDiffsBrowser(vector<Diff>& diffs)
{
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
        const char *grn_fmt = "@C60@.";      // green foreground text
        const char *red_fmt = "@C1@.";       // red foreground text
        const char *nor_fmt = "@.";          // normal text (no color)
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
        }
    }
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

// Update the filename browser with files from commit 'hash'
void UpdateFilenameBrowser(string& hash, vector<Diff>& diffs)
{
    string errmsg;

    // Load diffs for first commit (if any)
    diffs.clear();      // clear any previous first
    if (LoadDiffs(hash, diffs, errmsg) < 0) {
        fl_alert("ERROR: %s" , errmsg.c_str());
        exit(1);
    }

    // Update browser with filenames from loaded diffs[]
    filename_browser->clear();
    for (size_t i=0; i<diffs.size(); i++ ) {
        filename_browser->add(diffs[i].filename().c_str());
    }
    // Pick the first item
    if (diffs.size() > 0) {
        filename_browser->select(1);    // one based!
        filename_browser->do_callback();
    }

    // TODO: Add vector<Note> to Diff, and load any notes files found here
    //       See the README.txt for design document.
}

// Someone clicked on a new commit line
void GitLogBrowser_CB(Fl_Widget*, void*)
{
    int index   = git_log_browser->value();
    string hash = (index > 0) ? git_log_browser->text(index) : "";
    //DEBUG cout << "log browser picked: '" << hash << "'" << endl;

    // Parse hash
    int si = hash.find(' ');
    if (si<=0) return;          // nothing picked
    hash.resize(si);

    UpdateFilenameBrowser(hash, G_diffs);
    UpdateDiffsBrowser(G_diffs);
}

// Someone clicked on the filename browser
void FilenameBrowser_CB(Fl_Widget*, void*)
{
    int index = filename_browser->value();
    const char *s = (index > 0) ? filename_browser->text(index) : "";
    cout << "commit files browser picked: '" << s << "'" << endl;
}

void YouCantEdit()
{
    fl_alert("You can't add/edit the filename header lines.\n"
             "(Pick one of the diff lines instead)");
}

// When user hits 'Save' in EditNotesDialog
void SaveNotes_CB(Fl_Widget *w, void *userdata)
{
    DiffLine *dlp = (DiffLine*)userdata;
    dlp->notes(G_editnotes->notes());       // get new notes
    G_editnotes->hide();                    // hide dialog
}

void AddNotes_CB(Fl_Widget *w, void *userdata)
{
    DiffLine *dlp = (DiffLine*)userdata;
    if (!dlp) return YouCantEdit(); 
    G_editnotes->title(dlp->filename());                  // set dialog window's title
    G_editnotes->notes(dlp->notes());                     // load editor with previous notes
    G_editnotes->save_callback(SaveNotes_CB, (void*)dlp); // save button cb
    G_editnotes->show();                                  // show dialog
}

// Handle posting right-click menu over diffs_browser
void DiffsBrowserPopupMenu_CB(Fl_Widget *w, void *userdata)
{
    // Only interested in right clicks
    if (Fl::event_button() != 3) return;

    // Get 'Diff' instance as userdata for item clicked on
    int index = diffs_browser->value();
    DiffLine *dlp = (DiffLine*)diffs_browser->data(index);
    if (!dlp) return YouCantEdit(); 

    // Dynamically create menu, pop it up
    Fl_Menu_Button menu(Fl::event_x(), Fl::event_y(), 80, 1);
    menu.add("Add\\/edit notes..", 0, AddNotes_CB, (void*)dlp);
    menu.popup();
}

int main()
{
    Fl_Double_Window *win = make_window("repo-notes");
    win->end();
    win->show();
    win->resizable(tile);

    G_editnotes = new EditNotesDialog();

    // Configure diff_browser's popup menu
    diffs_browser->callback(DiffsBrowserPopupMenu_CB);

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
