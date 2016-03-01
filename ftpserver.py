#!/usr/bin/env python


import sys
import socket
import signal
import os


def main():
    # validate command-line args: program takes a hostname and a port #
    if len(sys.argv) != 2:
        sys.stderr.write("Usage: ftpserver.py <port #>\n")
        exit(1)

    # install interrupt handler to catch Ctrl+C
    signal.signal(signal.SIGINT, signal_handler)


    # set the var port. port must be converted to an integer, since it is stored in sys.argv as a string
    port = int(sys.argv[1])


    server_socket = create_socket(port)

    ftp_session(port, server_socket)

    server_socket.close


def signal_handler(singal, frame):
    print " Shutting down. Goodbye!"
    sys.exit(0)

def create_socket(port):
    host = socket.gethostname()
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind((host, port))
    server_socket.listen(5)

    print (socket.gethostname() + " now listening on port #"), port

    return server_socket


def ftp_session(port, server_socket):

    while True:
        command_socket, addr = server_socket.accept()
        print "Now connected with ", addr

        get_command(command_socket)

        command_socket.close()


def get_command(command_socket):

    msg_send = ""

    command_msg = command_socket.recv(1024)
    print "Command Received: " + command_msg

    if (command_msg != "-l") and (command_msg != "-g"):
       print "ERROR: Received invalid command.\n"
       msg_send = "ERROR: Received invalid command.\nAccepted Commands: < -l | -g >\n"

    elif (command_msg == "-l"):
           msg_send = list_files()


    command_socket.send(msg_send)


def list_files():
    file_structure = ""

    files = os.listdir('.')
    for f in files:
        file_structure += f + "\n"

    return file_structure


# Enables modularization of code
if __name__ == '__main__':
    main()
