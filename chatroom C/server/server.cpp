#include "stdafx.h"
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <process.h>
#include <Windows.h>
#include <time.h>

#define DEFAULT_PORT 5019

char temp[255];
int msg_len;
int addr_len;
struct sockaddr_in local, client_addr;
//count variable is the sum of people in the chat room
int count = 0;
//create a array to store clients' socket
int confd[50];
FILE *fp;
HANDLE hThread1;
// use two dimension array to store client's name and their index
char name[50][100] = {};

SOCKET sock, msg_sock;
WSADATA wsaData;

DWORD WINAPI receive_other(LPVOID arg)
{
	char buf_recive[100];
	char buf_send[100];
	int index = (int)arg;

	// recive client's username
	msg_len = recv(confd[index], name[index], sizeof(name[index]), 0);

	//error message
	if (msg_len == SOCKET_ERROR)
	{
		fprintf(stderr, "recv() failed with error %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}
	if (msg_len == 0)
	{
		printf("server closed connection\n");
		closesocket(sock);
		WSACleanup();
		return -1;
	}


	//Welcome message
	printf("Welcome \"%s\" to the chat room.\n", name[index]);
	sprintf(temp, "Welcome \"%s\" to the chat room.\n", name[index]);
	msg_len = recv(confd[index], temp, sizeof(temp), 0);
	//error message
	if (msg_len == SOCKET_ERROR)
	{
		fprintf(stderr, "recv() failed with error %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}
	if (msg_len == 0)
	{
		printf("server closed connection\n");
		closesocket(sock);
		WSACleanup();
		return -1;
	}
	fprintf(fp,"Welcome \"%s\" to the chat room.\n", name[index]);



	while(1)
	{
		//recive client's message
		msg_len = recv(confd[index], buf_recive, sizeof(buf_recive), 0);
		//error message
		if (msg_len == SOCKET_ERROR)
		{
			fprintf(stderr, "recv() failed with error %d\n", WSAGetLastError());
			WSACleanup();
			return -1;
		}
		if (msg_len == 0)
		{
			printf("server closed connection\n");
			closesocket(sock);
			WSACleanup();
			return -1;
		}


		//if client's message is not empty or username is not empty
		if(strcmp(buf_recive,"") != 0 && strcmp(buf_recive,"/private") != 0)
		{
			//print the message in the server panel
			printf("%s: %s\n",name[index], buf_recive);

			//write the message into log file
			sprintf(temp, "%s: %s\n", name[index], buf_recive);
			fprintf(fp,"%s",temp);

			//send message to other clients
			for(int i = 0; i <= count; i++)
			{
				//if index is myself, continue
				if(index == i || confd[i] == 0)
					continue;
				if(strcmp(buf_recive,"") != 0)
				{
					msg_len = send(confd[i], temp, sizeof(temp), 0);
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
						closesocket(sock);
						WSACleanup();
						return -1;
					}
				}
			}
		}


		//if client want send private message to someone
		if(strcmp(buf_recive,"/private") == 0)
		{
			char name2[100];
			msg_len = recv(confd[index], name2, sizeof(name2), 0);
			//error message
			if (msg_len == SOCKET_ERROR)
			{
				fprintf(stderr, "recv() failed with error %d\n", WSAGetLastError());
				WSACleanup();
				return -1;
			}
			if (msg_len == 0)
			{
				printf("server closed connection\n");
				closesocket(sock);
				WSACleanup();
				return -1;
			}


			//check the user is exist or not
			int check = -1;
			for(int i = 0; i < count; i++)
			{
				if(strcmp(name[i],name2) == 0)
				{
					check = i;
					break;
				}
			}
			if(check == -1)
			{
				printf("There is no client called  \"%s\".\n", name2);
				sprintf(temp, "There is no client called  \"%s\".\n", name2);

				msg_len = send(confd[index], temp, sizeof(temp), 0);
				//error message
				if (msg_len == SOCKET_ERROR)
				{
					fprintf(stderr, "recv() failed with error %d\n", WSAGetLastError());
					WSACleanup();
					return -1;
				}
				if (msg_len == 0)
				{
					printf("server closed connection\n");
					closesocket(sock);
					WSACleanup();
					return -1;
				}
				fprintf(fp, "There is no client called  \"%s\".\n", name2);
				//continue;
			}


			// client can type '/exit private' to exit private chat 
			while(1)
			{
				//recive client's message
				msg_len = recv(confd[index], buf_recive, sizeof(buf_recive), 0);
				//error message
				if (msg_len == SOCKET_ERROR)
				{
					fprintf(stderr, "recv() failed with error %d\n", WSAGetLastError());
					WSACleanup();
					return -1;
				}
				if (msg_len == 0)
				{
					printf("server closed connection\n");
					closesocket(sock);
					WSACleanup();
					return -1;
				}

				//if client want to exit private chat and return the public chat room
				if(strcmp(buf_recive,"/exit private") == 0)
					break;

				printf("Private message from %s to %s: %s\n", name[index], name2, buf_recive);
				sprintf(temp, "Private message from %s to you: %s\n", name[index], buf_recive);
				fprintf(fp, "Private message from %s to %s: %s\n", name[index], name2, buf_recive);

				for(int i = 0; i < count; i++)
				{
					if(strcmp(name[i],name2) == 0)
					{
						msg_len = send(confd[i], temp, sizeof(temp), 0);
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
							closesocket(sock);
							WSACleanup();
							return -1;
						}
					}
				}
			}
		}

		//if client type 'bye'
		if(strcmp(buf_recive,"bye") == 0)
		{
			char temp[255];
			//print the message in the server panel
			printf("\"%s\" have quit the chat room.\n", name[index]);

			//write the message into log file
			sprintf(temp, "\"%s\" have quit the chat room.\n", name[index]);
			fprintf(fp,"%s",temp);
			
			//decrease count
			count--;
			confd[index] = NULL;

			//write the message into log file
			printf("( Current number of chat rooms is: %d )\n", count);
			fprintf(fp,"( Current number of chat rooms is: %d )\n", count);
			break;
		}
	}
	return 0;
}

int main(int argc, char **argv)
{
	if (WSAStartup(0x202, &wsaData) == SOCKET_ERROR)
	{
		// stderr: standard error are printed to the screen.
		fprintf(stderr, "WSAStartup failed with error %d\n", WSAGetLastError());
		//WSACleanup function terminates use of the Windows Sockets DLL. 
		WSACleanup();
		return -1;
	}
	// Fill in the address structure
	local.sin_family		= AF_INET;
	local.sin_addr.s_addr	= INADDR_ANY;
	local.sin_port		= htons(DEFAULT_PORT);

	sock = socket(AF_INET,SOCK_STREAM, 0);	//TCP socket


	if(sock == INVALID_SOCKET)
	{
		fprintf(stderr, "socket() failed with error %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}

	if(bind(sock, (struct sockaddr *)&local, sizeof(local)) == SOCKET_ERROR)
	{
		fprintf(stderr, "bind() failed with error %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}

	//waiting for the connections
	if(listen(sock, 50) == SOCKET_ERROR)
	{
		fprintf(stderr, "listen() failed with error %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}

	printf("Waiting for the connections ........\n");

	addr_len = sizeof(client_addr);

	int index = 0;

	//create a log file to store chat records
	fp = fopen("log.txt","w+");

	//record the local time
	char *wday[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    time_t timep;
    struct tm *p;
    time(&timep);
    p = localtime(&timep);
    fprintf (fp, "%d/%d/%d ", (1900+p->tm_year), (p->tm_mon), p->tm_mday);
    fprintf(fp, "%s %d:%d:%d\n", wday[p->tm_wday], p->tm_hour, p->tm_min, p->tm_sec);
	fprintf(fp,"Chatting records:\n");

	//if people in chat room is less than 50
	while(count <= 50)
	{
		confd[count] = accept(sock, (struct sockaddr*)&client_addr, &addr_len);

		//error message
		if(msg_sock == INVALID_SOCKET)
		{
			fprintf(stderr, "accept() failed with error %d\n", WSAGetLastError());
			WSACleanup();
			return -1;
		}

		//print the connect success information
		printf("Accepted connection from %s, port %d\n",
			inet_ntoa(client_addr.sin_addr),
			htons(client_addr.sin_port));

		//write the message into log file
		fprintf(fp,"Accepted connection from %s, port %d\n",
			inet_ntoa(client_addr.sin_addr),
			htons(client_addr.sin_port));
		
		//increase the count
		++count;
		index = count - 1;
		
		//create a new child thread to receive cliends message
		hThread1 = CreateThread(NULL, 0, receive_other, (LPVOID)index, 0, NULL);

		//print the message in the server panel
		printf("( Current number of chat rooms is: %d )\n", count);
		//write the message into log file
		fprintf(fp,"( Current number of chat rooms is: %d )\n", count);

	}
	//close the file point
	fclose(fp);
	//terminate the child thread
	TerminateThread(hThread1,0);
	closesocket(sock);
	WSACleanup();
}