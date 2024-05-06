// vim: autoindent tabstop=8 shiftwidth=4 expandtab softtabstop=4

#include <sys/stat.h>   // mkdir(), etc
#include <sys/types.h>
#include <sys/stat.h>   // stat()
#include <sys/dir.h>    // opendir()
#include <stdio.h>      // popen
#include <unistd.h>     // unlink
#include <string.h>     // strchr
#include <fcntl.h>

#include <iostream>
#include <string>
#include <sstream>      // stringstream
#include <regex>        // regex_replace.. (note: this really slows down g++!)

#include "Subs.H"
#include "MainWindow.H" // tty->printf()..

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

// Is filename an existing "regular file"?
bool IsFile_SUBS(const string& filename) {
    struct stat buf;
    if (stat(filename.c_str(), &buf) < 0) return false;
    if (! (buf.st_mode & S_IFREG)) return false;
    return true;
}

// Return the ".repo-notes" directory name for the current project
string RepoDirname_SUBS(bool create) {
    string dirname = ".repo-notes";        // TODO: hunt cwd (and up) for .git dir, and put it there
    if (create && !IsDir_SUBS(dirname)) mkdir(dirname.c_str(), 0777);
    return dirname;
}

// Return the notes dirname for a particular commit
string CommitDirname_SUBS(const string& commit_hash, bool create) {
    // Create commits dir
    string dirname = RepoDirname_SUBS(create) + string("/commits");
    if (create && !IsDir_SUBS(dirname)) { mkdir(dirname.c_str(), 0777); }
    // Create commit hash dir
    dirname += string("/") + commit_hash;
    if (create && !IsDir_SUBS(dirname)) { mkdir(dirname.c_str(), 0777); }
    // Return path to commit hash dir
    return dirname;
}

// Return the "notes" filename, e.g. .repo-notes/<commit_hash>/notes-<diff_index#>-<line#>.
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
string NotesFilename_SUBS(const string& commit_hash,
                          int   diff_index,       // what we use for <difffile_index#>
                          int   line_num,         // what we use as the <line#>
                          bool  create) {         // true: create commit dir, false: don't create dir
    stringstream ss;
    ss << CommitDirname_SUBS(commit_hash, create) // "some/path/.repo-notes/<commit>"
       << "/notes-" << diff_index                 // "some/path/.repo-notes/<commit>/notes-12"
       << "-"       << line_num                   // "some/path/.repo-notes/<commit>/notes-12-3"
       << ".txt";                                 // "some/path/.repo-notes/<commit>/notes-12-3.txt"
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
    tty->printf(ANSI_INFO "Executing: %s\n" ANSI_NOR, command.c_str());
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

// Descend a directory, accumulate all filenames found in files[]
//     Returns 0 on success, with found files in files[], and any warnings in warnings[]
//     On error returns -1 (errmsg has reason)
//
int DescendDir_SUBS(const string& dirname,      // dirname to descend
                    vector<string>& files,      // returned filenames found
                    vector<string>& warnings,   // warnings (if any) encountered
                    string& errmsg)             // errmsg if return is -1
{
    DIR *dirp;
    struct stat buf;
    struct direct *dp;
    string pathname;
    // Open dir
    if ((dirp = opendir(dirname.c_str()))==NULL) {
        errmsg = string("opendir(") + dirname
               + string("): ") + string(strerror(errno));
        return -1;
    }
    // Read all dir entries in this dir
    while ((dp = readdir(dirp)) != NULL) {
	// Skip . and .. to avoid endless recurse
	if (strcmp(dp->d_name,"." )==0) continue;
	if (strcmp(dp->d_name,"..")==0) continue;
        // Build pathname for each dir entry encountered
        pathname = dirname + "/" + string(dp->d_name);
        // Stat entry to see what it is
	if (stat(pathname.c_str(), &buf)==EOF) {
            string warning = string("stat(") + pathname + "): " + string(strerror(errno));
            warnings.push_back(warning);
	} else {
	    if (buf.st_mode & S_IFDIR) {
                // Dir? Recurse into it
                //DEBUG printf("FOUND SUBDIR: %s\n", pathname.c_str());
                if ( DescendDir_SUBS(pathname.c_str(), files, warnings, errmsg) < 0) {
                    // Close dir and return up the recursion hierarchy
                    closedir(dirp);
                    return -1;
                }
            } else {
                // Not a dir? It's a file, add to files[]
                //DEBUG tty->printf("FOUND FILE: %s\n", pathname.c_str());
                files.push_back(pathname);
            }
        }
    }
    // Clean up, done
    if (closedir(dirp) < 0) {
        errmsg = string("closedir(") + dirname
               + string("): ") + string(strerror(errno));
        return -1;
    }
    return 0;
}

// Remove leading whitespace from string 's'
void StripLeadingWhite_SUBS(string& s)
{
    s = regex_replace(s, regex("^[ \t]*"), "");
}

int DeleteFile_SUBS(const string& filename)
{
    tty->printf(ANSI_INFO "removing empty notes file: %s" ANSI_NOR, filename.c_str());
    return unlink(filename.c_str());
}
