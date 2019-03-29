#include <WinSock2.h>
#include "variables.h"
#include <stdio.h> // for printf
#pragma comment(lib, "WS2_32")


void main()
{
	WORD winsock_version = 0x202;
	WSADATA winsock_data;

	printf("Hello Server!\n");

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

	// INADDR_ANY allows us to bind to any address
	local_address.sin_addr.s_addr = INADDR_ANY;

	// if we were unable to bind
	if (bind(sock, (SOCKADDR*)&local_address, sizeof(local_address)) == SOCKET_ERROR)
	{
		printf("bind failed: %d", WSAGetLastError());
		return;
	}

	// This is our coude to create buffers for two players so we can start receiving
	// packets
	int8 buffer[SOCKET_BUFFER_SIZE];
	int32 player_x = 0;
	int32 player_y = 0;

	bool32 is_running = 1;

	// now we start our loop to receive packets from both clients
	while (is_running)
	{
		// get input packet for clients position
		int flags = 0;
		SOCKADDR_IN from;
		int from_size = sizeof(from);
		int bytes_received = recvfrom(sock, buffer, SOCKET_BUFFER_SIZE, flags, (SOCKADDR*)&from,
			&from_size);

		// if there is a problem while receiving
		if (bytes_received == SOCKET_ERROR)
		{
			printf("recvfrom returned SOCKET_ERROR, WSAGetLastError() %d", WSAGetLastError());
			break;
		}

		// process user input
		char client_input = buffer[0];
		// show which client we are reading from
		printf("%d.%d.%d.%d:%d - %c\n", from.sin_addr.S_un.S_un_b.s_b1,
			from.sin_addr.S_un.S_un_b.s_b2, from.sin_addr.S_un.S_un_b.s_b3,
			from.sin_addr.S_un.S_un_b.s_b4, from.sin_port, client_input);

		// we switch the first character of the client input to the player's action
		switch (client_input)
		{
		case 'w':
			++player_y;
			break;
		case 'a':
			--player_x;
			break;
		case 's':
			--player_y;
			break;
		case 'd':
			++player_x;
			break;

		case 'q':
			is_running = 0;
			break;

		default:
			printf("unhandled input %c\n", client_input);
			break;
		}

		// now we create a state packet with the updated state of the game
		int32 write_index = 0;

		// first, we put all player 1's data into the buffer
		memcpy(&buffer[write_index], &player_x, sizeof(player_x));
		write_index += sizeof(player_x);

		// then we put all of player 2's data into the buffer
		memcpy(&buffer[write_index], &player_y, sizeof(player_y));
		write_index += sizeof(player_y);

		// then we indicate whether the server is still running
		memcpy(&buffer[write_index], &is_running, sizeof(is_running));

		// send back to client
		int buffer_length = sizeof(player_x) + sizeof(player_y) + sizeof(is_running);
		flags = 0;
		SOCKADDR* to = (SOCKADDR*)&from;
		int to_length = sizeof(from);
		
		// if we had an error sending data
		if (sendto(sock, buffer, buffer_length, flags, to, to_length) == SOCKET_ERROR)
		{
			printf("sendto failed: %d", WSAGetLastError());
			return;
		}
	}

	printf("done");

	// Do we need WSACleanup() ?
	return;
}