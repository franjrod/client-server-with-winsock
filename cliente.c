/*
	 Winsock client
*/

#include<stdio.h>
#include<winsock2.h>

#pragma comment(lib,"ws2_32.lib") 
#pragma warning(disable : 4996)

int main(int argc, char* argv[])
{
	WSADATA wsa;
	SOCKET s;
	struct sockaddr_in server;
	char message[2000], server_reply[2000];
	int recv_size;
	int ws_result;

	// Initialise winsock
	printf("\nInitialising Winsock...\n");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		return 1;
	}

	printf("Initialised.\n");

	//Create a socket
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
	}

	printf("Socket created.\n");

	// create the socket  address (ip address and port)
	server.sin_addr.s_addr = inet_addr("25.72.225.147");
	server.sin_family = AF_INET;
	server.sin_port = htons(68000);

	//Connect to remote server
	ws_result = connect(s, (struct sockaddr*)&server, sizeof(server));
	if (ws_result < 0)
	{
		puts("Connect error");
		return 1;
	}

	puts("Connected\n");

	//Recive
	if ((recv_size = recv(s, server_reply, 2000, 0)) > 0)
	{
		server_reply[recv_size + 1] = '\0';
		fprintf(stdout, "\nServer says: %s\n", server_reply);
	}

	puts("\nCommands:\n");
	puts("\tKEY - To generate a key\n");
	puts("\tQUIT - To close cnnection\n");

	while (TRUE)
	{
		/* Zeroing the buffers */
		memset(server_reply, 0x0, 2000);
		memset(message, 0x0, 2000);

		//Send
		fprintf(stdout, "Say something to the server: ");
		fgets(message, 2000, stdin);
		message[strlen(message) - 1] = '\0';
		send(s, message, strlen(message), 0);

		//Function KEY - Request key
		if (strcmp(message, "KEY") == 0)
		{
			//Recive
			recv_size = recv(s, server_reply, 2000, 0);
			printf("Server answer: %s\n", server_reply);
		}

		//Function QUIT - Request to close
		if (strcmp(message, "QUIT") == 0)
		{
			//Recive
			recv_size = recv(s, server_reply, 2000, 0);
			printf("Server answer: %s\n", server_reply);
			break;
		}
	}

	// Close the socket
	closesocket(s);

	//Cleanup winsock
	WSACleanup();

	return 0;
}

