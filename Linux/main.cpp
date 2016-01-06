#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
//#include <ws2tcpip.h>
#include <pthread.h>

#include <iostream>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <string>
#include <fstream>
#include "LinuxSockets.h"

#define BUFLEN 4089

std::vector<std::string> inqueue;
std::vector<std::string> outqueue;

char* message = "GET /plain HTTP/1.1\nHost: ipecho.net\nUser-Agent: Agent Smith\n\n";
static pthread_mutex_t cs_mutex =  PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

std::vector<std::string> MessageTrim(std::string mes)
{
    std::vector<std::string> result;
    std::string temp;
    
    for(uint32_t x = 0; x < mes.length(); x++)
    {
        if(mes[x] == 0x0D && mes[x+1] == 0x0A)
        {
            result.push_back(temp);
            temp = std::string();
            x++;
            continue;
        }
        
        if(mes[x] < 0x20 || mes[x] > 0x7E)
            continue;
        
        temp += mes[x];
    }
    
    return result;
}

std::string Queue_pop_first(std::vector<std::string>& queue)
{
    std::string result;
    if(queue.size() > 0)
    {
        result = queue[0];
        queue.erase(queue.begin(), queue.begin());
    }
    return result;
}

void* AGENT(void* ptr)
{
    char bufin[BUFLEN];
    NetworkManager::Initialize();
    int rtcd = NetworkManager::Connect(std::string("ipecho.net"), true, std::string("80"));
        
    if(rtcd > 0)
    {
        std::cout << "Error stabilishing a connection with the host" << std::endl;
        return 0L;
    }
    
    send(NetworkManager::GetSocket(), message, strlen(message), 0);
    
    while(recv(NetworkManager::GetSocket(), bufin, BUFLEN, 0) < 1)
    {
        sleep(1);
    }
    
    std::string incomming(bufin);
    pthread_mutex_lock( &cs_mutex );
    inqueue.push_back(incomming);
    pthread_mutex_unlock( &cs_mutex );
    
    NetworkManager::Disconnect();
    return 0L;
}

int main(int args, char** argv)
{
        pthread_t agentthread;
        agentthread = pthread_create(&agentthread, 0L, AGENT, 0L);
        pthread_join(agentthread, NULL);
        
        while(inqueue.size() < 1)
            sleep(1);
        
        for(int x = 0; x < inqueue.size(); x++)
        {
            std::vector<std::string> procmsg = MessageTrim(Queue_pop_first(inqueue));
            for(int y = 0; y < procmsg.size(); y++)
                if(procmsg[y].compare("e") == 0)
                    std::cout << procmsg[y+1] << std::endl;
        }
        
        return 0;
}