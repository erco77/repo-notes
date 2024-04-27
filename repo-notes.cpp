#include "Diff.H"	// single commit diff
    for (size_t i=0; i<commits.size(); i++ ) {
    // Pick the first item
    if (commits.size() > 0) {
        git_log_browser->select(1);	// one based!
	git_log_browser->do_callback();
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
    filename_browser->clear();
    for (size_t i=0; i<diffs.size(); i++ ) {
	filename_browser->add(diffs[i].filename().c_str());
    }
    // Pick the first item
    if (diffs.size() > 0) {
        filename_browser->select(1);	// one based!
        filename_browser->do_callback();
    }
}
// Someone clicked on a new commit line
void GitLogBrowser_CB(Fl_Widget*, void*)
    int index   = git_log_browser->value();
    string hash = (index > 0) ? git_log_browser->text(index) : "";
    cout << "log browser picked: '" << hash << "'" << endl;

    // Parse hash
    int si = hash.find(' ');
    if (si<=0) return;		// nothing picked
    hash.resize(si);
    UpdateFilenameBrowser(hash);
// Someone clicked on the filename browser
void FilenameBrowser_CB(Fl_Widget*, void*)
    int index = filename_browser->value();
    const char *s = (index > 0) ? filename_browser->text(index) : "";
    cout << "commit files browser picked: '" << s << "'" << endl;
    // Configure callbacks
    git_log_browser->when(FL_WHEN_CHANGED);
    git_log_browser->callback(GitLogBrowser_CB);
    filename_browser->when(FL_WHEN_CHANGED);
    filename_browser->callback(FilenameBrowser_CB);
