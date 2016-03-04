#!/usr/bin/env python

""" IMPORTS """
import sys
import socket
import signal

from socket import (            # Sockets API
    socket,
    gethostbyname,
    AF_INET,
    SOCK_STREAM,
    SOL_SOCKET,
    SO_REUSEADDR
)

from struct import pack, unpack # Structured binary data
"""         """

def main():
    # install interrupt handler to catch Ctrl+C
    # NOTE: USING SIGINT ON THE CLIENT WILL CAUSE ERRORS.
    signal.signal(signal.SIGINT, signal_handler)

    server_host = gethostbyname(sys.argv[1])
    server_port = int(sys.argv[2])

    # initialize the client socket
    command_socket = socket(AF_INET, SOCK_STREAM)
    command_socket.connect((server_host, server_port))
    print 'Now connected to', server_host, 'on command_port #', server_port


    msg_send = raw_input("please enter your message: ")

    packet_length = 2 + len(msg_send)

    # Build packet.
    packet = pack(">H", packet_length)
    packet += msg_send

    command_socket.sendall(packet)

    command_socket.close()



"""
    FUNCTION:   signal_handler()
    receives:   the signal to be handled and current stack frame.
    returns:    nothin.
    purpose:    handle SIGINT and shutdown "gracefully".
"""
def signal_handler(singal, frame):
    print " Shutting down. Goodbye!"
    sys.exit(0)



# Enables modularization of code by defining the entry point for the program.
if __name__ == '__main__':
    main()
