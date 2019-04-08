#include <stdio.h>
#include <WinSock2.h>
#include <time.h>

#include "variables.cpp"
#include "client_net.cpp"
#include "common_net.cpp"
#include "client_graphics.cpp"

#pragma comment(lib, "WS2_32")
#pragma warning(disable:4996)

struct Input
{
	bool32 up, down, left, right;
};

static bool32 g_is_running;
static Input g_input;

static void log_warning(const char* msg)
{
	OutputDebugStringA(msg);
}

static void update_input(WPARAM keycode, bool32 value)
{
	switch (keycode)
	{
		case 'A':
			g_input.left = value;
			break;
		case 'D':
			g_input.right = value;
			break;
		case 'w':
			g_input.up = value;
			break;
		case 'S':
			g_input.down = value;
			break;
	}
}

LRESULT CALLBACK WindowProc(HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param)
{
	switch (message)
	{
		case WM_QUIT:
		case WM_DESTROY:
			g_is_running = 0;
			break;
		case WM_KEYDOWN:
			update_input(w_param, 1);
			break;
		case WM_KEYUP:
			update_input(w_param, 0);
			break;
	}

	return DefWindowProc(window_handle, message, w_param, l_param);
}

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE /*prev_instance*/, LPSTR /*cmd_line*/, int cmd_show)
{
	WNDCLASS window_class;
	window_class.style = 0;
	window_class.lpfnWndProc = WindowProc;
	window_class.cbClsExtra = 0;
	window_class.cbWndExtra = 0;
	window_class.hInstance = instance;
	window_class.hIcon = 0;
	window_class.hCursor = 0;
	window_class.hbrBackground = 0;
	window_class.lpszMenuName = 0;
	window_class.lpszClassName = "app_window_class";

	ATOM window_class_atom = RegisterClass(&window_class);

	assert(window_class_atom);

	constexpr uint32 c_window_width = 1280;
	constexpr uint32 c_window_height = 720;

	HWND window_handle;
	{
		LPCSTR window_name = "";
		DWORD style = WS_OVERLAPPED;
		int x = CW_USEDEFAULT;
		int y = 0;
		HWND parent_window = 0;
		HMENU menu = 0;
		LPVOID param = 0;

		window_handle = CreateWindow(window_class.lpszClassName, window_name, style, x, y, c_window_width, c_window_height, parent_window, menu, instance, param);

		assert(window_handle);
	}
	ShowWindow(window_handle, cmd_show);

	// init graphics
	constexpr uint32 c_num_vertices = 4 * c_max_clients;
	Graphics::Vertex vertices[c_num_vertices];

	srand((unsigned int)time(0));
	for (uint32 i = 0; i < c_max_clients; ++i)
	{
		// generate colour for client
		float32 r = 0.0f;
		float32 g = 0.0f;
		float32 b = 0.0f;

		float32 temp = (float32)(rand() % 100) / 100.0f;

		switch (rand() % 6)
		{
		case 0:
			r = 1.0f;
			g = temp;
			break;

		case 1:
			r = temp;
			g = 1.0f;
			break;

		case 2:
			g = 1.0f;
			b = temp;
			break;

		case 3:
			g = temp;
			b = 1.0f;
			break;

		case 4:
			r = 1.0f;
			b = temp;
			break;

		case 5:
			r = temp;
			b = 1.0f;
			break;
		}

		// assign colour to all 4 verts, and zero positions to begin with
		uint32 start_verts = i * 4;
		for (uint32 j = 0; j < 4; ++j)
		{
			vertices[start_verts + j].pos_x = 0.0f;
			vertices[start_verts + j].pos_y = 0.0f;
			vertices[start_verts + j].col_r = r;
			vertices[start_verts + j].col_g = g;
			vertices[start_verts + j].col_b = b;
		}
	}

	constexpr uint32 c_num_indices = 6 * c_max_clients;
	uint16 indices[c_num_indices];

	for (uint16 index = 0, vertex = 0; index < c_num_indices; index += 6, vertex += 4)
	{
		// quads will be top left, bottom left, bottom right, top right
		indices[index] = vertex;				// 0
		indices[index + 1] = vertex + 2;		// 2
		indices[index + 2] = vertex + 1;		// 1
		indices[index + 3] = vertex;			// 0
		indices[index + 4] = vertex + 3;		// 3
		indices[index + 5] = vertex + 2;		// 2
	}

	Graphics::State graphics_state;
	Graphics::init(window_handle, instance, c_window_width, c_window_height, c_num_vertices, indices, c_num_indices, &graphics_state);

	// init winsock
	if (!Net::init())
	{
		log_warning("Net::init failed\n");
		return 0;
	}
	Net::Socket sock;
	if (!Net::socket_create(&sock))
	{
		log_warning("create_socket failed\n");
		return 0;
	}

	uint8 buffer[c_socket_buffer_size];
	Net::IP_Endpoint server_endpoint = Net::ip_endpoint_create(127, 0, 0, 1, c_port);

	buffer[0] = (uint8)Client_Message::Join;
	if (!Net::socket_send(&sock, buffer, 1, &server_endpoint))
	{
		log_warning("join message failed to send\n");
		return 0;
	}


	struct Player_State
	{
		float32 x, y, facing;
	};
	Player_State objects[c_max_clients];
	uint32 num_objects = 0;
	uint16 slot = 0xFFFF;

	Timing_Info timing_info = timing_info_create();


	// main loop
	g_is_running = 1;
	while (g_is_running)
	{

		// sample the clock at the beginning of each tick
		LARGE_INTEGER tick_start_time;
		QueryPerformanceCounter(&tick_start_time);

		// Windows messages from win32 programming resources
		MSG message;
		UINT filter_min = 0;
		UINT filter_max = 0;
		UINT remove_message = PM_REMOVE;
		// Grabs packets from the socket for as long as there are any available
		while (PeekMessage(&message, window_handle, filter_min, filter_max, remove_message))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}


		// Process Packets
		Net::IP_Endpoint from;
		uint32 bytes_received;
		// Packet Layout:
		// first byte is a 0 or 1, indicating if we were allowed to join
		// If that byte is 1, then the next 2 bytes are a uint16 for our slot
		while (Net::socket_receive(&sock, buffer, c_socket_buffer_size, &from, &bytes_received))
		{
			switch (buffer[0])
			{
			case Server_Message::Join_Result:
			{
				if (buffer[1])
				{
					memcpy(&slot, &buffer[2], sizeof(slot));
				}
				else
				{
					log_warning("server didn't let us in\n");
				}
			}
			break;

			// State packet contains all the player objects
			// read until we run out of bytes
			// Server sends the slot (ID) which isn't used yet
			// this field is to figure out which object the player actually owns
			case Server_Message::State:
			{
				num_objects = 0;
				uint32 bytes_read = 1;
				while (bytes_read < bytes_received)
				{
					uint16 id; // unused
					memcpy(&id, &buffer[bytes_read], sizeof(id));
					bytes_read += sizeof(id);

					memcpy(&objects[num_objects].x, &buffer[bytes_read], sizeof(objects[num_objects].x));
					bytes_read += sizeof(objects[num_objects].x);

					memcpy(&objects[num_objects].y, &buffer[bytes_read], sizeof(objects[num_objects].y));
					bytes_read += sizeof(objects[num_objects].y);

					memcpy(&objects[num_objects].facing, &buffer[bytes_read], sizeof(objects[num_objects].facing));
					bytes_read += sizeof(objects[num_objects].facing);

					++num_objects;
				}
			}
			break;
			}
		}

		// Send input
		// input is sent every tick
		// key events sent to the window, information is stored in global g_input struct
		// pakcet has four buttons encoded in bitfield
		if (slot != 0xFFFF)
		{
			buffer[0] = (uint8)Client_Message::Input;
			int bytes_written = 1;

			memcpy(&buffer[bytes_written], &slot, sizeof(slot));
			bytes_written += sizeof(slot);

			uint8 input = (uint8)g_input.up |
				((uint8)g_input.down << 1) |
				((uint8)g_input.left << 2) |
				((uint8)g_input.right << 3);
			buffer[bytes_written] = input;
			++bytes_written;

			if (!Net::socket_send(&sock, buffer, bytes_written, &server_endpoint))
			{
				log_warning("socket_send failed\n");
			}
		}


		// Draw
		// run through each object and read from the most recent state packet
		// compute the 4 vertex positions to represent it
		for (uint32 i = 0; i < num_objects; ++i)
		{
			constexpr float32 size = 0.05f;
			float32 x = objects[i].x * 0.01f;
			float32 y = objects[i].y * -0.01f;

			uint32 verts_start = i * 4;
			vertices[verts_start].pos_x = x - size; // TL (hdc y is +ve down screen)
			vertices[verts_start].pos_y = y - size;
			vertices[verts_start + 1].pos_x = x - size; // BL
			vertices[verts_start + 1].pos_y = y + size;
			vertices[verts_start + 2].pos_x = x + size; // BR
			vertices[verts_start + 2].pos_y = y + size;
			vertices[verts_start + 3].pos_x = x + size; // TR
			vertices[verts_start + 3].pos_y = y - size;
		}
		// zero the x and y positions of all vertices for player objects
		// which have no actual player to show
		for (uint32 i = num_objects; i < c_max_clients; ++i)
		{
			uint32 verts_start = i * 4;
			for (uint32 j = 0; j < 4; ++j)
			{
				vertices[verts_start + j].pos_x = vertices[verts_start + j].pos_y = 0.0f;
			}
		}
		// pass the updated vertices to the graphics system to update and draw
		// finally wait for the tick to end and start over
		Graphics::update_and_draw(vertices, c_num_vertices, &graphics_state);


		wait_for_tick_end(tick_start_time, &timing_info);
	}

	// send message to the server that we're leaving
	// if this isn't sent then the server times out
	buffer[0] = (uint8)Client_Message::Leave;
	int bytes_written = 1;
	memcpy(&buffer[bytes_written], &slot, sizeof(slot));
	Net::socket_send(&sock, buffer, bytes_written, &server_endpoint);

	// todo( jbr ) return wParam of WM_QUIT
	return 0;
}


/*
void main()
{

	printf("Hello Client!\n");
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

	int8 buffer[SOCKET_BUFFER_SIZE];
	int32 player_x;
	int32 player_y;

	printf("type w, a, s, or d to move, q to quit\n");
	bool32 is_running = 1;
	while (is_running)
	{
		// get input
		scanf_s("\n%c", &buffer[0], 1);

		// send to server
		int buffer_length = 1;
		int flags = 0;
		SOCKADDR* to = (SOCKADDR*)&server_address;
		int to_length = sizeof(server_address);
		if (sendto(sock, buffer, buffer_length, flags, to, to_length) == SOCKET_ERROR)
		{
			printf("sendto failed: %d", WSAGetLastError());
			return;
		}

		// wait for reply
		flags = 0;
		SOCKADDR_IN from;
		int from_size = sizeof(from);
		int bytes_received = recvfrom(sock, buffer, SOCKET_BUFFER_SIZE, flags, (SOCKADDR*)&from, &from_size);

		if (bytes_received == SOCKET_ERROR)
		{
			printf("recvfrom returned SOCKET_ERROR, WSAGetLastError() %d", WSAGetLastError());
			break;
		}

		// grab data from packet
		int32 read_index = 0;

		memcpy(&player_x, &buffer[read_index], sizeof(player_x));
		read_index += sizeof(player_x);

		memcpy(&player_y, &buffer[read_index], sizeof(player_y));
		read_index += sizeof(player_y);

		memcpy(&is_running, &buffer[read_index], sizeof(is_running));

		printf("x:%d, y:%d, is_running:%d\n", player_x, player_y, is_running);
	}

	printf("done");

	return;
}*/