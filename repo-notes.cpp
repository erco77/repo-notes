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

#include <FL/Fl.H>
#include <FL/fl_message.H>
#include "MainWindow.H"
#include "Subs.H"
#include "Commit.H"

// Each multiline note is managed in a <commit>/notes file
//
//      ~/.reponotes/preferences                        - gui prefs
//      ~/.reponotes/commits/<commit>/notes             - all notes for this commit
//
// ----
// Widget:
//
//    Commits                   File                     Diffs/comments
//   .------------------------..-----------------------..--------------------------------------------.
//   : 0dfaa12 Recent commit  :: somewhere/Makefile    :: somewhere/Makefile                         :  <-- file header
//   : ff77abc older commit   :: somewhere/foo.c       ::............................................:
//   :                        :: somewhere/bar.c       :: --- -OLD 0001                              :
//   :                        ::                       :: --- -OLD 0002                              :
//   :                        ::                       :: --- -OLD 0003                              :
//   :                        ::                       :: 001 +NEW aaaa                              :
//   :                        ::                       :: 002 +NEW bbbb                              :
//   :                        ::                       ::============================================:
//   :                        ::                       ::     ^  This mod here affects ABC and       :
//   :                        ::                       ::        is later reverted in commit XYZ     :
//   :                        ::                       ::============================================:
//   :                        ::                       :: 003 +NEW cccc                              :
//   :                        ::                       :: 004 context a                              :
//   :                        ::                       :: 005 context b                              :
//   :                        ::                       :: 006                                        :
//   `------------------------``-----------------------``--------------------------------------------`
//
// Fitting notes:
//     As diff (unlimited context) is added to Diffs/comments widget, it looks matching lines#
//     from the Notes data, and begins an insert of the text for that note (between the ==='s above)
//     which can be edited via right click menu.
//     Also, a highlighted line in that window can have a right click menu to add new notes.
// 

// Run 'git log' and put its output in the left browser
void UpdateGitLogBrowser(vector<Commit>& commits)
{
    git_log_browser->clear();
    for (int i=0; i<(int)commits.size(); i++ ) {
	git_log_browser->add(commits[i].oneline().c_str());
    }
}

class Diff {
    string filename_;		// filename for this diff
    vector<string> diff_lines_;	// lines of diff
    // TODO: Add vector<Note> here too? May be several for each diff
public:
    Diff()  { }
    ~Diff() { }

    // Set diff filename
    void filename(const char* val) { filename_ = val; }
    void filename(string& val)     { filename_ = val; }
    string filename() const        { return filename_; }

    // Add a diff line
    void add(const char* val)  { diff_lines_.push_back(string(val)); }
    void add(string &val)      { diff_lines_.push_back(val); }

    // Clear class
    void clear(void) { filename(""); diff_lines_.clear(); }
};

// Run 'git log' and put its output in the left browser
void UpdateFilenameBrowser(vector<Diff>& diffs)
{
    commit_files_browser->clear();
    for (int i=0; i<(int)diffs.size(); i++ ) {
	commit_files_browser->add(diffs[i].filename().c_str());
    }
}

void LoadDiffs(string& hash, vector<Diff> &diffs)
{
    // Load all diffs for this commit
    vector<string> lines;
    string errmsg;
    cout << "Loading diffs from hash " << hash << ": ";
    if (LoadCommand_SUBS(string("git show -U10000 ") + hash, lines, errmsg) < 0) {
        fl_alert("ERROR: %s" , errmsg.c_str());
        exit(1);
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
    //    |diff --git a/Makefile b/Makefile		<-- START EACH NEW DIFF HERE
    //    |new file mode 100644  ^^^^^^^^^^ --> We want this as the filename of the diff, without the "b/"
    //    |index 0000000..e692784
    //    |--- /dev/null
    //    |+++ b/Makefile		                <-- WE NEED THESE FOR filenames[], without the "b/"
    //    |@@ -0,0 +1,8 @@
    //    |+run: repo-notes         
    //    |+       ./repo-notes
    //    |..
    //
    Diff diff;
    char diff_filename[512];
    for (int i=0; i<(int)lines.size(); i++) {
        const char *s = lines[i].c_str();
	cout << "Working on: " << s << endl;
	// New diff?
	if (sscanf(s, "diff --git %*s %511s", diff_filename) == 1) {
	    cout << "--- DIFF FILENAME: " << diff_filename << endl;
	    // Save previous (if any)
	    if (diff.filename() != "") {
	        diffs.push_back(diff);
		diff.clear();
	    }
	    // Set filename, skip leading "b/" prefix, if any
	    if (strncmp(diff_filename, "b/", 2)==0) diff.filename(diff_filename+2);
	    else                                    diff.filename(diff_filename);
	}
	// Add all lines to diff (only if already working on one)
	if (diff.filename() != "") {
	    diff.add(lines[i]);
	}
    }
    // Append last diff
    if (diff.filename() != "") {
        diffs.push_back(diff);
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
	LoadDiffs(hash, diffs);
	UpdateFilenameBrowser(diffs);
    }

    return Fl::run();
}
