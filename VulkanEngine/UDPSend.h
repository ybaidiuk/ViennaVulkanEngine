#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>


#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#if !defined(_WIN32)
#define SOCKET int
#endif

#if defined(_WIN32)
#define ISVALIDSOCKET(s) ((s) != INVALID_SOCKET)
#else
#define ISVALIDSOCKET(s) ((s) >= 0)
#endif




class UDPSend {

public:
	int sock;
	struct sockaddr_in addr;
	unsigned long packetnum;

	UDPSend();
	~UDPSend(){ if(sock) closesocket(sock); };
	void init(const char *address, int port );
	int send(char *buffer, int len  );
	void closeSock();
};



