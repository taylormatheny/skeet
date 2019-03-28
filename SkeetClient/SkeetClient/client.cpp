#include <stdio.h>
#include <winsock2.h>
#pragma comment(lib, "WS2_32")
#pragma warning(disable:4996)

typedef unsigned int unit32;
typedef unsigned short unit16;

const unit16 PORT = 9999;
const unit32 SOCKET_BUFFER_SIZE = 1024;

void main()
{

	// setting up winsocket will be the same as on the server
	WORD winsock_version = 0x202;
	WSADATA winsock_data;
	if (WSAStartup(winsock_version, &winsock_data))
	{
		printf("WSAStartup failed: %d", WSAGetLastError());
		return;
	}

	int address_family = AF_INET;
	int type = SOCK_DGRAM;
	int protocol = IPPROTO_UDP;
	SOCKET sock = socket(address_family, type, protocol);

	if (sock == INVALID_SOCKET)
	{
		printf("socket failed: %d", WSAGetLastError());
		return;
	}

	// send the data to the server
	SOCKADDR_IN server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(PORT);
	// inet_addr is depricated, so I should change this latter. 
	server_address.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

	// get a message from the user
	char message[SOCKET_BUFFER_SIZE];
	gets_s(message, SOCKET_BUFFER_SIZE);

	int flags = 0;
	// send the message to the server
	if (sendto(sock, message, strlen(message), flags,
		(SOCKADDR*)&server_address, sizeof(server_address)) == SOCKET_ERROR)
	{
		printf("sendto failed: %d", WSAGetLastError());
		return;
	}

	printf("done");

	return;
}