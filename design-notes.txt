Each multiline note is managed in a <commit>/notes file

     ~/.reponotes/preferences                        - gui prefs
     ~/.reponotes/commits/<commit>/notes             - all notes for this commit

----
Widget:

   Commits                   File                     Diffs/comments
  .------------------------..-----------------------..--------------------------------------------.
  : 0dfaa12 Recent commit  :: somewhere/Makefile    :: somewhere/Makefile                         :  <-- file header
  : ff77abc older commit   :: somewhere/foo.c       ::............................................:
  :                        :: somewhere/bar.c       :: --- -OLD 0001                              :
  :                        ::                       :: --- -OLD 0002                              :
  :                        ::                       :: --- -OLD 0003                              :
  :                        ::                       :: 001 +NEW aaaa                              :
  :                        ::                       :: 002 +NEW bbbb                              :
  :                        ::                       ::============================================:
  :   [commits_browser]    ::   [file_browser]      ::     ^  This mod here affects ABC and       :
  :                        ::                       ::        is later reverted in commit XYZ     :
  :                        ::                       ::============================================:
  :                        ::                       :: 003 +NEW cccc                              :
  :                        ::                       :: 004 context a    [diffs_browser]           :
  :                        ::                       :: 005 context b                              :
  :                        ::                       :: 006                                        :
  :-----------------------------------------------------------------------------------------------: <-- Edge "E"
  : status tty                         [tty]                                                      :
  :_______________________________________________________________________________________________:

 NOTE: To get the [tty] widget to keep its vertical size during window resizing 
 (so that the other widgets enlarge in height instead of the tty, unless the tty area itself
 is enlarged by moving edge "E"), an "invisible child box" is set to be the resizable() and is
 positioned roughly where the [diffs_browser] is. So:

     win->resizable(tile);		// window's resizable is the tile
     tile->resizable(invisible_box);	// tile's resizable is the invisible box

Fitting notes:
    As diff (unlimited context) is added to Diffs/comments widget, it looks matching lines#
    from the Notes data, and begins an insert of the text for that note (between the ==='s above)
    which can be edited via right click menu.
    Also, a highlighted line in that window can have a right click menu to add new notes.


