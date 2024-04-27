#include <sys/stat.h>   // mkdir(), etc
#include <sys/types.h>
#include <stdio.h>      // popen
#include <string.h>     // strchr
#include <unistd.h>
#include <fcntl.h>
#include <iostream>

#include "Subs.H"

// Strip out the trailing \n from string s
void StripCRLF_SUBS(char *s) {
    s = strchr(s, '\n');
    if (s) *s = 0;
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
