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

using namespace std;


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

// Note
//    A single note for a specific commit/filename/offset
//
class Note {
    string commit_;             // commit associated with note
    string filename_;           // file note is assigned to
    int    offset_;             // offset into file's diff for note
    string note_;               // the multiline note

public:
    Note() {
        offset_ = -1;
    }
    void commit(const string& val)   { commit_   = val; }
    void filename(const string& val) { filename_ = val; }
    void offset(int val)             { offset_   = val; }
    void note(const string& val)     { note_     = val; }
};

// CommitNotes
//    All notes for a particular commit
//
class CommitNotes {
    string       commit_;          // commit hash for this array of notes
    vector<Note> notes_;           // all notes for this commit
public:
    CommitNotes() { }
    ~CommitNotes() { }
    void commit(const string& val) { commit_ = val; }
    void add_note(Note& note)      { notes_.push_back(note); }
    int  total_notes(void) const   { return notes_.size(); }
    Note& get_note(int index)      { return notes_[index]; }
};

// Is dirname a directory?
bool IsDir(const string& dirname) {
    struct stat buf;
    if (stat(dirname.c_str(), &buf) < 0) return false;
    if (! (buf.st_mode & S_IFDIR)) return false;
    return true;
}

// Return the ".repo-notes" directory name for the current project
string RepoDirname(void) {
    string dirname = ".repo-notes";        // XXX: hack; hunt in cwd and parents for .git
    if (!IsDir(dirname)) mkdir(dirname.c_str(), 0777);
    return dirname;
}

// Return the notes filename for a particular commit
string CommitFilename(const string& commit) {
    string filename = RepoDirname() + string("/commits");
    if (!IsDir(filename)) mkdir(filename.c_str(), 0777);
    filename += string("/") + commit;
    return filename;
}

// Strip out the trailing \n from string s
void StripCRLF(char *s) {
    s = strchr(s, '\n');
    if (s) *s = 0;
}

// Run 'command' and return its output in lines[]
// Returns:
//    0 on success
//   -1 on error (errmsg has reason)
//
int LoadCommand(const string& command,
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
        StripCRLF(s);
        lines.push_back(string(s));
    }
    pclose(fp);
    return 0;
}

int main() {
    cout << "aaaa = " << CommitFilename(std::string("aaaa")) << endl;
    cout << "bbbb = " << CommitFilename(std::string("bbbb")) << endl;

    string errmsg;
    vector<string> lines;
    if (LoadCommand(string("git log"), lines, errmsg) < 0) {
        cerr << "ERROR: " << errmsg << endl;
        exit(1);
    }
    for (int i=0; i<lines.size(); i++ ) cout << i << ": " << lines[i] << "\n";
    return 0;
}

/***
void LoadNotes(const std::string& commit) {
    std::string filename = NotesFilename(commit)
}


    Note note;
    note.commit(current_commit);
    note.filename(current_filename);
    note.offset(line_no);
    note.set_note(note_text);
*/
