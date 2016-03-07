# ftp-server
CS 372: Intro to Networking, Project 2, FTP
Ian Taylor
taylori@onid.oregonstate.edu

Instructions:
    1) Enter 'make' to compile ftpserver.cpp
    2) Run as specified in the Project Guide lines.
        e.g. './ftpserver 40050' and 'python ftpclient.py flip2.oregonstate.edu 40050 -l 40051'

Notes:
    I really struggled with this assignment. I've tested it on both my home Macbook air and the flip server.
    Unfortunately, it runs as required on my home machine, but not on the flip server, although I have been
    able to cover most of the functionality in the assignment. The following are known issues that I could not fix:

    1) the client is unable connect to 'localhost'. the name of the flip server must be EXPLICITLY entered (e.g. 'flip2.oregonstate.edu').

    2) return the directory is highly inconsistent -- the program seems to randomly add extra, non-existent files to the directory, resulting in a seg fault on the server program's side.

    3) when issuing subsequent commands from the client, it is preferable to change the data_port. Attempting to connect too quickly on the same data port will likely cause the server and client to crash.

    Please note again that none of these issues occured on my personal computer. Consequently, I have been unable to successfully debug these issues. Nevertheless, my project CAN fulfill all the requirements; it might just be a little buggy in doing so.

    Thanks!
