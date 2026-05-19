#include <iostream>
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>

using namespace std;

// argc - how many on num of arguments + 1 in the same command line
// argv - very first thing on the command line <- [0], the next [1], and the next[2]

int main(int argc, char *argv[]) 
{
    // for the server, we only need to specify a port number
    if (argc != 2)
    {
        cerr << "Usage: port" << endl;
        exit(0);
    }

    // grab the port number
    int port = atoi(argv[1]); // only gets the first three numbers out of the ip(?)
    // buffer to send and receive messages with
    char msg[1500];

    // setup a socket and connection tools
    sockaddr_in servAddr;
    bzero((char*)&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = INADDR_ANY; // refer to test_socket_server.cpp for def
    servAddr.sin_port = htons(port); // refer to test_socket_server.cpp for def

    // open stream oriented socket with internet address
    // also keep track of socket descriptor
    int serverSd = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSd < 0)
    {
        cerr << "Error establishing the server socket" << endl;
        exit(0);
    }

    // bind the socket to its local address
    int bindStatus = ::bind(serverSd, (struct sockaddr*) &servAddr, sizeof(servAddr)); // ignore error

    if (bindStatus < 0)
    {
        cerr << "Error binding socket to local address" << endl;
        exit(0);
    }

    cout << "Waiting for client to connect..." << endl;
    // listen for up to 5 requests at a time
    listen(serverSd, 5);
    // receive a request from client using accept
    // we need a new address to connect with the client
    sockaddr_in newSockAddr;
    socklen_t newSockAddrSize = sizeof(newSockAddr);
    // accept, create a new socket descriptor to
    // handle the new connection with client
    int newSd = accept(serverSd, (sockaddr *)&newSockAddr, &newSockAddrSize);
    if (newSd < 0)
    {
        cerr << "Error accepting request from client!" << endl;
        exit(1);
    }

    cout << "Connected with client!" << endl;

    // keep track of session time
    struct timeval start1, end1;
    gettimeofday(&start1, NULL);
    // also keep track of the amount of data sent as well
    int bytesRead, bytesWritten = 0;
    while (1)
    {
        // receive a message from the client (listen)
        cout << "Awaiting client response..." << endl;
        memset(&msg, 0, sizeof(msg)); // clear the buffer
        bytesRead += recv(newSd, (char*) &msg, sizeof(msg), 0);
        if (!strcmp(msg, "exit"))
        {
            cout << "Client has quit the session" << endl;
            break;
        }
        cout << "Client: " << msg << endl;
        cout << ">";
        string data;
        getline(cin, data);
        memset(&msg, 0, sizeof(msg)); // clear the buffer
        strcpy(msg, data.c_str());
        if (data == "exit")
        {
            // send to client that server has closed the connection
            send(newSd, (char*)&msg, strlen(msg), 0);
            break;
        }

        // send the message to client
        bytesWritten += send(newSd, (char*) &msg, strlen(msg), 0);
    }

    // close the socket descriptors after finishing
    gettimeofday(&end1, NULL);
    close(newSd);
    close(serverSd);
    cout << "********Session********" << endl;
    cout << "Bytes written: " << bytesWritten << " Bytes read: " << bytesRead << endl;
    cout << "Elapsed time: " << (end1.tv_sec - start1.tv_sec) << " secs" << endl;
    cout << "Connection closed..." << endl;

    return 0;
}