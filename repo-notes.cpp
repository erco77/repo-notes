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

// Back end
#include "Subs.H"       // subs to support this project
#include "Notes.H"      // our notes
#include "Commit.H"     // commits from git
#include "Diff.H"       // single commit diff

// Globals
vector<Diff> G_diffs;   // all diffs for current project
                        // TODO: notes should be in here too

void UpdateDiffsBrowser(vector<Diff>& diffs)
{
    diffs_browser->clear();

    // For each diff, add FILENAME and DIFF LINES to browser..
    for (size_t di=0; di<diffs.size(); di++ ) { // diff index
        Diff& diff = diffs[di];

        // Add diff filename
        string filename;
        filename = string("@B47" "@b" "@l" "@.") + diff.filename();
        diffs_browser->add(filename.c_str());

        // Add diff lines
        for (size_t li=0; li<diff.lines(); li++) {  // line index
            string line = diff.line(li);
            switch (line[0]) {
                case '+': line = string("@C60@.") + line; break;
                case '-': line = string("@C1@." ) + line; break;
                default:  line = string("@."    ) + line; break;
            }
            diffs_browser->add(line.c_str(), (void*)&diff);
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

void AddNotes_CB(Fl_Widget *w, void *userdata)
{
    Diff *diff = (Diff*)userdata;
    if (!diff) {
        fl_alert("ERROR: AddNotes: userdata() is NULL");
        return;
    }
    fl_message("Add notes goes here..");
}

// Handle posting right-click menu over diffs_browser
void DiffsBrowserPopupMenu_CB(Fl_Widget *w, void *userdata)
{
    // Only interested in right clicks
    if (Fl::event_button() != 3) return;

    // Get 'Diff' instance as userdata for item clicked on
    int index = diffs_browser->value();
    Diff *diff = (Diff*)diffs_browser->data(index);
    if (!diff) {
        // must have a diff or we can do nothing
        fl_alert("ERROR: diffs_browser index %d: userdata() is NULL", index);
        return;
    }

    // Dynamically create menu, pop it up
    Fl_Menu_Button menu(Fl::event_x(), Fl::event_y(), 80, 1);
    //menu.add(diff->filename().c_str(), 0, 0, 0, FL_MENU_INACTIVE|FL_MENU_DIVIDER);
    menu.add("Add\\/edit notes..", 0, AddNotes_CB, (void*)diff);
    menu.popup();
}

int main()
{
    Fl_Double_Window *win = make_window("repo-notes");
    win->end();
    win->show();
    win->resizable(tile);

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
