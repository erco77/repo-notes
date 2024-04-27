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
#include "MainWindow.H"

// Back end
#include "Subs.H"	// subs to support this project
#include "Notes.H"	// our notes
#include "Commit.H"	// commits from git
#include "Diff.H"	// single commit diff

// Run 'git log' and put its output in the left browser
void UpdateGitLogBrowser(vector<Commit>& commits)
{
    git_log_browser->clear();
    for (size_t i=0; i<commits.size(); i++ ) {
	git_log_browser->add(commits[i].oneline().c_str());
    }
    // Pick the first item
    if (commits.size() > 0) {
        git_log_browser->select(1);	// one based!
	git_log_browser->do_callback();
    }
}

// Update the filename browser with files from commit 'hash'
void UpdateFilenameBrowser(string& hash)
{
    string errmsg;

    // Load diffs for first commit (if any)
    vector<Diff> diffs;
    if (LoadDiffs(hash, diffs, errmsg) < 0) {
	fl_alert("ERROR: %s" , errmsg.c_str());
	exit(1);
    }

    // Update browser with filenames from loaded diffs[]
    commit_files_browser->clear();
    for (size_t i=0; i<diffs.size(); i++ ) {
	commit_files_browser->add(diffs[i].filename().c_str());
    }
    // Pick the first item
    if (diffs.size() > 0) {
        commit_files_browser->select(1);	// one based!
        commit_files_browser->do_callback();
    }
}

// Someone clicked on a new commit line
void GitLogBrowser_CB(Fl_Widget*, void*)
{
    int index   = git_log_browser->value();
    string hash = (index > 0) ? git_log_browser->text(index) : "";
    cout << "log browser picked: '" << hash << "'" << endl;

    // Parse hash
    int si = hash.find(' ');
    if (si<=0) return;		// nothing picked
    hash.resize(si);
    UpdateFilenameBrowser(hash);
}

void CommitFilesBrowser_CB(Fl_Widget*, void*)
{
    int index = commit_files_browser->value();
    const char *s = index > 0 ? commit_files_browser->text(index)
                              : "";
    cout << "commit files browser picked: " << s << endl;
}

int main()
{
    Fl_Double_Window *win = make_window();
    win->end();
    win->show();

    // Configure callbacks
    git_log_browser->callback(GitLogBrowser_CB);
    commit_files_browser->callback(CommitFilesBrowser_CB);

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
