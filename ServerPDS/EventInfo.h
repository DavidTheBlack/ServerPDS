#ifndef EVENTINFO_H
#define EVENTINFO_H

#define	WINDOWINIT	0			//used to communicate to the client the initial process state
#define WINDOWCREATED 1
#define WINDOWCLOSED 2
#define WINDOWFOCUSED 3
#define NETCLIENTCONNECTED 4
#define NETWORKMESSAGE 5
#define NETWORKEXIT 7					//Used to communicate to close the connection
#define NETWORKEXITCODE L"7|exit"	




typedef struct {
	int eventType;
	HWND hWnd;
	DWORD pid;	
	std::string additionalInfo; //Additional information to deliver to the process
}EventInfo;

#endif



