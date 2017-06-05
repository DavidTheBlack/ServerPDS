//Network class that ensure the communication with the client
//it uses windwos mailslot to communicate with server controller

#pragma once
#define DEFAULT_BUFLEN 512

class Network
{

private:
	//Windows mailslot parameters
	LPTSTR Slot;	//Nome mailslot
	HANDLE hSlot;	//handle mailslot

	

	SOCKET hSocket;
	SOCKET hClientSocket;
	int iResult;

	bool startWinsock(BYTE majorVersion, BYTE minorVersion);
	bool startSocket(PCSTR portNumber);
	bool acceptClient();
	void errorShow(LPTSTR lpszFunction);
	//Send the message to the Mail Slot
	bool WriteSlot(HANDLE hSlot, LPCTSTR lpszMessage);


public:
	Network();
	~Network();
	//Initialize the socket and start listening for connection
	
	bool initNetwork(PCSTR portNumber);
	
	void receiveMessages();

	
};

