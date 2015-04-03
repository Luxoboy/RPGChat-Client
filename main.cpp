/* 
 * File:   main.cpp
 * Author: Anthony Correia <anthony.correia71@gmail.com>
 *
 * Created on 3 avril 2015, 15:42
 */

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <map>
#include <string>

#include "global.h"
#include "network.h"

#include "json/json.h"

char* messages_path = new char[50];
std::map<int, char*> messagesMap;

/**
 * Tries to load messages from file.
 * @return True if messages were loaded successfully.
 */
bool loadMessages();

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
    if(argc > 3)
        strcpy(messages_path, argv[3]);
    else
        strcpy(messages_path, "messages-fr");
    
    cout << "--------------" << endl << "Init client. Params :" << endl 
            <<"Server IP " << server_IP << endl << "Socket port " << socket_port
            << endl << "Messages file path: " << messages_path
            << endl << "-------------" << endl;
    
    if(loadMessages() && initNetwork())
    {
        cout << messagesMap[100];
        while(true)
        {
            char buf[1024];
            cin.getline(buf, 1024);
            send(buf);
            statusMutex.lock();
            if(status < 0)
                break;
            statusMutex.unlock();
        }
    }
        
    return 0;
}

void readMessage(char* msg)
{
    string str = "[";
    if(msg[0] == '1')
    {
        char* num = strtok(msg, " \r\n");
        int code = strtol(num, NULL, 0);
        if(num == 0L)
            return;
        str+=messagesMap[code];
        str+="]\n";
        cout << str;
    }
    else if(msg[0] == '{')
    {
        Reader r;
        Value root;
        if(r.parse(msg, root, false))
        {
            Value cmdVal = root["cmd"];
            if(cmdVal.isString())
            {
                string cmd = cmdVal.asString();
                if(cmd == "players")
                {
                    str+="List of players:\n";
                    Value players = root["players"];
                    for(Value player : players)
                    {
                        Value nickname = player["nickname"],
                                alive = player["alive"],
                                lp = player["lp"];
                        str+=nickname.asString()+" is ";
                        if(alive.asBool())
                            str+="alive";
                        else
                            str+="dead";
                        if(!lp.isNull())
                            str+=" ("+to_string(lp.asInt())+"lp)";
                    }
                }
                else if(cmd == "lp")
                {
                    Value lp = root["lp"], alive = root["alive"];
                    if(alive.asBool())
                    {
                        str+="You are alive and you have "+to_string(lp.asInt())+" life points.";
                    }
                    else
                        str+="You are dead.";
                }
                else if(cmd == "talk")
                {
                    Value nickname = root["nickname"], msg = root["msg"],
                            master = root["fromMaster"];
                    if(master.asBool())
                        str+="GAME MASTER says ";
                    else
                        str+=nickname.asString()+" says ";
                    str+="\""+msg.asString()+"\"";
                }
                else
                    return;
                str+="]\n";
                cout << str;
            }
        }
    }
}

bool loadMessages()
{
    bool ret = true;
    ifstream file(messages_path);
    if(file.is_open())
    {
        char line[1024], *num;
        file.getline(line, 1024);
        while(line[0] != '\0')
        {
            num = strtok(line, " ");
            if(num == NULL)
            {
                ret = false;
                messagesMap.clear();
                break;
            }
            int code = strtol(num, NULL, 0);
            if(code != 0L)
            {
                num = strtok(NULL, "\n");
                if(num == NULL)
                {
                    ret = false;
                    messagesMap.clear();
                    break;
                }
                char* duplicate = new char[strlen(num)+1];
                strcpy(duplicate, num);
                messagesMap[code] = duplicate;
                cout << "Loaded message " << code << " \"" << messagesMap[code]
                        << "\"" << endl;
            }
            file.getline(line, 1024);
        }
        file.close();
        cout << "Messages loaded successfully." << endl;
    }
    else
    {
        cout << "Failed to open messages file. Exiting." << endl;
        ret = false;
    }
    return ret;
}