# repo-notes
A gitk-like GUI tool that lets one add comments to diffs without a dependence on cloud based tools (e.g. repos that need to be local, private), but without the complexity and external dependencies of large "peer review" tools.

This tool focuses /only/ on commenting diffs, and does not provide any fancy features larger peer review tools do, to keep the tool simple. It's good for small local/privately maintained projects where commenting diffs is important.

The tool was written in C++ and uses FLTK for the GUI, allowing it to be easily built on mac and linux. (It may work on Windows too, though the use of popen() to communicate with git may be troublesome on Windows, not sure). The code has no other external dependencies.

The application mainly supports diff commenting similar to github's own commenting tool, but for local/private git projects not intended to be managed in the cloud.

Here's a screenshot of the app being used to add notes to one of my own git projects that is local (not on github).
![repo-notes screenshot](https://github.com/erco77/repo-notes/blob/b45f33205ba9a92314111f36a52e8a3f2d4bdc21/images/screenshot.png)
In the above screenshot, the user added comment blocks are shown with light-blue background highlight.

- New comments can be created by double-clicking on a line in the diff, and the comment will be added below that line.
- Existing comments can be edited by double-clicking on the comment.

The comments are saved as simple ascii files in a .repo-notes/ directory (in the same directory as the .git directory), and are maintained separately from git. However, this directory could itself be added to git if need be so that the comments become part of the repo.

Currently the comments are associated with the commit hash and the line number of the 'git show <hash>'.

