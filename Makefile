run: repo-notes
	./repo-notes

repo-notes: repo-notes.C
	clang-10 repo-notes.C -lstdc++ -o repo-notes

clean:
	/bin/rm -f repo-notes 2> /dev/null
