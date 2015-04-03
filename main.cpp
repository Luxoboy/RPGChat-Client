/* 
 * File:   main.cpp
 * Author: Anthony Correia <anthony.correia71@gmail.com>
 *
 * Created on 3 avril 2015, 15:42
 */

#include <cstdlib>
#include <iostream>

#include "global.h"
#include "network.h"

#include "json/json.h"

using namespace std;
using namespace Json;

/*
 * 
 */
int main(int argc, char** argv)
{
    if(argc > 1)
        server_IP = argv[1];
    else
        server_IP = "localhost";
    if(argc > 2)
        socket_port = argv[2];
    else
        socket_port = "4654";
    
    cout << "--------------" << endl << "Init client. Params :" << endl 
            <<"Server IP " << server_IP << endl << "Socket port " << socket_port
            << endl << "-------------" << endl;
    
    if(initNetwork())
    {
        while(true)
        {
            char buf[1000];
            cin >> buf;
            send(buf);
        }
    }
        
    return 0;
}

void readMessage(char* msg)
{
    cout << msg;
}