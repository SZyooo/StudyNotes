#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <thread>
#include <vector>

#define BUFFER_SIZE 2048
#define BUFF_SMALL 100


void send_error_msg(SOCKET socket)
{
	char protocol[] = "HTTP/1.0 400 Bad Request\r\n";
	char serv_name[] = "Server : Simple web server\r\n";
	char cnt_len[] = "Content-Length : 2048\r\n";
	char cnt_type[] = "Content-Type : text/html\r\n\r\n";
	char content[] = "<html><head><title>NETWORK</title></head><body><h1>400 Bad Request</h1></body></html>";
	send(socket, protocol, sizeof(protocol), 0);
	send(socket, serv_name, sizeof(serv_name), 0);
	send(socket, cnt_len, sizeof(cnt_len), 0);
	send(socket, cnt_type, sizeof(cnt_type), 0);
	send(socket, content, sizeof(content), 0);
	closesocket(socket);
}

char* content_type(char* file)
{
	char extension[BUFF_SMALL];
	char file_name[BUFF_SMALL];
	strcpy(file_name, file);
	strtok(file_name, ".");
	strcpy(extension, strtok(NULL, "."));
	if(!strcmp(extension, "html") || !strcmp(extension, "htm"))
		return "text/html";
	else
		return "text/plain";
}

void send_data(SOCKET client_socket, char* ct, char* file_name)
{
	char protocol[] = "HTTP/1.0 200 OK\r\n";
	char serv_name[] = "Server:Simple web server\r\n";
	char cnt_len[BUFF_SMALL];// "Content-length:2048\r\n";
	char cnt_type[BUFF_SMALL];
	char buf[BUFFER_SIZE] = {'\0'};
	FILE* send_file;
	sprintf(cnt_type, "Content-type:%s\r\n\r\n", ct);
	if((send_file = fopen(file_name, "rb")) == NULL)
	{
		send_error_msg(client_socket);
		return;
	}
	fseek(send_file, 0, SEEK_END);
	int sz = ftell(send_file);
	fseek(send_file, 0, SEEK_SET);
	sprintf(cnt_len, "Content-length:%d\r\n", sz);
	send(client_socket, protocol, strlen(protocol), 0);
	send(client_socket, serv_name, strlen(serv_name), 0);
	send(client_socket, cnt_len, strlen(cnt_len), 0);
	send(client_socket, cnt_type, strlen(cnt_type), 0);
	size_t n = 0;
	while((n = fread(buf, 1, BUFFER_SIZE, send_file) != 0))
	{
		send(client_socket, buf, strlen(buf), 0);
		//std::this_thread::sleep_for(std::chrono::seconds(1));
		memset(buf, 0, strlen(buf));
	}
	fclose(send_file);
	closesocket(client_socket);
}

void request_handler(SOCKET client_socket)
{
	char buf[BUFFER_SIZE];
	char method[BUFF_SMALL];
	char ct[BUFF_SMALL];
	char file_name[BUFF_SMALL];
	recv(client_socket, buf, sizeof(buf), 0);
	if(strstr(buf, "HTTP/") == NULL)
	{
		send_error_msg(client_socket);
		closesocket(client_socket);
		return;
	}
	strcpy(method, strtok(buf, " /"));
	if(strcmp(method, "GET") != 0)
	{
		send_error_msg(client_socket);
		closesocket(client_socket);
		return;
	}
	strcpy(file_name, strtok(NULL, " /"));
	strcpy(ct, content_type(file_name));
	send_data(client_socket, ct, file_name);
}

void error_handling(char* msg)
{
	fprintf(stderr, "%s\n", msg);
	exit(1);
}

int main(int argc, char* argv[])
{
	WSADATA wsa_data;
	SOCKET server_socket, client_socket;
	SOCKADDR_IN server_addr, client_addr;
	
	int client_addr_size;
	if (argc != 2)
	{
		printf("Usage: %s <port>\n", argv[0]);
		exit(1);
	}
	if(WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
		error_handling("WSAStartup() error");
	server_socket = socket(PF_INET, SOCK_STREAM, 0);
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(atoi(argv[1]));
	if(bind(server_socket, (SOCKADDR*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
		error_handling("bind() error");
	if (listen(server_socket, 5) == SOCKET_ERROR)
		error_handling("listen() error");
	
	std::vector<std::thread> threads;
	while (1)
	{
		client_addr_size = sizeof(client_addr);
		client_socket = accept(server_socket, (SOCKADDR*)&client_addr, &client_addr_size);
		if (client_socket == INVALID_SOCKET)
			error_handling("accept() error");
		printf("Connection Request: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
		threads.emplace_back(request_handler, client_socket);
	}
	for(auto& t : threads)
		t.join();
}