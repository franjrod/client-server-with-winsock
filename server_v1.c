/*
Simple winsock Server
*/

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <time.h>
#define TRUE 1
#define DS_TEST_PORT 68000
#define Path "..\\server\\chaves.txt"

#define MAX_LINE_LENGTH 80

#pragma comment (lib, "ws2_32.lib")
#pragma warning(disable : 4996)

void CheckFile(char* chave, int* found);
void WriteToFile(char* chave);
void GetKey(int numbers[], int stars[]);
void ordenar(int list[], int size);
int existsInArray(int k, int list[], int size);
int PedidoChave(char** keyLine[]);

// function ot handle the incoming connection
//	param: the socket of the calling client

DWORD WINAPI handleconnection(LPVOID lpParam);


int main()
{
	// Initialise winsock
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	printf("\nInitialising Winsock...");
	int wsResult = WSAStartup(ver, &wsData);
	if (wsResult != 0) {
		fprintf(stderr, "\nWinsock setup fail! Error Code : %d\n", WSAGetLastError());
		return 1;
	}

	// Create a socket
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening == INVALID_SOCKET) {
		fprintf(stderr, "\nSocket creationg fail! Error Code : %d\n", WSAGetLastError());
		return 1;
	}

	printf("\nSocket created.");

	// Bind the socket (ip address and port)
	struct sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(DS_TEST_PORT);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(listening, (struct sockaddr*)&hint, sizeof(hint));
	printf("\nSocket binded.");

	// Setup the socket for listening
	listen(listening, SOMAXCONN);
	printf("\nServer listening.");

	// Wait for connection
	struct sockaddr_in client;
	int clientSize;
	SOCKET clientSocket;
	SOCKET* ptclientSocket;
	DWORD dwThreadId;
	HANDLE  hThread;
	int conresult = 0;

	while (TRUE)
	{
		clientSize = sizeof(client);
		clientSocket = accept(listening, (struct sockaddr*)&client, &clientSize);

		ptclientSocket = &clientSocket;

		printf("\nHandling a new connection.");

		// Handle the communication with the client 

		hThread = CreateThread(
			NULL,                   // default security attributes
			0,                      // use default stack size  
			handleconnection,       // thread function name
			ptclientSocket,          // argument to thread function 
			0,                      // use default creation flags 
			&dwThreadId);   // returns the thread identifier 


		// Check the return value for success.
		// If CreateThread fails, terminate execution. 

		if (hThread == NULL)
		{
			printf("\nThread Creation error.");
			ExitProcess(3);
		}
	}

	// Close the socket
	closesocket(clientSocket);

	// Close listening socket
	closesocket(listening);

	//Cleanup winsock
	WSACleanup();
}



DWORD WINAPI handleconnection(LPVOID lpParam)
{
	char keyLine[MAX_LINE_LENGTH] = { 0 };
	char strMsg[1024];
	char strRec[1024];

	int i = 1;
	SOCKET cs;
	SOCKET* ptCs;

	ptCs = (SOCKET*)lpParam;
	cs = *ptCs;

	strcpy(strMsg, "\n100 OK...\n");
	printf("\n%s\n", strMsg);
	send(cs, strMsg, strlen(strMsg) + 1, 0);

	while (TRUE) {
		ZeroMemory(strRec, 1024);
		int bytesReceived = recv(cs, strRec, 1024, 0);
		if (bytesReceived == SOCKET_ERROR) {
			printf("\nReceive error!\n");
			break;
		}
		if (bytesReceived == 0) {
			printf("\nClient disconnected!\n");
			break;
		}

		printf("%i : %s\n", i++, strRec);
		send(cs, strRec, bytesReceived + 1, 0);

		if (strcmp(strRec, "date") == 0) {
			// current date/time based on current system
			time_t now = time(0);
			// convert now to string form
			char* dt = ctime(&now);

			strcpy(strMsg, "\n\nThe local date and time is: ");
			strcat(strMsg, dt);
			strcat(strMsg, "\n");

			send(cs, strMsg, strlen(strMsg) + 1, 0);

			// just to echo!
			// send(cs, strRec, bytesReceived + 1, 0);
		}

		if ((strcmp(strRec, "chave") == 0))
		{
			strcpy(strMsg, "\n");
			strcat(strMsg, ("\n", PedidoChave(keyLine)));
			strcat(strMsg, "\n");
			send(cs, strMsg, strlen(strMsg) + 1, 0);
		}

		if (strcmp(strRec, "bye") == 0) {

			strcpy(strMsg, "\nBye client...\n");
			send(cs, strMsg, strlen(strMsg) + 1, 0);

			// Close the socket
			closesocket(cs);
			return 0;
		}

	}

}

int PedidoChave(char** keyLine[]) {
	/*
	if (argc < 1)
	{
		return EXIT_FAILURE;
	}
	*/

	int numbers[5] = { 0, 0, 0, 0, 0 };
	int stars[2] = { 0, 0 };

	int found = 0;
	int counter = 0;
	//char keyLine[MAX_LINE_LENGTH] = { 0 };

	do {
		GetKey(numbers, stars);

		// TODO: remove test key
		/*
		if (counter == 0)
		{
			// Test key
			numbers[0] = 1;
			numbers[1] = 3;
			numbers[2] = 5;
			numbers[3] = 7;
			numbers[4] = 13;
			stars[0] = 2;
			stars[1] = 6;
		}
		*/

		ordenar(numbers, 5);
		ordenar(stars, 2);

		// Serialize key to compare
		sprintf(keyLine, "%d;%d;%d;%d;%d => %d;%d\n", numbers[0], numbers[1], numbers[2], numbers[3], numbers[4], stars[0], stars[1]);

		// Check if key exists
		CheckFile(keyLine, &found);

		counter++;
	} while (found == 1);

	WriteToFile(keyLine);

	//printf("key(%d):\n%s", counter, keyLine);

	return keyLine;
}

void CheckFile(char* chave, int* found) {
	int i;

	char line[MAX_LINE_LENGTH] = { 0 };
	unsigned int line_count = 0;
	char valor;

	FILE* file = fopen(Path, "r");

	if (!file)
	{
		perror(Path);
		return EXIT_FAILURE;
	}

	*found = 0;

	/* Get each line until there are none left */
	while (fgets(line, MAX_LINE_LENGTH, file))
	{
		/* Print each line */
		//printf("line[%06d]: %s", ++line_count, line);

		if (strcmp(chave, line) == 0)
		{
			*found = 1;
		}
	}

	/* Close file */
	if (fclose(file))
	{
		return EXIT_FAILURE;
		perror(Path);
	}
}

void WriteToFile(char* chave)
{
	FILE* fptr;

	fptr = (fopen(Path, "a"));
	if (fptr == NULL)
	{
		printf("Error!");
		exit(1);
	}

	fprintf(fptr, "%s", chave);

	fclose(fptr);
}

void GetKey(int numbers[], int stars[]) {
	srand(time(0));
	int i, j, k, t, n, m;
	k = 0;


	for (i = 0; i < 5; i++) {
		do {
			k = ((rand() % 50) + 1);
		} while (existsInArray(k, numbers, 5) == 1);

		numbers[i] = k;
	}

	for (i = 0; i < 2; i++) {
		do {
			k = ((rand() % 12) + 1);
		} while (existsInArray(k, stars, 2) == 1);

		stars[i] = k;
	}
}

void ordenar(int list[], int size) {
	int k, j, n, aux;
	n = size;

	for (k = 1; k < n; k++) {
		for (j = 0; j < n - k; j++) {
			if (list[j] > list[j + 1]) {
				aux = list[j];
				list[j] = list[j + 1];
				list[j + 1] = aux;
			}
		}
	}
}

int existsInArray(int k, int list[], int size) {
	for (int n = 0; n < size; n++) {
		if (k == list[n])
		{
			return 1;
		}
	}

	return 0;
}