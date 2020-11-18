CXX=g++
CXXFLAGS=-std=c++17 -Wall -Wpedantic# -Werror
DEBUG=-g
RELEASE=-O3
LIBS=-lcrypto -lssl

TMPDIR=tmp/
DOCDIR=doc/
BINDIR=

LOGIN=xmudry01
OUTPUT=isabot

TOKEN=

.PHONY: all build build-debug release debug doc archive tar targz zip clean-all clean

all: build

# Building the program
build: release
build-debug: debug

# Release building
release: ssl-release.o etc-release.o main-release.o
	#mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $(TMPDIR)ssl-release.o $(TMPDIR)etc-release.o $(TMPDIR)main-release.o $(LIBS) -o $(BINDIR)$(OUTPUT)

main-release.o: src/main.cpp
	mkdir -p $(TMPDIR)
	$(CXX) -c $(CXXFLAGS) $(RELEASE) src/main.cpp -o $(TMPDIR)main-release.o

etc-release.o: src/etc.cpp
	mkdir -p $(TMPDIR)
	$(CXX) -c $(CXXFLAGS) $(RELEASE) src/etc.cpp -o $(TMPDIR)etc-release.o

ssl-release.o: src/ssl.cpp
	mkdir -p $(TMPDIR)
	$(CXX) -c $(CXXFLAGS) $(RELEASE) src/ssl.cpp -o $(TMPDIR)ssl-release.o

# Debug building
debug: ssl-debug.o etc-debug.o main-debug.o
	#mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $(TMPDIR)ssl-debug.o $(TMPDIR)etc-debug.o $(TMPDIR)main-debug.o $(LIBS) -o $(BINDIR)$(OUTPUT)

main-debug.o: src/main.cpp
	mkdir -p $(TMPDIR)
	$(CXX) -c $(CXXFLAGS) $(DEBUG) src/main.cpp -o $(TMPDIR)main-debug.o

etc-debug.o: src/etc.cpp
	mkdir -p $(TMPDIR)
	$(CXX) -c $(CXXFLAGS) $(DEBUG) src/etc.cpp -o $(TMPDIR)etc-debug.o

ssl-debug.o: src/ssl.cpp
	mkdir -p $(TMPDIR)
	$(CXX) -c $(CXXFLAGS) $(DEBUG) src/ssl.cpp -o $(TMPDIR)ssl-debug.o

# Local documentation building
doc: $(DOCDIR)main.tex $(DOCDIR)manual.bib $(DOCDIR)czechiso.bst $(DOCDIR)isabot.png
	cd $(DOCDIR) && latexmk -pdf main.tex && latexmk -c
	mv $(DOCDIR)main.pdf manual_local.pdf

# Archive creation
archive: tar

tar:
	tar -cvf $(LOGIN).tar README Makefile manual.pdf src/main.cpp src/main.hpp src/ssl.cpp src/ssl.hpp src/etc.cpp src/etc.hpp

targz:
	tar -czvf $(LOGIN).tar.gz README Makefile manual.pdf src/main.cpp src/main.hpp src/ssl.cpp src/ssl.hpp src/etc.cpp src/etc.hpp

zip:
	zip -r $(LOGIN).zip README Makefile manual.pdf src/main.cpp src/main.hpp src/ssl.cpp src/ssl.hpp src/etc.cpp src/etc.hpp

# Cleaning
clean-all:
	rm -rf $(LOGIN).tar $(LOGIN).tar.gz $(LOGIN).zip bin $(TMPDIR) isabot manual_local.pdf

clean:
	rm -rf $(TMPDIR)
