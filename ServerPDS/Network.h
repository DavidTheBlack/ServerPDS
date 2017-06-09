//Network class that ensure the communication with the client
//it uses windwos mailslot to communicate with server controller

#pragma once
#define DEFAULT_BUFLEN 512

class Network
{

private:

	std::string portNumberStr;
	//Network Events
	HANDLE networkMessageRecEvent;	//Event for received mex	
	HANDLE networkClientConnEvent;	//Event for client connection


	//Windows mailslot parameters
	LPTSTR Slot;	//Nome mailslot
	HANDLE hSlot;	//handle mailslot

	//True if there is a client connected false otherwise
	bool clientConnected ;
	

	SOCKET hSocket;
	SOCKET hClientSocket;
	int iResult;

	

	bool startWinsock(BYTE majorVersion, BYTE minorVersion);
	bool startSocket(PCSTR portNumber);
	bool acceptClient();
	void errorShow(LPTSTR lpszFunction);
	//Send the message to the Mail Slot
	bool WriteSlot(HANDLE hSlot, LPCTSTR lpszMessage);

	

	bool restartNetwork();

	void closeConnection();

	bool receiveMessages();


public:
	Network();
	~Network();

	bool getConnectionState();

	void networkTask();

	//Initialize the socket and start listening for connection	
	bool initNetwork(std::string);
	
	

	
};

