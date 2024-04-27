FLTKDIR=/usr/local/src/fltk-1.4.x.git/
FLTKCONFIG=$(FLTKDIR)/build/fltk-config
FLUID=$(FLTKDIR)/build/bin/fluid

CXX = $(shell $(FLTKCONFIG) --cxx)
CXXFLAGS = $(shell $(FLTKCONFIG) --cxxflags) -Wall -O3 -I/other/include/paths...
LINKFLTK_IMG = $(shell $(FLTKCONFIG) --use-images --ldstaticflags)

run: repo-notes
	./repo-notes

# We use C++ file extensions ".cpp" and ".H" in this project
MainWindow.cpp: MainWindow.fl
	$(FLUID) -o MainWindow.cpp -h MainWindow.H -c MainWindow.fl

MainWindow.o: MainWindow.cpp MainWindow.H
	$(CXX) $(CXXFLAGS) MainWindow.cpp -c

Subs.o: Subs.cpp Subs.H
	$(CXX) $(CXXFLAGS) Subs.cpp -c

Commit.o: Commit.cpp Commit.H
	$(CXX) $(CXXFLAGS) Commit.cpp -c

repo-notes: repo-notes.cpp MainWindow.o Commit.o Subs.o
	$(CXX) $(CXXFLAGS) \
	    repo-notes.cpp \
	    Subs.o \
	    Commit.o \
	    MainWindow.o \
	    $(LINKFLTK_IMG) -o repo-notes

clean:
	/bin/rm -f repo-notes MainWindow.o   2> /dev/null
	/bin/rm -f MainWindow.cpp MainWindow.H 2> /dev/null
