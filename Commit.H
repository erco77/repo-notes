// vim: autoindent tabstop=8 shiftwidth=4 expandtab softtabstop=4

#ifndef COMMIT_H
#define COMMIT_H

#include <string>
#include <vector>

using namespace std;

class Commit {
    string commit_hash_;
    string comment_;
public:
    Commit()  { }
    ~Commit() { }

    // Entire commit as one line (hash + comment)
    string oneline(void) const  { string out = commit_hash_ + string(" ") + comment_; return out; }

    // Commit hash
    void   commit_hash(const char* val) { commit_hash_ = val; }
    void   commit_hash(string &val)     { commit_hash_ = val; }
    string commit_hash(void) const      { return commit_hash_; }

    // Commit comment
    void   comment(const char* val) { comment_ = val; }
    void   comment(string &val)     { comment_ = val; }
    string comment(void) const      { return comment_; }
};

int LoadCommits(vector<Commit>& commits, string& errmsg);

#endif
