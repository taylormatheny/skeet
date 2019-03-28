#include <WinSock2.h>
#include <stdio.h> // for printf
#pragma comment(lib, "WS2_32")

typedef unsigned int unit32;
typedef unsigned short unit16;

const unit16 PORT = 9999;
const unit32 SOCKET_BUFFER_SIZE = 1024;


void main()
{
	WORD winsock_version = 0x202;
	WSADATA winsock_data;

	// make sure winsock started correctly
	if (WSAStartup(winsock_version, &winsock_data))
	{
		printf("WSAStartup failed: %d", WSAGetLastError());
		return;
	}


	// now create a socket
	int address_family = AF_INET;
	int type = SOCK_DGRAM;

	// we will be using UDP
	int protocol = IPPROTO_UDP;

	SOCKET sock = socket(address_family, type, protocol);

	// check for errors
	if (sock == INVALID_SOCKET)
	{
		printf("socket failed: %d", WSAGetLastError());
		return;
	}

	// choose a specific port to bind our soccet to. It has to be above 1024.
	// for now, we will use 9999
	SOCKADDR_IN local_address;
	local_address.sin_family = AF_INET;

	// htons converts the port number to big-endian or network byte order
	local_address.sin_port = htons(PORT);

	// INADDR_ANY allows the socket to accept packets on all interfaces
	local_address.sin_addr.s_addr = INADDR_ANY;

	// if we were unable to bind
	if (bind(sock, (SOCKADDR*)&local_address, sizeof(local_address)) == SOCKET_ERROR)
	{
		printf("bind failed: %d", WSAGetLastError());
		return;
	}

	// now we start receiving packets
	// we want a small buffer size so we can send packets back and forth more often
	// because we are using UDP, we can have all clients send to the same socket
	char buffer[SOCKET_BUFFER_SIZE];
	int flags = 0;
	SOCKADDR_IN from;
	int from_size = sizeof(from);
	int bytes_received = recvfrom(sock, buffer, SOCKET_BUFFER_SIZE, flags, (SOCKADDR*)&from,
		&from_size);

	// if we are unable to receive from the server
	if (bytes_received == SOCKET_ERROR)
	{
		printf("recvfrom returned SOCKET_ERROR, WSAGetLastError() %d", WSAGetLastError());
	}
	// if we were able to receive from the server
	else
	{
		// this code can be used to figure out which player a message came from
		buffer[bytes_received] = 0;
		printf("%d.%d.%d.%d:%d - %s",
			from.sin_addr.S_un.S_un_b.s_b1,
			from.sin_addr.S_un.S_un_b.s_b2,
			from.sin_addr.S_un.S_un_b.s_b3,
			from.sin_addr.S_un.S_un_b.s_b4,
			from.sin_port,
			buffer);
	}

	printf("done");

	return;
}