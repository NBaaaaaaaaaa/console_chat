#include<io.h>
#include<stdio.h>
#include<winsock2.h>
#include <windows.h>
#include <string.h>
#include <time.h>
#include "LogInSignUp.h"

#pragma comment(lib,"ws2_32.lib")

struct clientData **online_clients = NULL;
int number_online_clients = 0;

// Структура данных клиента. Передается в поток.
struct clientData{
	enum StatusClient status_client;
	SOCKET client_socket;
	HANDLE *mutex;
};

// Добавление клиента в массив онлайн пользователей
void addClient(struct clientData *data) {
	number_online_clients++;
	struct clientData** temp = realloc(online_clients, number_online_clients * sizeof(struct clientData));
	if (temp == NULL) {
		printf("Ошибка: Не удалось выделить память\n");
		return;
	}
	online_clients = temp;

	// Добавляем нового клиента в конец массива
	online_clients[number_online_clients - 1] = data; 

	printf("Online clients - %d \n", number_online_clients);
}

// Удаление клиента из массива онлайн пользователей
void delClient(struct clientData *data) {
	// Поиск клиента в массиве
	for (size_t i = 0; i < number_online_clients; i++) {
		if (online_clients[i] == data) {
			// Найден клиент, удаляем его из массива
			for (size_t j = i; j < number_online_clients - 1; j++) {
				online_clients[j] = online_clients[j + 1];
			}
			// Уменьшаем количество онлайн клиентов
			number_online_clients--;

			// После удаления, уменьшаем размер массива
			struct clientData** temp = realloc(online_clients, number_online_clients * sizeof(struct clientData*));
			if (temp == NULL && number_online_clients > 0) {
				printf("Ошибка: Не удалось изменить размер массива\n");
				return;
			}
			online_clients = temp;

			printf("del. Online - %d\n", number_online_clients);
			return;
		}
	}

	// Если клиент не найден в массиве
	printf("Ошибка: Клиент не найден в массиве онлайн клиентов\n");
}


// Обработка подключенного клиента
void processingClient(struct clientData *data) {
	char* message, client_reply[2000];
	int message_len;
	int recv_size;

	struct clientData client_struc = *data;
	struct Client_un_pw username_password = {0};

	time_t time_now;

	SOCKET client_socket = client_struc.client_socket;

	puts("Connection accepted");

	// Добавление пользователя в массив
	WaitForSingleObject(client_struc.mutex, INFINITE);
	addClient(&client_struc);
	ReleaseMutex(client_struc.mutex);

	while (TRUE) {
		switch (client_struc.status_client)
		{
		case NOT_AUTHORIZED:
			message = "reg - sign up\nlog - log in";
			send(client_struc.client_socket, message, (int)strlen(message), 0);
			break;
		case ENTER_USERNAME:
			message = "username: ";
			send(client_struc.client_socket, message, (int)strlen(message), 0);
			break;
		case ENTER_PASSWORD:
			message = "password: ";
			send(client_struc.client_socket, message, (int)strlen(message), 0);
			break;
		}
		
		// Получение сообщения от пользователя
		if ((recv_size = recv(client_socket, client_reply, 2000, 0)) == SOCKET_ERROR)
		{
			puts("recv failed");
			//надо тут сделать закрытие всего что можно
			// Удаление пользователя из массива
			WaitForSingleObject(client_struc.mutex, INFINITE);
			delClient(&client_struc);
			ReleaseMutex(client_struc.mutex);
			return;
		}

		puts("Reply received\n");

		if (recv_size >= 0 && recv_size < 2000) {
			client_reply[recv_size - 1] = '\0';

			switch (client_struc.status_client)
			{
			case NOT_AUTHORIZED:
				
				if (!strcmp(client_reply, "reg\0")) {
					username_password.reg_or_log = 0;
				}
				else if (!strcmp(client_reply, "log\0")) {
					username_password.reg_or_log = 1;
				}
				else {
					break;
				}

				client_struc.status_client = ENTER_USERNAME;
				break;
			case ENTER_USERNAME:
				strcpy_s(username_password.username, sizeof(username_password.username), client_reply);
				
				client_struc.status_client = ENTER_PASSWORD;
				break;
			case ENTER_PASSWORD:
				strcpy_s(username_password.password, sizeof(username_password.username), client_reply);

				int result;
				
				if (username_password.reg_or_log) {
					result = log_in(username_password.username, username_password.password);
				}
				else {
					result = sign_up(username_password.username, username_password.password);
				}
				
				if (result) {
					client_struc.status_client = AUTHORIZED;
					break;
				}
				message = "Error(((";
				send(client_struc.client_socket, message, (int)strlen(message), 0);
				client_struc.status_client = NOT_AUTHORIZED;
				break;
				

			case AUTHORIZED:
				// Рассылка сообщения онлайн клиаентам
				time(&time_now);
				struct tm* local_time = localtime(&time_now);
				message_len = 6 + strlen(username_password.username) + 3 + recv_size;
				message = (char*)malloc(message_len * sizeof(char));
				snprintf(message, message_len, "%02d:%02d %s : %s", local_time->tm_hour, local_time->tm_min, username_password.username, client_reply);


				for (size_t i = 0; i < number_online_clients; i++) {
					if (online_clients[i]->client_socket != client_socket && online_clients[i]->status_client == AUTHORIZED) {
						
						send(online_clients[i]->client_socket, message, message_len, 0);
					}
				}
				//puts(client_reply);
				free(message);
			}

		}
		else {
			// Обработка ошибки или прерывания соединения
			// Например:
			puts("Error: Invalid recv_size or buffer overflow occurred");


			// Удаление пользователя из массива
			WaitForSingleObject(client_struc.mutex, INFINITE);
			delClient(&client_struc);
			ReleaseMutex(client_struc.mutex);

			closesocket(client_socket);
			ExitThread(0);
			return;
		}

		// Отправка сообщения клиенту
		//message = "Hello Client , I have received your connection. But I have to go now, bye\n";
		//send(new_socket, message, strlen(message), 0);

	}
}

int main(int argc, char* argv[])
{
	WSADATA wsa;
	SOCKET s, new_socket;
	struct sockaddr_in server, client;
	int c;

	struct clientData client_struc;

	HANDLE hMutex = CreateMutex(NULL, FALSE, NULL);

	enum StatusClient status_client = NOT_AUTHORIZED;

	// Инициализация winsock
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		return 1;
	}

	printf("Initialised.\n");

	// Создание сокета
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
	}

	printf("Socket created.\n");

	// Заполнение полей структуры 
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(8888);

	// Привязка сокета к адресу
	if (bind(s, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	puts("Bind done");

	// Прослушивание входящих соединений. 3 - размер очереди
	listen(s, 3);

	// Прием подключений
	puts("Waiting for incoming connections...");

	c = sizeof(struct sockaddr_in);

	while ((new_socket = accept(s, (struct sockaddr*)&client, &c)) != INVALID_SOCKET)
	{

		// Заполнение структуры клиента
		client_struc.status_client = status_client;
		client_struc.client_socket = new_socket;
		client_struc.mutex = &hMutex;

		// Вынос обработки клиента в отдельный поток
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)processingClient, &client_struc, 0, NULL);
		
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