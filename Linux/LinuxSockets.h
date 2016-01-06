#pragma once
#ifndef __AWESOMER_SOCKETS_H__
#define __AWESOMER_SOCKETS_H__
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>

#define SOCKET int
#define INVALID_SOCKET 0
#define DWORD uint32_t
#define SOCKET_ERROR -1
#define SD_RECEIVE 0x00
#define SD_SEND 0x01

class NetworkManager {
public:
	static int Initialize(void);
	static int Connect(std::string sIP, bool IpIsDomainName, std::string sPort);
	static bool IsInitialized(void);
	static SOCKET GetSocket(void);
	static void Disconnect();

private:
	

protected:
	
};

#endif