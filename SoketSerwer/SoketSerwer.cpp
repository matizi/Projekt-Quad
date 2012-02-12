

// prosty modu³ serwera
// komunikacji za pomoc¹ Winsock
// u¿ycie: server.exe

#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include "stdafx.h"

#define DEFAULT_PORT 5000
#define DEFAULT_BUFFER 4096

// tylko Visual C++
#pragma comment(lib, "ws2_32.lib")

// funkcja: w¹tek do komunikacji z klientem
DWORD WINAPI ClientThread(LPVOID lpParam)
{
    SOCKET sock = (SOCKET)lpParam;
    char szBuf[DEFAULT_BUFFER];
    int ret,
        nLeft,
        idx;

	// serwer bêdzie oczekiwa³ na informacje od klienta
	while(1)
	{
		// najpierw odbierz dane 
		ret = recv(sock, szBuf, DEFAULT_BUFFER, 0);
		if (ret == 0)
			break; 
		else if (ret == SOCKET_ERROR)
		{
			printf("b³¹d funkcji recv(): %d\n", WSAGetLastError());
			break; 
		} 

		szBuf[ret] = '\0';
		printf("RECV: '%s'\n", szBuf);

		// nastêpnie odeœlij te dane, poporcjuj jeœli trzeba
		// (niestety send() mo¿e nie wys³aæ wszystkiego)
		nLeft = ret;
		idx = 0;
		while(nLeft > 0)
		{
			ret = send(sock, &szBuf[idx], nLeft, 0);
			if (ret == 0)
				break;
			else if (ret == SOCKET_ERROR)
			{
				printf("b³¹d funkcji send(): %d\n", WSAGetLastError());
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
    printf("B³¹d ³adowania Winsock 2.2!\n");
    return 1;
}

// twórz gniazdo do nas³uchu po³¹czeñ klientów
sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
if (sListen == SOCKET_ERROR) 
{
    printf("B³¹d funkcji socket(): %d\n", WSAGetLastError());
    return 1;
}

// wybierz interfejs (warstwê komunikacyjn¹)
local.sin_addr.s_addr = htonl(INADDR_ANY);
local.sin_family = AF_INET;
local.sin_port = htons(DEFAULT_PORT); 
if (bind(sListen, (struct sockaddr *)&local, sizeof(local)) == SOCKET_ERROR)
{
    printf("B³¹d funkcji bind(): %d\n", WSAGetLastError());
    return 1;
}


// nas³uch
host = gethostbyname("localhost");
if (host == NULL)
{
    printf("Nie uda³o siê wydobyæ nazwy serwera\n");
    return 1;
}

listen(sListen, 2);
printf("Serwer nas³uchuje.\n");
printf("Adres: %s, port: %d\n", host->h_name, DEFAULT_PORT);

// akceptuj nadchodz¹ce po³¹czenia
while (1)
{
    iAddrSize = sizeof(client);
    sClient = accept(sListen, (struct sockaddr *)&client, &iAddrSize);
    if (sClient == INVALID_SOCKET)
    {
        printf("B³¹d funkcji accept(): %d\n", WSAGetLastError());
        return 1;
    }
    printf("Zaakceptowano po³¹czenie: serwer %s, port %d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
    hThread = CreateThread(NULL, 0, ClientThread, (LPVOID)sClient, 0, &dwThreadID);
    if (hThread == NULL)
    {
        printf("B³¹d funkcji CreateThread(): %d\n", WSAGetLastError());
        return 1;
    } 
    CloseHandle(hThread); 
} 
closesocket(sListen);
 
WSACleanup();
system("pause");
return 0;
}