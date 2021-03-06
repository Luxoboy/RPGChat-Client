#include "network.h"
#include "global.h"

#include <iostream>
#include <stdio.h>
#include <thread>

using namespace std;

int status;
struct addrinfo host_info; // The struct that getaddrinfo() fills up with data.
struct addrinfo *host_info_list; // Pointer to the to the linked list of host_info's.
std::string server_IP, socket_port;
int socket_d; // Socket descriptor
bool messageReceived;
char receivingBuffer[1000];
pthread_t receivingThread;
thread * listening_thread;
mutex statusMutex;

void recvThread()
{
    int socket_d_loc = socket_d;
    
    cout<< "Listening thread created successfully.\nListening...\n";
    while(true)
    {
        int res = recv(socket_d_loc, receivingBuffer, 1000, 0);
        
        if (res == 0)
        {
            cerr << "Connexion was shut down by server." << endl;
            statusMutex.lock();
            status = -1;
            statusMutex.unlock();
            return;
        }
        
        if(res != -1)
        {
            receivingBuffer[res] = '\0';
            messageReceived = true;
            readMessage(receivingBuffer);
        }
    }
    
    return;
}

bool initNetwork()
{
    host_info.ai_family = AF_INET; // IP version not specified. Can be both.
    host_info.ai_socktype = SOCK_STREAM; // Use SOCK_STREAM for TCP or SOCK_DGRAM for UDP.

    memset(&host_info, 0, sizeof host_info);

    status = getaddrinfo(server_IP.c_str(), socket_port.c_str(), &host_info, &host_info_list);

    if (status != 0)
    {
        cout << "Error setting the network structures with getadrinfo." << endl;
        return false;
    }
    
    if (!createSocket() || !connect())
    {
        return false;
    }
    
    cout << "Creating listening thread." << endl;
    listening_thread = new thread(&recvThread);
    status = 1;
    return true;
}

bool createSocket()
{
    cout << "Trying to create a socket..." << endl;
    socket_d = socket(host_info_list->ai_family, host_info_list->ai_socktype,
            host_info_list->ai_protocol);

    if (socket_d == -1)
    {
        cout << "Failed to create the socket." << endl;
        return false;
    }
    
    cout << "Socket created successfully." << endl;
    struct timeval timeout;      
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    
    if (setsockopt(socket_d, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
                sizeof(timeout)) < 0)
        perror("[SERVER] Setting socket timeout");
    return true;
}

bool closeSocket()
{
    if(close(socket_d) != 0)
    {
        perror("Closing socket");
        return false;
    }
    cout << "Socket closed successfully.\n";
    return true;
}

bool connect()
{
    cout << "Trying to connect to server." << endl;
    status = connect(socket_d, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1)
    {
        cerr << "Failed to connect to server." << endl;
        return false;
    }

    cout << "Successfully connected to server." << endl;
    return true;
}

bool send(string msg)
{
    if (status == -1)
    {
        return false;
    }
    
    int res = send(socket_d, msg.c_str(), msg.size(), 0);

    if ((uint)res != msg.size())
    {
        return false;
    }

    return true;
}
