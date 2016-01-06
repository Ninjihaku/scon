#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <unistd.h>
//#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <cstring>
#include "LinuxSockets.h"

struct addrinfo hints;
struct addrinfo *addrresult = NULL;
struct sockaddr_in  *sockaddr_ipv4 = NULL;
struct addrinfo *res = NULL, *ptr = NULL, addrinfo; //The socket addresses and information.
SOCKET ConnectSocket = INVALID_SOCKET, ServerSocket = INVALID_SOCKET; //The socket we are going to use for listening.
u_long iMode = 1;
bool isInitialized = false;

//Funtion that returns the state of the connection (If it was initialized or not)
bool NetworkManager::IsInitialized()
{
	return isInitialized;
}

//Function to initialize Winsock
int NetworkManager::Initialize(){
	//ZeroMemory(&addrinfo, sizeof(addrinfo)); //Fill addrinfo
        memset(&addrinfo, 0x0, sizeof(addrinfo));
	addrinfo.ai_family = AF_INET; // Return IPV4 addresses
	addrinfo.ai_protocol = IPPROTO_TCP; //TCP/IP
	addrinfo.ai_socktype = SOCK_STREAM; //Stream socket
	isInitialized = true; //We did the initialization
	return 0;
}

//Function to make a connection over TCP/IP, using either an IP or domain name.
int NetworkManager::Connect(std::string sIP, bool IpIsDomainName, std::string sPort)
{
	std::string ipAddr;
	if(sIP.size() < 1)
	{
		return 420; //sIP must not be null
	}

	if(!IsInitialized())
		Initialize(); //Ensure Winsock2 is initialized

	if(IpIsDomainName)
	{
		/* Domain Name resolution */
		DWORD dwretval = getaddrinfo((char*)&sIP[0], NULL, &hints, &addrresult); //Get the domain information
		if(dwretval != 0)
		{
			std::cout << "Failed to resolve the host name: " << sIP << ". Make sure that the name is correct and that your computer has online access." << std::endl;
			return dwretval; //Failed to resolve the host name
		}
		for(ptr=addrresult; ptr != NULL ;ptr=ptr->ai_next)
		{
			if(ptr->ai_family == AF_INET) //Look for an IPv4
			{
				sockaddr_ipv4 = (struct sockaddr_in *) ptr->ai_addr; //Retrieve the IPv4 information
				ipAddr = inet_ntoa(sockaddr_ipv4->sin_addr); //Get the IP
			}
		}
		if(ipAddr.size() < 1)
		{
			std::cout << "Error: Domain name resolved, but no IPv4 was found. This program does not support IPv6." << std::endl;
			return 420;
		}
	}else{
		ipAddr = sIP; //Assume we got an IP.
	}

	if(getaddrinfo((char*)&ipAddr[0],(char*)&sPort[0],&addrinfo,&res) != 0) {
		std::cout << "Failed to get socket information (" << ipAddr << ":" << sPort << "). Press any key to exit." << std::endl;
		//std::cin.ignore().get();
		return 420;
	}
	ptr = res; //Copy pointers. Both ptr and res point to the same memory region. It was like this in the MSDN.
	ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol); //This creates a connection socket

	if(ConnectSocket == INVALID_SOCKET)
	{
		std::cout << "Failed to create a listening socket. Maybe the port is in use? Press any key to exit." << std::endl;
		close(ConnectSocket);
		//std::cin.ignore().get();
		return 420;
	}

	if(connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen) == SOCKET_ERROR)
	{
		std::cout << "Could not connect to the server. This can be due to many causes (port in use, server not reachable or offline, wrong IP, etc). Press a key to exit." << std::endl;
		close(ConnectSocket);
		//std::cin.ignore().get();
		return 420;
	}

	freeaddrinfo(res); // Free the memory allocated for the addrinfo. I guess this is the reason why having two pointers to the same address.
	ioctl(ConnectSocket, FIONBIO, &iMode); //Make the connection socket non-blocking (Asynchronous).
	return 0;
}

//Function that returns the current socket in use
SOCKET NetworkManager::GetSocket()
{
	return ConnectSocket;
}

//Function to disconnect from wherever we are connected
void NetworkManager::Disconnect()
{
	shutdown(NetworkManager::GetSocket(), SD_SEND); //Disconnect from the server and terminate the program.
	close(NetworkManager::GetSocket()); //Dispose of the socket
	return;
}