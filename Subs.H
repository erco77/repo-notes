#ifndef SUBS_H
#define SUBS_H

#include <string>
#include <vector>

using namespace std;

void StripCRLF_SUBS(char *s);
int LoadCommand_SUBS(const string& command, vector<string>& lines, string& errmsg);
void Show_SUBS(vector<string>& lines);

#endif
