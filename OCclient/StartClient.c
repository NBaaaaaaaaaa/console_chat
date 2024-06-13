#include<stdio.h>
#include<winsock2.h>
#include <Ws2tcpip.h> 

#include <windows.h>

#pragma comment(lib,"ws2_32.lib")

// Структура данных клиента.
struct clientData {
	SOCKET client_socket;
	HANDLE* mutex;
};

void listet_server(struct clientData* data) {
	int recv_size;
	char server_reply[2000];

	while (TRUE) {
		if ((recv_size = recv(data->client_socket, server_reply, 2000, 0)) == SOCKET_ERROR) {
			puts("recv failed");
		}

		//puts("Reply received\n");

		if (recv_size >= 0 && recv_size < 2000) {
			server_reply[recv_size] = '\0';
			puts(server_reply);
		}
		else {
			// Обработка ошибки или прерывания соединения
			// Например:
			puts("Error: Invalid recv_size or buffer overflow occurred");
			break;
		}
	}
}

int main(int argc, char* argv[]) {
	WSADATA wsa;
	SOCKET s;
	struct sockaddr_in server;
	//char* message;

	char buffer[2000];

	struct clientData client_struc;
	HANDLE hMutex = CreateMutex(NULL, FALSE, NULL);

	// Инициализация winsock
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		return 1;
	}

	// Создание сокета
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
	}

	// Заполнение полей структуры 
	if (inet_pton(AF_INET, "127.0.0.1", &server.sin_addr) <= 0) {
		printf("Ошибка преобразования IP-адреса\n");
		return 1;
	}

	server.sin_family = AF_INET;
	server.sin_port = htons(8888);

	// Попытка установки соединения
	if (connect(s, (struct sockaddr*)&server, sizeof(server)) < 0)
	{
		return 1;
	}

	
	client_struc.client_socket = s;
	client_struc.mutex = &hMutex;

	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)listet_server, &client_struc, 0, NULL);

	// Попытка отправки сообщения
	//message = "Привет hello";
	while (TRUE) {
		fgets(buffer, sizeof(buffer), stdin);
		if (send(s, buffer, (int)strlen(buffer), 0) < 0)
		{
			return 1;
		}
		//puts(buffer);
	}

	// Проверка, были ли данные получены
	/*
	if ((recv_size = recv(s, server_reply, 2000, 0)) == SOCKET_ERROR)
	{
		puts("recv failed");
	}

	puts("Reply received\n");

	// Вывод принятого сообщения в консоль
	if (recv_size >= 0 && recv_size < 2000) {
		server_reply[recv_size] = '\0';
		puts(server_reply);
	}
	else {
		// Обработка ошибки или прерывания соединения
		// Например:
		puts("Error: Invalid recv_size or buffer overflow occurred");
	}
	*/
	//Add a NULL terminating character to make it a proper string before printing
	

	return 0;
}