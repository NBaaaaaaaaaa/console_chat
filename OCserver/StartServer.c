#include<io.h>
#include<stdio.h>
#include<winsock2.h>
#include <locale.h>

#pragma comment(lib,"ws2_32.lib")


SOCKET *online_clients = NULL;
size_t number_online_clients = 0;

// ���������� ������� � ������ ������ �������������
void addClient(const SOCKET *client_socket) {
	number_online_clients++;
	SOCKET* temp = realloc(online_clients, number_online_clients * sizeof(SOCKET));
	if (temp == NULL) {
		printf("������: �� ������� �������� ������\n");
		return;
	}
	online_clients = temp;
	online_clients[number_online_clients - 1] = *client_socket; // ��������� ������ ������� � ����� �������
}

void delClient(const SOCKET* client_socket) {
	// ����� ������� ������������ �� ������
	int index = -1;
	for (int i = 0; i < number_online_clients; i++) {
		if (online_clients[i] == *client_socket) {
			index = i;
			break;
		}
	}

	if (index == -1) {
		// �������� ����������� ����� �����
		printf("������: ������������ �� ������\n");
		return;
	}

	// ������������ ������ ��� ���������� ������������
	for (int i = index; i < number_online_clients - 1; i++) {
		online_clients[i] = online_clients[i + 1]; // �������� �������� �������
	}
	number_online_clients--; // ��������� ���������� �������������
	online_clients = realloc(online_clients, number_online_clients * sizeof(SOCKET)); // ��������� ������ �������
	printf("%d", (int)number_online_clients);
}


int main(int argc, char* argv[])
{
	WSADATA wsa;
	SOCKET s, new_socket;
	struct sockaddr_in server, client;
	int c;
	char* message, client_reply[2000];
	int recv_size;

	setlocale(LC_ALL, "UTF-8");

	// ������������� winsock
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		return 1;
	}

	printf("Initialised.\n");

	// �������� ������
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
	}

	printf("Socket created.\n");

	// ���������� ����� ��������� 
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(8888);

	// �������� ������ � ������
	if (bind(s, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	puts("Bind done");

	// ������������� �������� ����������. 3 - ������ �������
	listen(s, 3);

	// ����� �����������
	puts("Waiting for incoming connections...");

	c = sizeof(struct sockaddr_in);

	while ((new_socket = accept(s, (struct sockaddr*)&client, &c)) != INVALID_SOCKET)
	{
		puts("Connection accepted");
		addClient(&new_socket);

		while (TRUE) {
			if ((recv_size = recv(new_socket, client_reply, 2000, 0)) == SOCKET_ERROR)
			{
				puts("recv failed");
			}

			puts("Reply received\n");

			// ����� ��������� ��������� � �������
			if (recv_size >= 0 && recv_size < 2000) {
				client_reply[recv_size] = '\0';
				puts(client_reply);
			}
			else {
				// ��������� ������ ��� ���������� ����������
				// ��������:
				puts("Error: Invalid recv_size or buffer overflow occurred");

				delClient(&new_socket);
				closesocket(new_socket);
				return 1;
			}

			// �������� ��������� �������
			//message = "Hello Client , I have received your connection. But I have to go now, bye\n";
			//send(new_socket, message, strlen(message), 0);
	
		}

		
	}

	if (new_socket == INVALID_SOCKET)
	{
		printf("accept failed with error code : %d", WSAGetLastError());
		return 1;
	}

	closesocket(s);
	WSACleanup();

	return 0;
}