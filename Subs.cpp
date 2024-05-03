// vim: autoindent tabstop=8 shiftwidth=4 expandtab softtabstop=4

#include <sys/stat.h>   // mkdir(), etc
#include <sys/types.h>
#include <stdio.h>      // popen
#include <string.h>     // strchr
#include <unistd.h>
#include <fcntl.h>

#include <iostream>
#include <string>
#include <sstream>      // stringstream

#include "Subs.H"

// Strip out the trailing \n from string s
void StripCRLF_SUBS(char *s) {
    s = strchr(s, '\n');
    if (s) *s = 0;
}

// Convert a multiline string into vector of separate lines
void StringToLines_SUBS(string s, vector<string>& lines)
{
    size_t i = 0;
    string line;
    //DEBUG printf("--- IN '%s'\n", s.c_str());
    while (1) {
        i = s.find('\n');                       // find index of \n
        //DEBUG printf("    find() returned %ld\n", i);
        if (i == string::npos) break;           // none? done
        line = s;                               // copy line so we can truncate
        line.resize(i);                         // truncate at \n
        //DEBUG printf("OUT: '%s'\n", line.c_str());
	lines.push_back(line);
        s = s.substr(i+1, s.size()-(i+1));      // move to next line
    }
    //DEBUG printf("OUT: '%s'\n", s.c_str());
    lines.push_back(s);
}

// Is dirname a directory?
bool IsDir_SUBS(const string& dirname) {
    struct stat buf;
    if (stat(dirname.c_str(), &buf) < 0) return false;
    if (! (buf.st_mode & S_IFDIR)) return false;
    return true;
}

// Return the ".repo-notes" directory name for the current project
string RepoDirname_SUBS(bool create) {
    string dirname = ".repo-notes";        // TODO: hunt cwd (and up) for .git dir, and put it there
    if (create && !IsDir_SUBS(dirname)) mkdir(dirname.c_str(), 0777);
    return dirname;
}

// Return the notes filename for a particular commit
string CommitDirname_SUBS(const string& hash, bool create) {
    // Create commits dir
    string dirname = RepoDirname_SUBS(create) + string("/commits");
    if (create && !IsDir_SUBS(dirname)) { mkdir(dirname.c_str(), 0777); }
    // Create commit hash dir
    dirname += string("/") + hash;
    if (create && !IsDir_SUBS(dirname)) { mkdir(dirname.c_str(), 0777); }
    // Return path to hash dir
    return dirname;
}

// Return the "notes" filename, e.g. .repo-notes/<hash>/notes-<diff_index#>-<line#>.
//
//     We use index#s in place of the diff file's filename, because it may
//     may contain slashes (a directory hierarchy), and there's no reliable way
//     to flatten that "safely". For instance, trivially replacing '/' with '_'
//     can create a name collision with two actual file names, e.g.
//
//             somedir/file       -- somedir directory contains "file"
//             somedir_file       -- top level dir contains a file named "somedir_file"
//
//     ..if we trivially replace '/' with '_' in all filenames, the above two filenames
//     both end up being "somedir_file", even though they're separate.
//            
string NotesFilename_SUBS(const string& hash,
                          int   diff_index,      // what we use for <difffile_index#>
                          int   line_num,        // what we use as the <line#>
                          bool  create) {        // true: create commit dir, false: don't create dir
    stringstream ss;
    ss << CommitDirname_SUBS(hash, create)       // "some/path/.repo-notes/<commit>"
       << "/notes-" << diff_index                // "some/path/.repo-notes/<commit>/notes-12"
       << "-"       << line_num                  // "some/path/.repo-notes/<commit>/notes-12-3"
       << ".txt";                                // "some/path/.repo-notes/<commit>/notes-12-3.txt"
    return ss.str();
}

// Run 'command' and return its output in lines[]
// Returns:
//    0 on success
//   -1 on error (errmsg has reason)
//
int LoadCommand_SUBS(const string& command,
                vector<string>& lines,
                string& errmsg)
{
    char s[2048];
    FILE *fp = popen(command.c_str(), "r");
    if (!fp) {
        errmsg += string("Command '") + command + string("': ") + string(strerror(errno));
        return -1;
    }
    while (fgets(s, sizeof(s)-1, fp)) {
        StripCRLF_SUBS(s);
        lines.push_back(string(s));
    }
    pclose(fp);
    return 0;
}

void Show_SUBS(vector<string>& lines)
{
    for (int i=0; i<(int)lines.size(); i++ )
        cout << i << ": " << lines[i] << "\n";
}
