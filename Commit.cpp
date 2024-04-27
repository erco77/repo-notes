// vim: autoindent tabstop=8 shiftwidth=4 expandtab softtabstop=4

#include "Subs.H"
#include "Commit.H"

// Load commits for the current project
// Returns:
//    0 on success, commits[] loaded
//   -1 on error (errmsg has reason)
//
int LoadCommits(vector<Commit>& commits, string& errmsg)
{
    // Load one line git log
    // Example:
    //  _____________________________________________
    // |5f50819 (HEAD -> master) Added notes file
    // |210e73d Initial commit: initial development
    //  ^^^^^^^ ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    //  hash    comment
    //
    vector<string> lines;
    if (LoadCommand_SUBS(string("git log --oneline"), lines, errmsg) < 0) {
        return -1;
    }
    char hash[100];
    char comment[1000];
    Commit commit;
    for (int i=0; i<(int)lines.size(); i++ ) {
        string line = lines[i] + "\n";          // need trailing "\n" for sscanf()
        const char *s = line.c_str();
        if (sscanf(s, "%99s %999[^\n]", hash, comment) == 2) {
            commit.hash(hash);
            commit.comment(comment);
            commits.push_back(commit);
        }
    }
    return 0;
}

