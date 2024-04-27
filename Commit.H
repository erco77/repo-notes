#ifndef COMMIT_H
#define COMMIT_H

#include <string>
#include <vector>

using namespace std;

class Commit {
    string hash_;
    string comment_;
public:
    Commit()  { }
    ~Commit() { }

    // Entire commit as one line (hash + comment)
    string oneline(void) const  { string out = hash_ + string(" ") + comment_; return out; }

    // Commit hash
    void   hash(const char* val) { hash_ = val; }
    void   hash(string &val)     { hash_ = val; }
    string hash(void) const      { return hash_; }

    // Commit comment
    void   comment(const char* val) { comment_ = val; }
    void   comment(string &val)     { comment_ = val; }
    string comment(void) const      { return comment_; }
};

int LoadCommits(vector<Commit>& commits, string& errmsg);

#endif
