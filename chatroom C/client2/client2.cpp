#include "stdafx.h"
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define DEFAULT_PORT 5019

char buf_send[100];
char buf_recive[100];
int msg_len;
struct sockaddr_in server_addr;
struct hostent *hp;
SOCKET connect_sock;
WSADATA wsaData;

DWORD WINAPI receive_other(void *arg)
{
	while(1)
	{
		//recive message send from server
		msg_len = recv(connect_sock, buf_recive, sizeof(buf_recive), 0);
		if (msg_len == SOCKET_ERROR)
		{
			fprintf(stderr, "recv() failed with error %d\n", WSAGetLastError());
			closesocket(connect_sock);
			WSACleanup();
			return -1;
		}
		if (msg_len == 0)
		{
			printf("server closed connection\n");
			closesocket(connect_sock);
			WSACleanup();
			return -1;
		}

		//print the message in the server panel
		printf("%s", buf_recive);
	}
	return 0;
}

int main(int argc, char **argv)
{
	char   *server_name = "localhost";
	unsigned short port = DEFAULT_PORT;
	unsigned int addr;

	if (WSAStartup(0x202, &wsaData) == SOCKET_ERROR)
	{
		// stderr: standard error are printed to the screen.
		fprintf(stderr, "WSAStartup failed with error %d\n", WSAGetLastError());
		//WSACleanup function terminates use of the Windows Sockets DLL. 
		WSACleanup();
		return -1;
	}

	if (isalpha(server_name[0]))
		hp = gethostbyname(server_name);
	else
	{
		addr = inet_addr(server_name);
		hp = gethostbyaddr((char*)&addr, 4, AF_INET);
	}

	if (hp==NULL)
	{
		fprintf(stderr, "Cannot resolve address: %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}

	//copy the resolved information into the sockaddr_in structure
	memset(&server_addr, 0, sizeof(server_addr));
	memcpy(&(server_addr.sin_addr), hp->h_addr, hp->h_length);
	server_addr.sin_family = hp->h_addrtype;
	server_addr.sin_port = htons(port);

	connect_sock = socket(AF_INET,SOCK_STREAM, 0); //TCP socket

	if (connect_sock == INVALID_SOCKET)
	{
		fprintf(stderr, "socket() failed with error %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}

	printf("Client connecting to: %s\n", hp->h_name);

	if (connect(connect_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) 
		== SOCKET_ERROR)
	{
		fprintf(stderr, "connect() failed with error %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}
	
	//send username to server
	char name[100] = {};
	printf("Please input your usename: ");
	scanf("%s", name);
	msg_len = send(connect_sock, name, sizeof(name), 0);
	//error message
	if (msg_len == SOCKET_ERROR)
	{
		fprintf(stderr, "send() failed with error %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}
	if (msg_len == 0)
	{
		printf("server closed connection\n");
		closesocket(connect_sock);
		WSACleanup();
		return -1;
	}

	printf("Welcome to the chat room!\n");
	
	//create a new child thread to receive cliends message
	HANDLE hThread1 = CreateThread(NULL, 0, receive_other, NULL, 0, NULL);

	//if client do not type 'bye'
	while(1)
	{
		//get client's message
		gets(buf_send);
		msg_len = send(connect_sock, buf_send, sizeof(buf_send), 0);
		//error message
		if (msg_len == SOCKET_ERROR)
		{
			fprintf(stderr, "send() failed with error %d\n", WSAGetLastError());
			WSACleanup();
			return -1;
		}
		if (msg_len == 0)
		{
			printf("server closed connection\n");
			closesocket(connect_sock);
			WSACleanup();
			return -1;
		}

		//if client type 'bye', then terminate child thread.
		if(strcmp(buf_send,"bye") == 0)
		{
			TerminateThread(hThread1,0);
			printf("You have quit the chat room.\n");
			break;
		}

		if(strcmp(buf_send,"/private") == 0)
		{
			char name2[100];
			printf("Please enter the username you want to talk to privately: ");
			gets(name2);
			msg_len = send(connect_sock, name2, sizeof(name2), 0);
			//error message
			if (msg_len == SOCKET_ERROR)
			{
				fprintf(stderr, "send() failed with error %d\n", WSAGetLastError());
				WSACleanup();
				return -1;
			}
			if (msg_len == 0)
			{
				printf("server closed connection\n");
				closesocket(connect_sock);
				WSACleanup();
				return -1;
			}
		}
	}
	closesocket(connect_sock);
	WSACleanup();
}