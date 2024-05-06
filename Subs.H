// vim: autoindent tabstop=8 shiftwidth=4 expandtab softtabstop=4

#ifndef SUBS_H
#define SUBS_H

#include <string>
#include <vector>

using namespace std;

// Fl_Terminal ANSI sequences to change colors of messages
#define ANSI_RED   "\033[31m"    // nor red
#define ANSI_GRN   "\033[32m"    // nor grn
#define ANSI_INFO  "\033[2;32m"  // dim grn
#define ANSI_INFO2 "\033[2;33m"  // dim yel
#define ANSI_NOR   "\033[0m"     // normal color

void StripCRLF_SUBS(char *s);
void StripLeadingWhite_SUBS(string& s);
void StringToLines_SUBS(string s, vector<string>& lines);
int  LoadCommand_SUBS(const string& command, vector<string>& lines, string& errmsg);
void Show_SUBS(vector<string>& lines);

bool   IsDir_SUBS(const string& dirname);
bool   IsFile_SUBS(const string& filename);
string RepoDirname_SUBS(void);
string CommitDirname_SUBS(const string& commit_hash, bool create);
string NotesFilename_SUBS(const string& commit_hash, int diff_index, int line_num, bool create);
int    DescendDir_SUBS(const string& dirname, vector<string>& files, vector<string>& warnings, string& errmsg);
int    DeleteFile_SUBS(const string& filename);

#endif
