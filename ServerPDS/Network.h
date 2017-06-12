//Network class that ensure the communication with the client
//it uses windwos mailslot to communicate with server controller

#ifndef NETWORK_H
#define NETWORK_H


class Network
{

private:

	std::string portNumberStr;
	//Network Events
	HANDLE networkMessageRecEvent;		//Event for received mex	
	HANDLE networkClientConnEvent;		//Event for client connection

	EventInfo messageInfo;				//Network message information to pass at the controller
	std::queue<EventInfo> MessageQueue;	//Network message queue
	void pushNetworkMessage(EventInfo);	//Add network message to the network queue
	std::mutex mut;

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
	
	bool WriteSlot(HANDLE hSlot, LPCTSTR lpszMessage);			//Send the message to the Mail Slot
	bool restartNetwork();
	void closeConnection();
	bool receiveMessages();


public:
	Network();
	~Network();

	bool getConnectionState();

	EventInfo getNetworkMessage();								//Return the first message in the network message queue
	size_t getNetworkMessagesNumber();							//Return the numbr of messages in the network message queue	

	void networkTask();

	bool initNetwork(std::string);								//Initialize the socket and start listening for connection	
	
	

	
};
#endif // !NETWORK_H

