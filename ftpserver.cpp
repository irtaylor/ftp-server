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
void get_command(int command_socket);



int main(int argc, char*argv[]){

    // check the number of arguments.
    // ftpserver ONLY takes a port number to start the server
    if(argc != 2){
        cerr << "Usage: ./ftpserver <port #>" << endl;
        exit(1);
    }

    signal(SIGINT, signal_handler);

    int command_port = atoi(argv[1]);
    int server_socket = create_socket(command_port);

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
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket < 0){
        fprintf(stderr, "ftpserver: error opening socket.\n");
        exit(1);
    }

    // create a structure containing an internet address:
    struct sockaddr_in serv_addr; // serv_addr contains server address.


    // sets all values in a buffer to zero
    // this line initializes serv_addr to zeros
    bzero((char *) &serv_addr, sizeof(serv_addr));

    // set up the serv_addr struct
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY; // the IP of the machine the server is running
    serv_addr.sin_port = htons(port); //  little-endian to big-endian

    // binds socket to address (the address of the current host and the port number of the server
    // params: 1) socket fd, 2) address to which it is bound, 3) size of address
    if (bind(server_socket, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        fprintf(stderr, "ftpserver: error on binding. perhaps try a different port?\n");
        exit(1);
    }

    // listen on the socket for connections. 2nd param is number of connections that can be waiting
    listen(server_socket, 5);

    //cout << "Now listening on port #" << port << "." << endl;

    return server_socket;

}


/**
 * FUNCTION:    ftp_session()
 * receives:    the listening port and the active server_socket
 * returns:     nothin.
 * purpose:     accept a connection from a client through the command port, and accept ftp commands. if the client or server close the connection, the server will resume listening on the port for a new connection until a SIGINT is received.
 **/
void ftp_session(int port, int server_socket){

    while (true){
        printf("Now listening on port %d...\n", port);

        struct sockaddr_in cli_addr; // cli_addr contains address of client to connect to the server

        // clilen stores the size of the address of the client. needed for accept() system call:
        // accept() causes process to block until a client connects to the server. thus, it wakes up the process when a connection from a client has been successfully established
        socklen_t clilen = sizeof(cli_addr);

        // create the socket on which to accept connections
        int new_server_socket = accept(server_socket, (struct sockaddr *) &cli_addr, &clilen);

        if (new_server_socket < 0){
            fprintf(stderr, "chatserve: error on accept().\n");
            exit(1);
        }
        else{
            // if connection goes through, print the IP of the connected client.
            // Source: http://linux.die.net/man/3/inet_ntoa
            printf("Now connected with %s.\n", inet_ntoa(cli_addr.sin_addr));
        }

        //char * client_handle = exchange_handle(new_server_socket, handle);
        //exchange_messages(new_server_socket, handle, client_handle);

        get_command(new_server_socket);

        close(new_server_socket);

    }
}

/**
 * FUNCTION:    get_command()
 * receives:    the command_socket
 * returns:     nothin.
 * purpose:     read a command sent from the client and respond appropriately
 **/
 #ifndef COMMAND_SIZE
 #define COMMAND_SIZE 2
 #endif
void get_command(int command_socket){
    string msg_send = "";

    // ALLOCATE space for the incoming command
    char * command_msg = (char *)malloc((COMMAND_SIZE + 1) * sizeof(char));
    if(!command_msg){
        fprintf(stderr, "ftpserver: allocation of command_msg failed.\n");
        exit(1);
    }

    command_msg = command_socket.recv(1024)
    /*print "Command Received: " + command_msg

    if (command_msg != "-l") and (command_msg != "-g"):
       print "ERROR: Received invalid command.\n"
       msg_send = "ERROR: Received invalid command.\nAccepted Commands: < -l | -g >\n"

    elif (command_msg == "-l"):
           msg_send = list_files()


    command_socket.send(msg_send)*/

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
