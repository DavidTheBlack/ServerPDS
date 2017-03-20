#include "stdafx.h"
#define WIN32_MEAN_AND_LEAN
#include <WinSock2.h>
#include <Windows.h>
#include <ws2tcpip.h>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "4444"

const int REQ_WINSOCK_VER = 2;


bool InitializeServer(int portNumber) {
	
	SOCKET hSocket = INVALID_SOCKET; //socket to listen for new connections
	SOCKET hClientSocket = INVALID_SOCKET; // socket to give to the clients
	
	//address info for the server to listen to
	struct addrinfo *result = NULL;
	struct addrinfo hints;
	int iResult; // for error checking return values
	
	// set address information
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;    // TCP connection
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return false;
	}

	// Create a SOCKET for connecting to server
    if (hSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol) == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return false;
		}
	// Setup the TCP listening socket
	iResult = bind(hSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(hSocket);
		WSACleanup();
		return false;
	}
	
	// no longer need address information: freeaddrinfo frees address information that the getaddrinfo function dynamically allocates in addrinfo structures.
	freeaddrinfo(result);
	
	// start listening for new clients attempting to connect
	iResult = listen(hSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(hSocket);
		WSACleanup();
		return false;
	}

	return true;
}

bool InitializeWinsock() {
	
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	std::cout << "Initializing winsock... ";
	/* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
	wVersionRequested = MAKEWORD(2, 2);

	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		/* Tell the user that we could not find a usable */
		/* Winsock DLL.                                  */
		printf("WSAStartup failed with error: %d\n", err);
		return false;
	}

	/* Confirm that the WinSock DLL supports 2.2.*/
	/* Note that if the DLL supports versions greater    */
	/* than 2.2 in addition to 2.2, it will still return */
	/* 2.2 in wVersion since that is the version we      */
	/* requested.                                        */

	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
		/* Tell the user that we could not find a usable */
		/* WinSock DLL.                                  */
		printf("Could not find a usable version of Winsock.dll\n");
		WSACleanup();
		return false;
	}
	else
		printf("The Winsock 2.2 dll was found okay\n");

return true;
}


int main(int argc, char *argv[]) {
	
	InitializeWinsock();
	std::cin.get();
	return 0;
}