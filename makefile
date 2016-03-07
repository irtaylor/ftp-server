# Ian Taylor
# CS 372 -- PROJECT 2
# makefile: a basic build script for the ftpserver program.



main: ftpserver.cpp
	g++ -std=c++0x ftpserver.cpp -o ftpserver

make clean:
	rm ftpserver server
