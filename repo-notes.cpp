#include "MainWindow.H"
#include "Subs.H"
#include "Commit.H"
    if (LoadCommand_SUBS(string("git show -U10000 ") + hash, lines, errmsg) < 0) {
    string errmsg;
    if (LoadCommits(commits, errmsg) < 0) {
        fl_alert("ERROR: %s" , errmsg.c_str());
	exit(1);
    }