#include <WinSock2.h>
#include <ws2tcpip.h>
#include <strsafe.h>
#include <iostream>
#include <sstream>
#include <string>
#include <queue>
#include <mutex>
#include <algorithm>
#include <stdint.h>
#include "EventInfo.h"
#include "Network.h"
#pragma comment(lib, "Ws2_32.lib")





	
	Network::Network() :iResult(0), hSocket(INVALID_SOCKET), hClientSocket(INVALID_SOCKET), clientConnected(false) {		
		
	}

	Network::~Network() {

		iResult = shutdown(hClientSocket, SD_SEND);
		if (iResult == SOCKET_ERROR) {
			errorShow(TEXT("shutdown network connection"));			
		}

		// cleanup
		closesocket(hClientSocket);
		WSACleanup();
	}

	bool Network::getConnectionState()
	{
		return clientConnected;
	}

	EventInfo Network::getNetworkMessage()
	{
		std::lock_guard<std::mutex> l(mut);
		EventInfo temp;
		if (MessageQueue.empty()) {
			temp.additionalInfo = "Empty";
		}
		else {			
			temp = MessageQueue.front();
			MessageQueue.pop();
		}
		return temp;
	}

	void Network::pushNetworkMessage(EventInfo info)
	{
		std::lock_guard<std::mutex> l(mut);
		MessageQueue.push(info);
	}

	size_t Network::getNetworkMessagesNumber()
	{
		std::lock_guard<std::mutex> l(mut);
		return MessageQueue.size();
	}

	//Method that will be threaded
	void Network::networkTask()
	{
		while (true) {
			if (!receiveMessages()) {
				std::cout << "Receive message ha restituito un errore" << std::endl;
				//Se la receive message torna false devo riavviare il socket e accettare nuove connessioni
				//@TODO gestire la notifica della connessione e disconnessione tramite evento
				restartNetwork();
			}
		}	
	}

	//Initialize the socket and start listening for connection and prepare the event
	bool Network::initNetwork(std::string portNumber)
	{
		//Initialize event detail
		networkClientConnEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, L"eventClientConNet");
		networkMessageRecEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, L"eventRecNet");
		

		portNumberStr = portNumber;
		if (!startWinsock(2, 2)) return false;
		if (!startSocket(portNumberStr.c_str())) return false;
		if (!acceptClient()) return false;
		
		return true;
	}

	//Reinitialize connection after a network error
	bool Network::restartNetwork() {
		if (!startWinsock(2, 2)) return false;
		if (!startSocket(portNumberStr.c_str())) return false;
		if (!acceptClient()) return false;

		return true;
	}

	//Close connection 
	void Network::closeConnection()
	{
		std::cout << "Chiusura del socket" << std::endl;
		clientConnected = false;
		
		if(hClientSocket != INVALID_SOCKET)
			closesocket(hClientSocket);
		std::cout << "client socket - closeConnection: " << hClientSocket << std::endl;
		WSACleanup();

	}

	bool Network::startWinsock(BYTE majorVersion, BYTE minorVersion) {

		WSADATA wsaData;
		std::cout << "Initializing winsock... "<<std::endl;
		/* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
		WORD wVersionRequested = MAKEWORD(minorVersion, majorVersion);
		iResult = WSAStartup(wVersionRequested, &wsaData);
		if (iResult != 0) {
			/* Tell the user that we could not find a usable */
			/* Winsock DLL.                                  */
			std::cout << "WSAStartup failed with error: " << iResult << std::endl;;
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
			std::cout << "Could not find a usable version of Winsock.dll" << std::endl;
			WSACleanup();
			return false;
		}
		else
			std::cout << "The Winsock 2.2 dll was found okay" << std::endl;

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
			std::cout << "getaddrinfo failed with error: " << iResult << std::endl;
			WSACleanup();
			return false; //non va più bene!
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
		// Accept a client socket connection
		hClientSocket = accept(hSocket, NULL, NULL);
		if (hClientSocket == INVALID_SOCKET) {
			errorShow(TEXT("accept"));
			closeConnection();
			return false;
		}				
		closesocket(hSocket);
		clientConnected = true;
		SetEvent(networkClientConnEvent);
		
		return true;
	}

	//Method that receives network messages and communicates them to the controller
	bool Network::receiveMessages()
	{		
		int iSendResult;
		int iResult;

		// Receive until the peer shuts down the connection or send close connection message
		do {
			int idx = 0;			
			uint32_t varSize;
			iResult = recv(hClientSocket, (char*)&varSize, sizeof(varSize), 0);
			if (iResult == SOCKET_ERROR) {
			//Gestione errore
				std::cout << "recv failed with error: \n" << WSAGetLastError() << std::endl;
			closeConnection();
			return false;	//ritorno dalla funzione di ricezione con errore
			}


			uint32_t nLeft = varSize;
			char *recvbuf = new char[varSize];

			while (nLeft > 0) {
			iResult = recv(hClientSocket, (char*)&recvbuf[idx], nLeft, 0);
			if (iResult == SOCKET_ERROR) {
				//Gestione errore
				printf("recv failed with error: %d\n", WSAGetLastError());
				closeConnection();
				return false; //ritorno dalla funzione di ricezione con errore
			}
			nLeft -= iResult;
			idx += iResult;
			}
							
			std::string totalDataStr = std::string(recvbuf);
			totalDataStr.erase(varSize);
			size_t tagPos= totalDataStr.find("|");
			std::string pidStr = totalDataStr.substr(0, tagPos);	//First part of the message is the pid of the process
			std::string keyStr = totalDataStr.substr(tagPos + 1);	//second part of the messa is the shortcut key to send at process

				
			//PASSAGGIO DELLE INFORMAZIONI AL CONTROLLER
			messageInfo.pid = std::stoul(pidStr, nullptr, 0);
			messageInfo.eventType = NETWORKMESSAGE;
			messageInfo.additionalInfo = keyStr;					//Shortcut inviata dal client

			pushNetworkMessage(messageInfo);			

			//Messaggio ricevuto, sollevo evento
			SetEvent(networkMessageRecEvent);						
				
				
			// Echo the buffer back to the sender
			iSendResult = send(hClientSocket, recvbuf, iResult, 0);
			if (iSendResult == SOCKET_ERROR) {
			printf("send failed with error: %d\n", WSAGetLastError());
			closeConnection();
			return false;
			}			
			else if (iResult == 0)
			printf("Connection closing...\n");
			
		} while (iResult > 0);		
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



