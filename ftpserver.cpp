/**
 * Ian Taylor
 * CS 372 -- PROJECT 2
 * ftpserver.cpp: a simple server for a ftp server program written in C++
 **/

/** ftpserver.cpp uses the socket API to run a simple server for a chatting program. It sets of a socket and waits
 *  for the client to connect. The client and server exchange handles and can then begin alternating sending
 *  messages. The client-server interaction MUST follow the following form: Client messages Server, Server messages
 *  Client, Client messages again, etc. Messaging out of turn will cause errors. Either the client or server can
 *  close the connection by typing "\quit". This will cause the client to shutdown,
 **/

/**
 * NOTE: my primary reference for writing the server AND client programs was: http://www.linuxhowtos.org/C_C++/socket.htm
 * Additional references for specific lines of code are listed in the code itself.
 *
 **/


#include <iostream>
using std::cerr;
using std::cin;
using std::endl;
using std::cout;

#include <string.h>
#include <string>
using std::string;

#include <csignal>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>


// define the maximum message size
#ifndef MAX_SIZE
#define MAX_SIZE 500
#endif

// PROTOTYPES //
void signal_handler(int signum);
int create_socket(int port);
void ftp_session(int port, int server_socket);
char * _get_command(int command_socket, int buffer_size);
char * _recv_all(int command_socket);
void _show_directory();




int main(int argc, char*argv[]){

    // check the number of arguments.
    // ftpserver ONLY takes a port number to start the server
    if(argc != 2){
        cerr << "Usage: ./ftpserver <port #>" << endl;
        exit(1);
    }

    // install a signal handler for graceful shutdowns.
    signal(SIGINT, signal_handler);

    // get the command_port and server_socket to be used on the server
    int command_port = atoi(argv[1]);
    int server_socket = create_socket(command_port);

    // launch the ftp session
    ftp_session(command_port, server_socket);


    close(server_socket);


    return 0;
}

/**
 * FUNCTION:    create_socket()
 * receives:    the listening port.
 * returns:     the newly created server_socket.
 * purpose:     use the socket API to set up a basic socket to listen for and accept connections, and facilitate data transfer.
 **/
int create_socket(int port){

    // creates new socket.
    // params: 1) address domain of socket, 2) type of socket (STREAM reads in a stream as from a file / pipe), 3) the protocol. 0 chooses TCP for stream sockets, UDP for datagram sockets
    int new_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (new_socket < 0){
        fprintf(stderr, "ftpserver: error opening socket.\n");
        exit(1);
    }

    // create a structure containing an internet address:
    struct sockaddr_in socket_addr; // serv_addr contains server address.


    // sets all values in a buffer to zero
    // this line initializes serv_addr to zeros
    bzero((char *) &socket_addr, sizeof(socket_addr));

    // set up the serv_addr struct
    socket_addr.sin_family = AF_INET;
    socket_addr.sin_addr.s_addr = INADDR_ANY; // the IP of the machine the server is running
    socket_addr.sin_port = htons(port); //  little-endian to big-endian

    // binds socket to address (the address of the current host and the port number of the server
    // params: 1) socket fd, 2) address to which it is bound, 3) size of address
    if (bind(new_socket, (struct sockaddr *) &socket_addr, sizeof(socket_addr)) < 0){
        fprintf(stderr, "ftpserver: error on binding. perhaps try a different port?\n");
        exit(1);
    }

    // listen on the socket for connections. 2nd param is number of connections that can be waiting
    listen(new_socket, 5);

    //cout << "Now listening on port #" << port << "." << endl;

    return new_socket;

}







/**
 * FUNCTION:    ftp_session()
 * receives:    the listening port and the active server_socket
 * returns:     nothin.
 * purpose:     accept a connection from a client through the command port, and accept ftp commands. if the client or server close the connection, the server will resume listening on the port for a new connection until a SIGINT is received.
 **/
#ifndef COMMAND_SIZE
#define COMMAND_SIZE 2
#endif

#ifndef MAX_CXN_ATTEMPTS
#define MAX_CXN_ATTEMPTS 12 // Arbitrary number of connection requests
#endif
void ftp_session(int port, int server_socket){

    while (true){
        cout << endl << "Server socket now listening on port " << port << "..." << endl;


        int command_socket, data_socket; // one socket to transfer commands, one to transfer data
        char * command_recv; // the received command
        char * client_IP; // the IPv4 client address
        char * file_name; // file to transfer.
        char * data_port_string; // string representation of the data_port_string for data transfers
        int data_port; // integer representation of the data port.
        int connection_attempts = 0;
        int status;
        int server_socket2;

        struct sockaddr_in cli_addr; // cli_addr contains address of client to connect to the server

        // clilen stores the size of the address of the client. needed for accept() system call:
        // accept() causes process to block until a client connects to the server. thus, it wakes up the process when a connection from a client has been successfully established
        socklen_t clilen = sizeof(cli_addr);

        // create the socket on which to accept connections
        command_socket = accept(server_socket, (struct sockaddr *) &cli_addr, &clilen);


        if (command_socket < 0){
            fprintf(stderr, "ftpserver: error on command_socket accept().\n");
            exit(1);
        }

        // if connection goes through, print the IP of the connected client.
        // Source: http://linux.die.net/man/3/inet_ntoa
        client_IP = inet_ntoa(cli_addr.sin_addr);
        cout << "Command socket connected with " << client_IP << " on port #" << port << "..." << endl;

        // Get the commands from the client.
        command_recv = _recv_all(command_socket);

        if(strcmp(command_recv, "-g") == 0){
            file_name = _recv_all(command_socket);
        }

        data_port_string = _recv_all(command_socket);
        data_port = atoi(data_port_string);


        cout << endl << "Received commands from " << client_IP << ":" << endl;
        cout << "Command: " << command_recv << endl;

        if(strcmp(command_recv, "-g") == 0){
            cout << "File name: " << file_name << endl;
        }
        cout << "Data port: " << data_port << endl;


        // Create the data port connection
        cout << "Creating data connection..." << endl;
        server_socket2 = create_socket(data_port);

        data_socket = accept(server_socket2, (struct sockaddr *) &cli_addr, &clilen);
        if (data_socket < 0){
            fprintf(stderr, "ftpserver: error on data_socket accept().\n");
            exit(1);
        }
        cout << "Data socket connected with " << client_IP << "on port #" << data_port << "..." << endl;

        cout << endl << "DATA TRANSMISSION:" << endl;
        if(strcmp(command_recv, "-l") == 0){
            cout << client_IP << " requested directory contents..." << endl;
            cout << "Sending directory to " << client_IP << ":" << data_port << "..." << endl;
        }
        else if(strcmp(command_recv, "-g") == 0){
            cout << client_IP << " requested file " << file_name << "..." << endl;
            cout << "Sending "<< file_name << " to " << client_IP << ":" << data_port << "..." << endl;
        }



        close(data_socket);
        close(server_socket2);
        close(command_socket);

    }
}






/**
 * FUNCTION:    _recv_all()
 * receives:    the command_socket
 * returns:     the received message
 * purpose:     read from a socket
 **/
char * _recv_all(int command_socket){
    char * msg_recv = (char *)malloc((100 + 1) * sizeof(char));
    int n = 0;
    unsigned short packet_length;       // Number of bytes in packet
    unsigned short data_length;         // Number of bytes in encapsulated data

    // Receive the packet length.
    n = recv(command_socket, &packet_length, 2, 0);
    packet_length = ntohs(packet_length);

    data_length = packet_length - sizeof(packet_length);
    //printf("%d\n", data_length);

    n = recv(command_socket, msg_recv, data_length, 0);
    msg_recv[packet_length - 2] = '\0';

    return msg_recv;
}






/**
 * FUNCTION:    _get_command()
 * receives:    the command_socket
 * returns:     nothin.
 * purpose:     read a command sent from the client and respond appropriately
 **/
char * _get_command(int command_socket, int buffer_size){

    int bytes_received = 0;

    // ALLOCATE space for the incoming command
    char * command_msg = (char *)malloc((buffer_size + 1) * sizeof(char));
    if (!command_msg){
        fprintf(stderr, "ftpserver: allocation of command_msg failed.\n");
        exit(1);
    }

    bytes_received = recv(command_socket, command_msg, buffer_size, 0);
    cout << "BYTES: " << bytes_received << endl;

    if (bytes_received < 0){
        fprintf(stderr, "ftpservr: error reading command_msg from socket.\n");
    }
    command_msg[bytes_received] = '\0';

    send(command_socket, "", 0, 0);

    /*if (strcmp(command_msg, "-l") != 0 && strcmp(command_msg, "-g") != 0){
        fprintf(stderr, "ftpserver: received invalid command.\n");
    }*/

    return command_msg;

}

/**
 * FUNCTION:    _show_directory()
 * receives:    nothin...
 * returns:     nothin.
 * purpose:     show the ftp server's directory contents.
 **/
void _show_directory(){
    cout << "Sending directory to client..." << endl;

}

/**
 * FUNCTION:    signal_handler()
 * receives:    the signum desired to be handled.
 * returns:     nothin.
 * purpose:     gracefully handle SIGINT.
 **/
void signal_handler(int signum){
    cout << " Shutting down. Goodbye!" << endl;
    exit(signum);
}
