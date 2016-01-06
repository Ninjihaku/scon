#pragma once
#ifndef __AWESOMER_SOCKETS_H__
#define __AWESOMER_SOCKETS_H__
#include <string>
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