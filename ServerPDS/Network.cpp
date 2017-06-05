#include <WinSock2.h>
#include <ws2tcpip.h>
#include <strsafe.h>
#include <iostream>
#include <string>
#include <algorithm>
#include "Network.h"
#pragma comment(lib, "Ws2_32.lib")




	
	Network::Network():iResult(0), hSocket(INVALID_SOCKET), hClientSocket(INVALID_SOCKET) {
		//Windows Mail Slot initialization
		Slot = TEXT("\\\\.\\mailslot\\ms1");
		hSlot = CreateFile(Slot, GENERIC_WRITE,
			FILE_SHARE_READ,
			(LPSECURITY_ATTRIBUTES)NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
			(HANDLE)NULL);
	}

	Network::~Network() {

		iResult = shutdown(hClientSocket, SD_SEND);
		if (iResult == SOCKET_ERROR) {
			errorShow(TEXT("shutdown"));
			closesocket(hClientSocket);
			WSACleanup();
		}

		// cleanup
		closesocket(hClientSocket);
		WSACleanup();
	}

	//Initialize the socket and start listening for connection
	bool Network::initNetwork(PCSTR portNumber)
	{
		if (!startWinsock(2, 2)) return false;
		if (!startSocket(portNumber)) return false;
		if (!acceptClient()) return false;
		
		return true;
	}	

	bool Network::startWinsock(BYTE majorVersion, BYTE minorVersion) {

		WSADATA wsaData;
		std::cout << "Initializing winsock... ";
		/* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
		WORD wVersionRequested = MAKEWORD(minorVersion, majorVersion);
		iResult = WSAStartup(wVersionRequested, &wsaData);
		if (iResult != 0) {
			/* Tell the user that we could not find a usable */
			/* Winsock DLL.                                  */
			std::cout << "WSAStartup failed with error: " << iResult;
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
			std::cout << "Could not find a usable version of Winsock.dll\n";
			WSACleanup();
			return false;
		}
		else
			std::cout << "The Winsock 2.2 dll was found okay\n";

		return true;
	}

	bool Network::startSocket(PCSTR portNumber) {

		struct addrinfo *result = NULL;
		struct addrinfo hints;

		// set address information
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;    // TCP connection
		hints.ai_flags = AI_PASSIVE;

		// Resolve the server address and port
		iResult = getaddrinfo(NULL, portNumber, &hints, &result); //the IP address portion of the socket address structure is set to INADDR_ANY for IPv4 addresses or IN6ADDR_ANY_INIT for IPv6 addresses.
		if (iResult != 0) {
			std::cout << "getaddrinfo failed with error: " << iResult;
			WSACleanup();
			return false;//non va più bene!
		}

		// Create a SOCKET for connecting to server
		hSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (hSocket == INVALID_SOCKET) {
			errorShow(TEXT("socket"));
			freeaddrinfo(result);
			WSACleanup();
			return false;
		}

		iResult = bind(hSocket, result->ai_addr, (int)result->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			errorShow(TEXT("bind"));
			freeaddrinfo(result);
			closesocket(hSocket);
			WSACleanup();
			return false;
		}

		// no longer need address information : freeaddrinfo frees address information that the getaddrinfo function dynamically allocates in addrinfo structures.
		freeaddrinfo(result);

		// start listening for new clients attempting to connect
		iResult = listen(hSocket, SOMAXCONN);
		if (iResult == SOCKET_ERROR) {
			errorShow(TEXT("listen"));
			closesocket(hSocket);
			WSACleanup();
			return false;
		}
		return true;
	}

	bool Network::acceptClient() {
		// Accept a client socket
		hClientSocket = accept(hSocket, NULL, NULL);
		if (hClientSocket == INVALID_SOCKET) {
			errorShow(TEXT("accept"));
			closesocket(hSocket);
			WSACleanup();
			return false;
		}
		std::cout << "Client connesso" << std::endl;
		//closesocket(hSocket);
		return true;
	}

	void Network::errorShow(LPTSTR lpszFunction)
	{
		// Retrieve the system error message for the last-error code
		LPVOID lpMsgBuf;
		LPVOID lpDisplayBuf;
		DWORD dw = GetLastError();

		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dw,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf,
			0, NULL);

		// Display the error message and exit the process

		lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
			(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
		StringCchPrintf((LPTSTR)lpDisplayBuf,
			LocalSize(lpDisplayBuf) / sizeof(TCHAR),
			TEXT("%s failed with error %d: %s"),
			lpszFunction, dw, lpMsgBuf);
		MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

		LocalFree(lpMsgBuf);
		LocalFree(lpDisplayBuf);
		//ExitProcess(dw);
	}

	//Method used to send message via messageslot
	bool Network::WriteSlot(HANDLE hSlot, LPCTSTR lpszMessage)
	{
		DWORD cbWritten;
		BOOL fResult = WriteFile(hSlot, lpszMessage,
			(DWORD)(lstrlen(lpszMessage) + 1) * sizeof(TCHAR),
			&cbWritten, (LPOVERLAPPED)NULL);
		if (!fResult)
		{
			return false;
		}
		return true;
	}

	//Method that receives network messages and communicates them to the controller
	void Network::receiveMessages()
	{		
		char recvbuf[DEFAULT_BUFLEN];
		int recvbuflen = DEFAULT_BUFLEN;
		int iSendResult;

		//Coda dei messaggi globali;
		std::string globalBuffer;

		//Messaggio correntemente analizzato 
		std::string currentMessage;

		// Receive until the peer shuts down the connection or send close connection message
		do {

			iResult = recv(hClientSocket, recvbuf, recvbuflen, 0);
			if (iResult > 0) {
				
				std::string receivedMex= std::string(recvbuf);

				//Accolo i messaggi ricevuti a quelli globali
				globalBuffer = globalBuffer + receivedMex;

				//ANALISI DEI MESSAGGI RICEVUTI								
				int headerPos = globalBuffer.find("<s>");
				//Find the first occurence of the message trailer
				int trailerPos = globalBuffer.find("<e>");

				currentMessage = globalBuffer.substr(headerPos, trailerPos );
				currentMessage = currentMessage.substr(3);
				
				std::cout << "buffer globale: " << globalBuffer << std::endl;
				std::cout << "messaggio corrente: " << currentMessage << std::endl;

				globalBuffer.replace(headerPos, trailerPos + 2, "");
				std::cout << "buffer globale dopo replace: " << globalBuffer << std::endl;



				/*
				int tokenPos=receivedMex.find("|");
				std::wcout << "Token in posizione: " << tokenPos << std::endl;
				int pidLength = tokenPos - 3;
				std::string pidString = receivedMex.substr(3, pidLength);
				std::cout << "PID: " << pidString << std::endl;
				*/
				// Echo the buffer back to the sender
				iSendResult = send(hClientSocket, recvbuf, iResult, 0);
				if (iSendResult == SOCKET_ERROR) {
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(hClientSocket);
					WSACleanup();
					return;
				}
			}
			else if (iResult == 0)
				printf("Connection closing...\n");
			
			else {
				printf("recv failed with error: %d\n", WSAGetLastError());
				closesocket(hClientSocket);
				WSACleanup();
				return;
			}

		} while (iResult > 0);


	}


