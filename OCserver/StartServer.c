#include<io.h>
#include<stdio.h>
#include<winsock2.h>

#include <windows.h>

#pragma comment(lib,"ws2_32.lib")

struct clientData *online_clients = NULL;
size_t number_online_clients = 0;
unsigned int thread_id = 0;

// Структура данных клиента. Передается в поток.
struct clientData{
	unsigned int id;
	SOCKET client_socket;
	const HANDLE *mutex;
};

// Добавление клиента в массив онлайн пользователей
void addClient(const struct clientData *data) {
	number_online_clients++;
	SOCKET* temp = realloc(online_clients, number_online_clients * sizeof(struct clientData));
	if (temp == NULL) {
		printf("Ошибка: Не удалось выделить память\n");
		return;
	}
	online_clients = temp;
	online_clients[number_online_clients - 1] = *data; // Добавляем нового клиента в конец массива
}

void delClient(const struct clientData *data) {
	// Поиск индекса пользователя по сокету
	int index = -1;
	for (int i = 0; i < number_online_clients; i++) {
		if (online_clients[i].id == data->id) {
			index = i;
			break;
		}
	}

	if (index == -1) {
		// Добавить логирования потом здесь
		printf("Ошибка: Пользователь не найден\n");
		return;
	}

	// Освобождение памяти для удаленного пользователя
	for (int i = index; i < number_online_clients - 1; i++) {
		online_clients[i] = online_clients[i + 1]; // Сдвигаем элементы массива
	}
	number_online_clients--; // Уменьшаем количество пользователей
	online_clients = realloc(online_clients, number_online_clients * sizeof(struct clientData)); // Уменьшаем размер массива
	printf("%d", (int)number_online_clients);
}

void processingClient(struct clientData *data) {
	char* message, client_reply[2000];
	int recv_size;

	SOCKET client_socket = data->client_socket;

	puts("Connection accepted");

	WaitForSingleObject(data->mutex, INFINITE);
	addClient(data);
	ReleaseMutex(data->mutex);

	while (TRUE) {
		if ((recv_size = recv(client_socket, client_reply, 2000, 0)) == SOCKET_ERROR)
		{
			puts("recv failed");
		}

		puts("Reply received\n");

		// Вывод принятого сообщения в консоль
		if (recv_size >= 0 && recv_size < 2000) {
			client_reply[recv_size] = '\0';
			puts(client_reply);
		}
		else {
			// Обработка ошибки или прерывания соединения
			// Например:
			puts("Error: Invalid recv_size or buffer overflow occurred");


			WaitForSingleObject(data->mutex, INFINITE);
			delClient(data);
			ReleaseMutex(data->mutex);

			closesocket(client_socket);
			ExitThread(0);
			return 1;
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

	const HANDLE hMutex = CreateMutex(NULL, FALSE, NULL);

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
		thread_id++;
		client_struc.id = thread_id;
		client_struc.client_socket = new_socket;
		client_struc.mutex = &hMutex;

		// Вынос обработки клиента в отдельный поток
		CreateThread(NULL, 0, processingClient, &client_struc, 0, NULL);
		
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