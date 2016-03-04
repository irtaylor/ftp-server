#!/usr/bin/env python

"""
    Ian Taylor
    CS 372 -- PROJECT 2
    ftpclient: a simple ftpclient in python.
"""

"""

"""

"""
    NOTE: my primary references for the client were http://www.linuxhowtos.org/C_C++/socket.htm and https://docs.python.org/2/library/socket.html.
"""


""" IMPORTS """
import sys
import socket
import signal
"""         """


def main():

    validate_args()

    # install interrupt handler to catch Ctrl+C
    # NOTE: USING SIGINT ON THE CLIENT WILL CAUSE ERRORS.
    # PROGRAMS SHOULD BE TERMINATED BY TYPING "\quit".
    signal.signal(signal.SIGINT, signal_handler)


    remote_host = sys.argv[1]
    command_port = int(sys.argv[2])
    command_msg = sys.argv[3]
    file_name = ""
    data_port = 0

    if len(sys.argv) == 5:
        data_port = int(sys.argv[4])
    elif len(sys.argv) == 6:
        file_name = sys.argv[4]
        data_port = int(sys.argv[5])


    # create a connection between the client and specified host on a given port
    command_socket = create_command_connection(remote_host, command_port)

    command_socket.send(command_msg)

    if command_msg == "-g":
        command_socket.send(file_name)

    msg_recv = command_socket.recv(1024)
    print "Server: " + msg_recv

    command_socket.close()




"""
    FUNCTION:   establish_command_connection()
    receives:   the remote_host and port to connect to for sending ftp commands.
    returns:    the active command_socket.
    purpose:    create a command_socket and establish the connection w/ the server.
"""
def create_command_connection(remote_host, command_port):
    # initialize the client socket
    command_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    command_socket.connect((remote_host, command_port))
    print 'Now connected to ', remote_host, 'on command_port #', command_port

    return command_socket


"""
    FUNCTION:   signal_handler()
    receives:   the signal to be handled and current stack frame.
    returns:    nothin.
    purpose:    handle SIGINT and shutdown "gracefully".
"""
def signal_handler(singal, frame):
    print " Shutting down. Goodbye!"
    sys.exit(0)



"""
    FUNCTION:   validate_args()
    receives:   nothin.
    returns:    nothin.
    purpose:    validate command line args. display correct error messages for all cases of invalid user input.
"""
def validate_args():

    usage = "Usage: ftpclient.py <server_host> <server_port #> < -l | -g file_name> <data_port #>\n"

    # validate command-line args: program takes a remote_host, command_port # to send commands, a command option (either list directory contents or get a file w/ file_name specified), and a data_port # for the file transfer

    # Case 1: incorrect number of args
    if len(sys.argv) not in (5, 6):
        sys.stderr.write(usage)
        exit(1)

    # Case 2: user enters < -g > but omits <file_name>
    elif (len(sys.argv) == 5) and (sys.argv[3] == "-g"):
        sys.stderr.write("Invalid Command: Please specify a filename.\n")
        sys.stderr.write(usage)
        exit(1)

    # Case 3: user enters < -l > and specified superfluous <file_name>
    elif (len(sys.argv) == 6) and (sys.argv[3] == "-l"):
        sys.stderr.write("Invalid Command: Please omit filename when using < -l >.\n")
        sys.stderr.write(usage)
        exit(1)


# Enables modularization of code by defining the entry point for the program.
if __name__ == '__main__':
    main()
