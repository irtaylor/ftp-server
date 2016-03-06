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
import signal
from socket import (            # Sockets API
    socket,
    gethostname,
    gethostbyname,
    AF_INET,
    SOCK_STREAM,
    SOL_SOCKET,
    SO_REUSEADDR
)

from struct import pack, unpack # Structured binary data
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

    print

    # create a command connection between the client and specified host on a given port.
    # use to send / receive commands and messages.
    command_socket = create_connection(remote_host, command_port)
    print 'Command socket connected to', remote_host, 'on port #', command_port, '...'


    """
            Need to send and receive messages in useful order.
            I'm opting to send them as follows:
                1) Send the basic command "-l" or "-g"
                2) if "-g", send the name of the file to transfer
                3) send the data port that the client will use for all data transfers
    """

    _send_msg(command_msg, command_socket)

    if command_msg == "-g":
        _send_msg(file_name, command_socket)

    _send_msg(str(data_port), command_socket)


    print "Creating data connection..."
    data_socket = create_connection(remote_host, data_port)
    print 'Data socket connected to', remote_host, 'on port #', data_port, '...'

    # run function to either get directory contents or get the requested file 
    if command_msg == "-l":
        _get_directory()
    elif command_msg == "-g":
        _get_file()


    data_socket.close()
    command_socket.close()



def _get_directory():
    return

def _get_file(file_name):
    return



def create_socket(port):
    host = gethostname()
    new_socket = socket(AF_INET, SOCK_STREAM)
    new_socket.bind((host, port))
    new_socket.listen(5)
    return new_socket


def _send_msg(msg_send, command_socket):

    # packet_length serves as a "header" to tell the receipient how big of a packet they are receiving
    packet_length = 2 + len(msg_send)

    # Build packet.
    packet = pack(">H", packet_length)
    packet += msg_send

    command_socket.sendall(packet)


"""
    FUNCTION:   establish_command_connection()
    receives:   the remote_host and port to connect to for sending ftp commands.
    returns:    the active command_socket.
    purpose:    create a command_socket and establish the connection w/ the server.
"""
def create_connection(remote_host, port):
    # initialize the client socket
    new_socket = socket(AF_INET, SOCK_STREAM)
    new_socket.connect((remote_host, port))

    return new_socket


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
