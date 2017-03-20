#include "stdafx.h"
#define WIN32_MEAN_AND_LEAN
#include <WinSock2.h>
#include <Windows.h>
#include <ws2tcpip.h>
#include <iostream>
#define DEFAULT_BUFLEN 512

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "4444"

const int REQ_WINSOCK_VER = 2;


bool InitializeServer(char *portNumber) {
	
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;


	SOCKET hSocket = INVALID_SOCKET; //socket to listen for new connections
	SOCKET hClientSocket = INVALID_SOCKET; // socket to give to the clients
	
	//address info for the server to listen to
	struct addrinfo *result = NULL;
	struct addrinfo hints;
	int iResult; // for error checking return values
	int iSendResult;
	
	// set address information
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;    // TCP connection
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, portNumber, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return false;
	}

	// Create a SOCKET for connecting to server
	hSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (hSocket == INVALID_SOCKET) {
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

	//Aggiunti il 2017/03/20

	// Accept a client socket
	hClientSocket = accept(hSocket, NULL, NULL);
	if (hClientSocket == INVALID_SOCKET) {
		printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(hSocket);
		WSACleanup();
		return 1;
	}

	// No longer need server socket
	closesocket(hSocket);

	// Receive until the peer shuts down the connection
	do {

		iResult = recv(hClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			printf("Bytes received: %d\n", iResult);

			// Echo the buffer back to the sender
			iSendResult = send(hClientSocket, recvbuf, iResult, 0);
			if (iSendResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(hClientSocket);
				WSACleanup();
				return 1;
			}
			printf("Bytes sent: %d\n", iSendResult);
		}
		else if (iResult == 0)
			printf("Connection closing...\n");
		else {
			printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(hClientSocket);
			WSACleanup();
			return 1;
		}

	} while (iResult > 0);

	// shutdown the connection since we're done
	iResult = shutdown(hClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(hClientSocket);
		WSACleanup();
		return 1;
	}

	// cleanup
	closesocket(hClientSocket);
	WSACleanup();

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

//	char recvbuf[DEFAULT_BUFLEN];
//	int recvbuflen = DEFAULT_BUFLEN;
//
//
//	SOCKET hSocket = INVALID_SOCKET; //socket to listen for new connections
//	SOCKET hClientSocket = INVALID_SOCKET; // socket to give to the clients
//
//										   //address info for the server to listen to
//	struct addrinfo *result = NULL;
//	struct addrinfo hints;
//	int iResult; // for error checking return values
//	int iSendResult;
//
//	// set address information
//	ZeroMemory(&hints, sizeof(hints));
//	hints.ai_family = AF_INET;
//	hints.ai_socktype = SOCK_STREAM;
//	hints.ai_protocol = IPPROTO_TCP;    // TCP connection
//	hints.ai_flags = AI_PASSIVE;
//
//	// Resolve the server address and port
//	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
//	if (iResult != 0) {
//		printf("getaddrinfo failed with error: %d\n", iResult);
//		WSACleanup();
//		return false;
//	}
//
//	// Create a SOCKET for connecting to server
//	hSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
//	if (hSocket == INVALID_SOCKET) {
//		printf("socket failed with error: %ld\n", WSAGetLastError());
//		freeaddrinfo(result);
//		WSACleanup();
//		return false;
//	}
//
//	// Setup the TCP listening socket
//	iResult = bind(hSocket, result->ai_addr, (int)result->ai_addrlen);
//	if (iResult == SOCKET_ERROR) {
//		printf("bind failed with error: %d\n", WSAGetLastError());
//		freeaddrinfo(result);
//		closesocket(hSocket);
//		WSACleanup();
//		return false;
//	}
//
//	// no longer need address information: freeaddrinfo frees address information that the getaddrinfo function dynamically allocates in addrinfo structures.
//	freeaddrinfo(result);
//
//	// start listening for new clients attempting to connect
//	iResult = listen(hSocket, SOMAXCONN);
//	if (iResult == SOCKET_ERROR) {
//		printf("listen failed with error: %d\n", WSAGetLastError());
//		closesocket(hSocket);
//		WSACleanup();
//		return false;
//	}
//
//	//Aggiunti il 2017/03/20
//
//	// Accept a client socket
//	hClientSocket = accept(hSocket, NULL, NULL);
//	if (hClientSocket == INVALID_SOCKET) {
//		printf("accept failed with error: %d\n", WSAGetLastError());
//		closesocket(hSocket);
//		WSACleanup();
//		return 1;
//	}
//
//	// No longer need server socket
//	closesocket(hSocket);
//
//	// Receive until the peer shuts down the connection
//	do {
//
//		iResult = recv(hClientSocket, recvbuf, recvbuflen, 0);
//		if (iResult > 0) {
//			printf("Bytes received: %d\n", iResult);
//
//			// Echo the buffer back to the sender
//			iSendResult = send(hClientSocket, recvbuf, iResult, 0);
//			if (iSendResult == SOCKET_ERROR) {
//				printf("send failed with error: %d\n", WSAGetLastError());
//				closesocket(hClientSocket);
//				WSACleanup();
//				return 1;
//			}
//			printf("Bytes sent: %d\n", iSendResult);
//		}
//		else if (iResult == 0)
//			printf("Connection closing...\n");
//		else {
//			printf("recv failed with error: %d\n", WSAGetLastError());
//			closesocket(hClientSocket);
//			WSACleanup();
//			return 1;
//		}
//
//	} while (iResult > 0);
//
//	// shutdown the connection since we're done
//	iResult = shutdown(hClientSocket, SD_SEND);
//	if (iResult == SOCKET_ERROR) {
//		printf("shutdown failed with error: %d\n", WSAGetLastError());
//		closesocket(hClientSocket);
//		WSACleanup();
//		return 1;
//	}
//
//	// cleanup
//	closesocket(hClientSocket);
//	WSACleanup();
//
//
//
//
//
//
//
//return true;
}


int main(int argc, char *argv[]) {
	
	InitializeWinsock();
	InitializeServer(DEFAULT_PORT);
	std::cin.get();
	return 0;
}