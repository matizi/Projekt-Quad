

// prosty modu� serwera
// komunikacji za pomoc� Winsock
// u�ycie: server.exe

#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include "stdafx.h"

#define DEFAULT_PORT 5000
#define DEFAULT_BUFFER 4096

// tylko Visual C++
#pragma comment(lib, "ws2_32.lib")

// funkcja: w�tek do komunikacji z klientem
DWORD WINAPI ClientThread(LPVOID lpParam)
{
    SOCKET sock = (SOCKET)lpParam;
    char szBuf[DEFAULT_BUFFER];
    int ret,
        nLeft,
        idx;

	// serwer b�dzie oczekiwa� na informacje od klienta
	while(1)
	{
		// najpierw odbierz dane 
		ret = recv(sock, szBuf, DEFAULT_BUFFER, 0);
		if (ret == 0)
			break; 
		else if (ret == SOCKET_ERROR)
		{
			printf("b��d funkcji recv(): %d\n", WSAGetLastError());
			break; 
		} 

		szBuf[ret] = '\0';
		printf("RECV: '%s'\n", szBuf);

		// nast�pnie ode�lij te dane, poporcjuj je�li trzeba
		// (niestety send() mo�e nie wys�a� wszystkiego)
		nLeft = ret;
		idx = 0;
		while(nLeft > 0)
		{
			ret = send(sock, &szBuf[idx], nLeft, 0);
			if (ret == 0)
				break;
			else if (ret == SOCKET_ERROR)
			{
				printf("b��d funkcji send(): %d\n", WSAGetLastError());
				break; 
			} 
			nLeft -= ret;
			idx += ret;
		}
	} 
	return 0; 
}


int _tmain(int argc, char *argv[])
{
    WSADATA wsd;
    SOCKET sListen,
    sClient;
    int iAddrSize;
    HANDLE hThread;
    DWORD dwThreadID;
    struct sockaddr_in local, client; 
    struct hostent *host = NULL;

// inicjuj Winsock 2.2 
if (WSAStartup(MAKEWORD(2,2), &wsd) != 0)
{
    printf("B��d �adowania Winsock 2.2!\n");
    return 1;
}

// tw�rz gniazdo do nas�uchu po��cze� klient�w
sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
if (sListen == SOCKET_ERROR) 
{
    printf("B��d funkcji socket(): %d\n", WSAGetLastError());
    return 1;
}

// wybierz interfejs (warstw� komunikacyjn�)
local.sin_addr.s_addr = htonl(INADDR_ANY);
local.sin_family = AF_INET;
local.sin_port = htons(DEFAULT_PORT); 
if (bind(sListen, (struct sockaddr *)&local, sizeof(local)) == SOCKET_ERROR)
{
    printf("B��d funkcji bind(): %d\n", WSAGetLastError());
    return 1;
}


// nas�uch
host = gethostbyname("localhost");
if (host == NULL)
{
    printf("Nie uda�o si� wydoby� nazwy serwera\n");
    return 1;
}

listen(sListen, 2);
printf("Serwer nas�uchuje.\n");
printf("Adres: %s, port: %d\n", host->h_name, DEFAULT_PORT);

// akceptuj nadchodz�ce po��czenia
while (1)
{
    iAddrSize = sizeof(client);
    sClient = accept(sListen, (struct sockaddr *)&client, &iAddrSize);
    if (sClient == INVALID_SOCKET)
    {
        printf("B��d funkcji accept(): %d\n", WSAGetLastError());
        return 1;
    }
    printf("Zaakceptowano po��czenie: serwer %s, port %d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
    hThread = CreateThread(NULL, 0, ClientThread, (LPVOID)sClient, 0, &dwThreadID);
    if (hThread == NULL)
    {
        printf("B��d funkcji CreateThread(): %d\n", WSAGetLastError());
        return 1;
    } 
    CloseHandle(hThread); 
} 
closesocket(sListen);
 
WSACleanup();
system("pause");
return 0;
}