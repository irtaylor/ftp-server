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

#include <string>
using std::string;

#include <csignal>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>



// define the maximum message size
#ifndef MAX_SIZE
#define MAX_SIZE 500
#endif

// PROTOTYPES //
void signal_handler(int signum);
int create_socket(int port);
void ftp_session(int port, int server_socket);
char * get_command(int command_socket, int buffer_size);

void send_msg(int sock, char * msg);
int sendall(int sock, char * buf, int len);
int send_packet_length(int sock, unsigned int * packet_length);
char * recv_all(int command_socket);

char ** list_dir();
void send_dir(int data_socket, char ** dir_contents, int num_files);
void send_file(int data_socket, char ** dir_contents, char * file_name, int num_files);





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
void ftp_session(int port, int server_socket){

    while (true){
        cout << endl << "Server socket now listening on port " << port << "..." << endl;


        int command_socket, data_socket; // one socket to transfer commands, one to transfer data
        char * command_recv; // the received command
        char * client_IP; // the IPv4 client address
        char * file_name; // file to transfer.
        char * data_port_string; // string representation of the data_port_string for data transfers
        int data_port; // integer representation of the data port.
        //int status;
        int server_socket2;

        char ** dir_contents; // list of files in current directory;
        int num_files = 0; // number of files in directory;

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
        command_recv = recv_all(command_socket);

        if(strcmp(command_recv, "-g") == 0){
            file_name = recv_all(command_socket);
        }

        data_port_string = recv_all(command_socket);
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

        // DATA TRANSMISSION: Based on the user command, transfer either directory contents, or the file //
        cout << endl << "DATA TRANSMISSION:" << endl;

        // we will always need to get the directory contents and number of files//
        dir_contents = list_dir();
        for(int i = 0; dir_contents[i] != NULL; i += 1){
            num_files += 1;
        }

        // LIST directory contents
        if(strcmp(command_recv, "-l") == 0){
            cout << client_IP << " requested directory contents..." << endl;
            cout << "Sending directory to " << client_IP << ":" << data_port << "..." << endl;
            send_dir(data_socket, dir_contents, num_files);

        }
        // GET and send requested file
        else if(strcmp(command_recv, "-g") == 0){
            cout << client_IP << " requested file " << file_name << "..." << endl;
            cout << "Sending "<< file_name << " to " << client_IP << ":" << data_port << "..." << endl;
            send_file(data_socket, dir_contents, file_name, num_files);
        }

        // FREE dir_contents
        free(dir_contents);

        close(data_socket);
        close(server_socket2);
        close(command_socket);

    }
}

void send_dir(int data_socket, char ** dir_contents, int num_files){
    char num_files_string[100];


    sprintf(num_files_string,"%d",num_files);
    cout << "Number of files to send: " << num_files_string << endl;

    // inform the client of the number of incoming files first!
    send_msg(data_socket, num_files_string);

    for(int i = 0; dir_contents[i] != NULL; i += 1){
        send_msg(data_socket, dir_contents[i]);
    }
}

void send_file(int data_socket, char ** dir_contents, char * file_name, int num_files){
    bool file_exists = false;
    char error_msg[10] = "NOT FOUND";
    char success_msg[11] = "FILE FOUND";

    // check if the requested file exists
    for(int i = 0; dir_contents[i] != NULL; i += 1){
        if (strcmp(file_name, dir_contents[i]) == 0) {
					file_exists = true;
		}
    }
    if(!file_exists){
        send_msg(data_socket, error_msg);
    }
    else{
        send_msg(data_socket, success_msg);
        // TRANSFER THE FILE //
        

    }

}



char * recv_all(int command_socket){
    char * msg_recv;
    int total = 0;                      // Bytes received
    int n = 0;
    unsigned short packet_length;       // Number of bytes in packet
    unsigned short data_length;         // Number of bytes in encapsulated data

    // Receive the packet length.
    n = recv(command_socket, &packet_length, 2, 0);
    packet_length = ntohs(packet_length);
    //printf("Packet length: %d\n", packet_length);

    data_length = packet_length - sizeof(packet_length);
    //printf("Data length: %d\n", data_length);

    // allocate for the incoming message
    msg_recv = (char *)malloc(data_length * sizeof(char));

    // loop until all data received
    while(total < data_length){
        n = recv(command_socket, msg_recv + total, (data_length - total), 0);
        total += n;
    }
    msg_recv[data_length] = '\0';

    return msg_recv;
}



void send_msg(int sock, char * msg){
    // Prefix each message with a 4-byte length (network byte order)

    unsigned int packet_length;
    int len = strlen(msg);

    //cout << "Size of packet length: " << sizeof(packet_length) << endl;
    //cout << "Msg length: " << len << endl;

    // Pack the data!
    packet_length = htons(sizeof(packet_length) + len);

    //cout << "Packet length: " << packet_length << endl;
    if(send_packet_length(sock, &packet_length) == -1){
        perror("send_packet_length failed\n");
        exit(1);
    }

    sendall(sock, msg, len);

}


int send_packet_length(int sock, unsigned int * packet_length){
    int total = 0;
    int bytesleft = 4;
    int n;

    while(total < 4){
        n = send(sock, packet_length + total, bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }
    //cout << "Bytes sent: " << total << endl;

    return n==-1?-1:0; // return -1 on failure, 0 on success

}



int sendall(int sock, char * buf, int len){

    int total = 0;        // how many bytes we've sent
    int bytesleft = len; // how many we have left to send
    int n;

    while(total < len) {
        n = send(sock, buf + total, bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }

    len = total; // return number actually sent here
    //cout << "Bytes sent: " << len << endl;

    return n==-1?-1:0; // return -1 on failure, 0 on success
}




/**
 * FUNCTION:    _list_dir()
 * receives:    nothin.
 * returns:     nothin.
 * purpose:     an array of strings of the directory contents.
 * referenced from: http://stackoverflow.com/a/612176/4316660 and http://www.gnu.org/software/libc/manual/html_node/Directory-Entries.html
 **/
char ** list_dir(){

    char ** dir_contents = NULL;
    int num_files = 0;
    DIR *dir;
    struct dirent *dir_entry;

    if ((dir = opendir (".")) != NULL) {
        // print all the files and directories within directory
        while ((dir_entry = readdir (dir)) != NULL) {

            // For the first file in the directory, allocate initial space for dir_contents.
            // Otherwise, reallocate space for dir_contents for the next file.
            if(dir_contents == NULL){
                dir_contents = (char **)malloc(sizeof(char *));
            }
            else{
                dir_contents = (char **)realloc(dir_contents, (num_files + 1) * sizeof(char *));
            }

            // check that the basic malloc and realloc succeeded.
            assert(dir_contents != NULL);

            // allocate space for the size of the file name to be added to dir_contents, and validate the allocation.
            dir_contents[num_files] = (char *)malloc((strlen(dir_entry->d_name) + 1) * sizeof(char));
            assert(dir_contents[num_files] != NULL);

            dir_contents[num_files] = dir_entry->d_name;
            num_files += 1;
        }
        closedir (dir);
    }
    else {
        // could not open directory
        fprintf(stderr, "ftpserver: failed to open directory.\n");
        exit(1);
    }

    return dir_contents;
}


/**
 * FUNCTION:    _get_command()
 * receives:    the command_socket
 * returns:     nothin.
 * purpose:     read a command sent from the client and respond appropriately
 **/
char * get_command(int command_socket, int buffer_size){

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
 * FUNCTION:    signal_handler()
 * receives:    the signum desired to be handled.
 * returns:     nothin.
 * purpose:     gracefully handle SIGINT.
 **/
void signal_handler(int signum){
    cout << " Shutting down. Goodbye!" << endl;
    exit(signum);
}
