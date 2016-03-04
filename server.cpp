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

// PROTOTYPES //
void signal_handler(int signum);
int create_socket(int port);
void recvAll(int socket, void *buffer, int numBytes);

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

    printf("Now listening on port %d...\n", command_port);


    int command_socket, data_socket; // one socket to transfer commands, one to transfer data
    char * command_recv; // the received command
    char * client_IP; // the IPv4 client address
    char * file_name; // file to transfer.
    char * data_port; // the data_port for data transfers

    struct sockaddr_in cli_addr; // cli_addr contains address of client to connect to the server

    // clilen stores the size of the address of the client. needed for accept() system call:
    // accept() causes process to block until a client connects to the server. thus, it wakes up the process when a connection from a client has been successfully established
    socklen_t clilen = sizeof(cli_addr);

    // create the socket on which to accept connections
    command_socket = accept(server_socket, (struct sockaddr *) &cli_addr, &clilen);


    if (command_socket < 0){
        fprintf(stderr, "ftpserver: error on accept().\n");
        exit(1);
    }

    // if connection goes through, print the IP of the connected client.
    // Source: http://linux.die.net/man/3/inet_ntoa
    client_IP = inet_ntoa(cli_addr.sin_addr);
    printf("Now connected with %s.\n", client_IP);


    char msg_recv[1024];
    int n = 0;
    unsigned short packetLength;       // Number of bytes in packet
	unsigned short dataLength;         // Number of bytes in encapsulated data
	//char tmpTag[TAG_LEN + 1];          // Temporary tag transfer buffer
	//char tmpData[MAX_PAYLOAD_LEN + 1]; // Temporary payload transfer buffer

	// Receive the packet length.
	//recvAll(command_socket, &packetLength, sizeof(packetLength));
    n = recv(command_socket, &packetLength, 2, 0);
	packetLength = ntohs(packetLength);

    dataLength = packetLength - sizeof(packetLength);
    printf("%d\n", dataLength);

    n = recv(command_socket, msg_recv, dataLength, 0);
    msg_recv[packetLength - 2] = '\0';
    cout << msg_recv << endl;






    close(server_socket);


    return 0;
}
/*
void recvAll(int socket, void *buffer, int numBytes)
{
	int ret;               // Return value for 'recv'
	int receivedBytes;     // Total number of bytes received

	// Retrieve the given number of bytes.
	receivedBytes = 0;
	while (receivedBytes < numBytes) {
		ret = recv(socket, buffer + receivedBytes, numBytes - receivedBytes, 0);

		// Error encountered.
		if (ret == -1) {
			perror("recv");
			exit(1);
		}

		// Data received.
		else {
			receivedBytes += ret;
		}
	}
}
*/




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
 * FUNCTION:    signal_handler()
 * receives:    the signum desired to be handled.
 * returns:     nothin.
 * purpose:     gracefully handle SIGINT.
 **/
void signal_handler(int signum){
    cout << " Shutting down. Goodbye!" << endl;
    exit(signum);
}
