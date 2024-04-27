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
    for (int i=0; i<(int)commits.size(); i++ ) {
	git_log_browser->add(commits[i].oneline().c_str());
    }
}

// Run 'git log' and put its output in the left browser
void UpdateFilenameBrowser(vector<Diff>& diffs)
{
    commit_files_browser->clear();
    for (int i=0; i<(int)diffs.size(); i++ ) {
	commit_files_browser->add(diffs[i].filename().c_str());
    }
}

int main()
{
    Fl_Double_Window *win = make_window();
    win->end();
    win->show();

    // Load all commits for current project
    vector<Commit> commits;
    string errmsg;
    if (LoadCommits(commits, errmsg) < 0) {
        fl_alert("ERROR: %s" , errmsg.c_str());
	exit(1);
    }
    UpdateGitLogBrowser(commits);

    // Load diffs for first commit (if any)
    vector<Diff> diffs;
    if (commits.size()) {
        string hash = commits[0].hash();
	if (LoadDiffs(hash, diffs, errmsg) < 0) {
	    fl_alert("ERROR: %s" , errmsg.c_str());
	    exit(1);
	}
	UpdateFilenameBrowser(diffs);
    }

    return Fl::run();
}
