/* Program that requests the external IP address from a remote web server */

#define _WIN32_WINNT 0x501
#include <Windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstring>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include "WindowsSockets.h"

#define BUFLEN 4089

//Those are the message queues. 'inqueue' stores the messages we get from the remote server.
//'outqueue' stores the mesages that are to be sent to the server.
std::vector<std::string> inqueue;
std::vector<std::string> outqueue;

/*
NOTE: In this case 'outqueue' is unused. I added it because I want to use this program as a
template for other, more complex programs that require from a complete listener. Note that
the AGENT listener is not required either, but it's there for the same exact reason.
*/

//The GET request we send to the server. Ask for the /plain page using Agent Smith as out user agent.
//This might stand out a bit in their logs, but since we're not doing anything bad, it's not a big deal.
char* message = "GET /plain HTTP/1.1\nHost: ipecho.net\nUser-Agent: Agent Smith\n\n";

//Critical section
CRITICAL_SECTION cs_mutex;

//Trim the server messages. It's not a stetically appealing solution (no string.substr or boost), but
//it works well and it's quite simple and portable.
std::vector<std::string> MessageTrim(std::string mes)
{
    std::vector<std::string> result;
    std::string temp;
    
    for(uint32_t x = 0; x < mes.length(); x++)
    {
		//Splits whenever a carriage return + new line is found.
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
    
	//NOTE: Two carriage returns + new lines in a row means the message ends there.
	//This program does not handle that as it is not required for it's purpose.
	
    return result;
}

//Pop the first message in a queue. I want them FIFO.
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

//The listener thread. It's called AGENT since it acts in behalf of the main process, and/or the user.
//It's not necesarily an user-agent, in this case it acts in behalf of the main process.
DWORD WINAPI AGENT(LPVOID pParam)
{
	UNREFERENCED_PARAMETER(pParam);
    char bufin[BUFLEN];
    NetworkManager::Initialize();
	//std::ofstream ms;
	//ms.open("data.bin", std::ios::binary);
	//NOTE: ipecho is a external service whose purpose is to tell you the IP address you're using
	//to connect to the internet. In other words, the IP your ISP assigned to your router or whatever.
	//This service might not always be available. If that's the case, the command won't work.
	
    int rtcd = NetworkManager::Connect(std::string("ipecho.net"), true, std::string("80"));
        
    if(rtcd > 0)
    {
        std::cout << "Error stabilishing a connection with the host" << std::endl;
        return 0L;
    }
    
	//Request the page with the plain IP address.
    send(NetworkManager::GetSocket(), message, strlen(message), 0);
	
	//NOTE: MAKE SURE you ALWAYS LISTEN AND WAIT FOR A RESPONSE RIGHT AFTER A REQUEST. Do not send
	//any other requests until a response has been received, otherwise we might end bombing and
	//ddosing the server, which might be a very bad thing.
    
	//Wait for a response
    while(recv(NetworkManager::GetSocket(), bufin, BUFLEN, 0) < 1){}
    
	//Process the response and store it in the queue
	//ms << bufin;
    std::string incomming(bufin);
    EnterCriticalSection( &cs_mutex );
    inqueue.push_back(incomming);
    LeaveCriticalSection( &cs_mutex );
    
	//Disconnect and terminate the AGENT thread.
    NetworkManager::Disconnect();
	//ms.close();
}


//Mandatory main routine.
int main(int args, char** argv)
{
		//Initialize the critical section and start the AGENT
		HANDLE thread = 0L;
        InitializeCriticalSection(&cs_mutex);
		thread = CreateThread(NULL, 0L, AGENT, NULL, 0, NULL);
        
		//Wait for something to be processed
        while(inqueue.size() < 1){}//sleep(1);
        
		//Process whatever we have
        for(int x = 0; x < inqueue.size(); x++)
        {
            std::vector<std::string> procmsg = MessageTrim(Queue_pop_first(inqueue));
			
			//Line 12 is the HTML code. The rest is the header.
            //for(int y = 0; y < procmsg.size(); y++)
                //if(procmsg[y].compare("e") == 0)
                    std::cout << procmsg[11] << std::endl;
        }
        CloseHandle(thread);
        return 0;
}