#include<stdio.h>
#include<winsock2.h>
#include <locale.h>

#pragma comment(lib,"ws2_32.lib")

int main(int argc, char* argv[])
{
	WSADATA wsa;
	SOCKET s;
	struct sockaddr_in server;
	char* message, server_reply[2000];
	int recv_size;

	char buffer[2000];

	setlocale(LC_ALL, "UTF-8");

	// ������������� winsock
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		return 1;
	}

	// �������� ������
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
	}

	// ���������� ����� ��������� 
	if (inet_pton(AF_INET, "127.0.0.1", &server.sin_addr) <= 0) {
		printf("������ �������������� IP-������\n");
		return 1;
	}

	server.sin_family = AF_INET;
	server.sin_port = htons(8888);

	// ������� ��������� ����������
	if (connect(s, (struct sockaddr*)&server, sizeof(server)) < 0)
	{
		return 1;
	}


	// ������� �������� ���������
	//message = "������ hello";
	while (TRUE) {
		fgets(buffer, sizeof(buffer), stdin);
		if (send(s, buffer, (int)strlen(buffer), 0) < 0)
		{
			return 1;
		}
		puts(buffer);
	}

	// ��������, ���� �� ������ ��������
	/*
	if ((recv_size = recv(s, server_reply, 2000, 0)) == SOCKET_ERROR)
	{
		puts("recv failed");
	}

	puts("Reply received\n");

	// ����� ��������� ��������� � �������
	if (recv_size >= 0 && recv_size < 2000) {
		server_reply[recv_size] = '\0';
		puts(server_reply);
	}
	else {
		// ��������� ������ ��� ���������� ����������
		// ��������:
		puts("Error: Invalid recv_size or buffer overflow occurred");
	}
	*/
	//Add a NULL terminating character to make it a proper string before printing
	

	return 0;
}