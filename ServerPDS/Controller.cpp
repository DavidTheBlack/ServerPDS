#include "stdafx.h"
#include <thread>
#include <iostream>
#include <string>
#include <sstream>
#include <list>
#include <mutex>
#include <Windows.h>
#include <queue>
#include <fstream>
#include <strsafe.h>
#include "WindowsEnum.h"
#include "MyHook.h"
#include "EventInfo.h"
#include "ProcessModel.h"
#include "Network.h"
#include "Controller.h"


//Metodo di inizializzazione del controller, richiama la enumWindows per fotografare lo stato corrente dei processi attivi
//Salva l alista dei processi attivi nel model
bool Controller::Init()
{

	//Creo il mailslot
	MakeSlot(Slot);

	//Event initialization
	eventX86 = CreateEvent(
		NULL,               // default security attributes
		TRUE,               // manual-reset event
		FALSE,              // initial state is nonsignaled
		TEXT("eventX86")	// object name
	);

	eventX64 = CreateEvent(
		NULL,               // default security attributes
		TRUE,               // manual-reset event
		FALSE,              // initial state is nonsignaled
		TEXT("eventX64")	// object name
	);

	eventClientConNet = CreateEvent(
		NULL,						// default security attributes
		TRUE,						// manual-reset event
		FALSE,						// initial state is nonsignaled
		TEXT("eventClientConNet")	// object name		
	);

	eventRecNet = CreateEvent(
		NULL,               // default security attributes
		TRUE,               // manual-reset event
		FALSE,              // initial state is nonsignaled
		TEXT("eventRecNet")	// object name
	);
	

	

	WindowsEnum we;
	we.enum_windows();
	model.setProcessesList(we.getData());
	//model.setProcessesList(we.enum_windows().getData());
	
	//Trying to start the network
	if (!netObj.initNetwork("4444")) {
		std::cout << "Impossibile avviare la connessione di rete" << std::endl;
	}
	
	//@TODO gestire gli errori
	return true;
}

//Create the mail slot to pass the information from the dll
bool Controller::MakeSlot(LPTSTR lpszSlotName)
{
	hSlot = CreateMailslot(lpszSlotName,
		0,                             // no maximum message size 
		MAILSLOT_WAIT_FOREVER,         // no time-out for operations 
		(LPSECURITY_ATTRIBUTES)NULL); // default security

	if (hSlot == INVALID_HANDLE_VALUE)
	{
		printf("CreateMailslot failed with %d\n", GetLastError());
		return FALSE;
	}
	return TRUE;
}

//Read the mail slot and return all the messages readed in a string vector
bool Controller::ReadSlot()
{
	DWORD cbMessage, cMessage, cbRead;
	BOOL fResult;
	LPTSTR lpszBuffer;
	DWORD cAllMessages;

	cbMessage = cMessage = cbRead = 0;
	

	fResult = GetMailslotInfo(hSlot, // mailslot handle 
		(LPDWORD)NULL,               // no maximum message size 
		&cbMessage,                  // size of next message 
		&cMessage,                   // number of messages 
		(LPDWORD)NULL);              // no read time-out 

	if (!fResult)
	{
		//@TODO scrivere gli errori in un file di log
		/*printf("GetMailslotInfo failed with %d.\n", GetLastError());*/ 
		
		return false;
	}


	//Se il mailslot non ha messaggi in coda esco dal metodo e ritorno true
	if (cbMessage == MAILSLOT_NO_MESSAGE)
	{
		return true;
	}

	cAllMessages = cMessage;

	while (cMessage != 0)  // retrieve all messages
	{
	
		// Allocate memory for the message. 
		lpszBuffer = (LPTSTR)GlobalAlloc(GPTR, cbMessage);
		if (NULL == lpszBuffer)
			return FALSE;	

		lpszBuffer[0] = '\0';

		fResult = ReadFile(hSlot,
			lpszBuffer,
			cbMessage,
			&cbRead,
			NULL);

		if (!fResult)
		{
			//@TODO SCRIVERE IL MESSAGGIO IN UN FILE DI LOG
			//printf("ReadFile failed with %d.\n", GetLastError());
			GlobalFree((HGLOBAL)lpszBuffer);
			return FALSE;
		}

		
		// Save the message into the messageList. 
		std::wstring mex(lpszBuffer);
		hookMessageQueue.push(mex);

		GlobalFree((HGLOBAL)lpszBuffer);


		//Check if there are other messages in the mailSlot
		fResult = GetMailslotInfo(hSlot,	// mailslot handle 
			(LPDWORD)NULL,					// no maximum message size 
			&cbMessage,						// size of next message 
			&cMessage,						// number of messages 
			(LPDWORD)NULL);					// no read time-out 

		if (!fResult)
		{
			//@TODO scrivere il messaggio di errore in un file di log
			//printf("GetMailslotInfo failed (%d)\n", GetLastError()); 
			return false;
		}
	}	
	return true;
}

//Extrapolate the event info and process handle from the message stored in the mail slot
EventInfo Controller::MessageToHandle_Event_Struct(std::wstring message)
{
	EventInfo hwndEventInfo;

	std::wstringstream ss(message);
	std::wstring vec[3]; //vettore che contiene hwnd del processo (prima posizione) ed evento relativo (seconda posizione)	
	int i = 0;
	while (ss.good() && i<3) {
		ss >> vec[i];
		++i;
	}	

	std::wstring hwndString = vec[0];				//handle processo che ha generato evento
	std::wstring eventTypeStr = vec[1];				//evento generato dal processo
	std::wstring additionalInformation = vec[2];	//Informazioni aggiuntive passate al processo

	//Converto la stringa in hwnd										
	intptr_t hwndAddress;
	std::wstringstream ssTemp;
	ssTemp << std::hex << hwndString;
	ssTemp >> hwndAddress;
	hwndEventInfo.hWnd = reinterpret_cast<HWND>(hwndAddress);

	hwndEventInfo.eventType = std::stoi(eventTypeStr);


	return hwndEventInfo;
}

void Controller::ManageHookEvent(EventInfo info) {

	//Message elaboration
	//Processo creato eventType==1
	switch (info.eventType)
	{
	case WINDOWCREATED:		//Processo creato
		std::wcout << "Il processo con Handle: " << info.hWnd << " e' stato creato" << info.eventType << std::endl;
		if (!model.addProcess(info.hWnd)) {
			//@TODO sollevare eccezione impossibilità inserire dato nel model
		}
		//@TODO inviare dato al client
		break;
	case WINDOWCLOSED:		//Processo chiuso
		std::wcout << "Il processo con Handle: " << info.hWnd << " e' stato distrutto " << info.eventType << std::endl;
		if (!model.removeProcess(info.hWnd)) {
			//@TODO sollevare eccezione impossibilità inserire dato nel model
		}
		//@TODO inviare dato al client
		break;
	case WINDOWFOCUSED:		//Processo ha preso il focus
		std::wcout << "Il processo con Handle: " << info.hWnd << " ha ottenuto focus" << info.eventType << std::endl;
		if (!model.setFocusedProcess(info.hWnd)) {
			//@TODO sollevare eccezione impossibilità settare focus
		}
		//@TODO inviare dato al client
		break;
	
	default:
		break;
	}
	//Reset eventi processi x86 e x64
	ResetEvent(eventX86);
	ResetEvent(eventX64);

}

void Controller::ManageNetworkEvent(EventInfo netEventInfo)
{
	switch (netEventInfo.eventType)
	{
	case NETCLIENTCONNECTED:		//Client connesso
		std::cout << "Client connesso" << std::endl;		
		ResetEvent(eventClientConNet);
		break;
	case NETWORKMESSAGE:		//messaggio di rete ricevuto
			std::cout << "Processo con PID" << netEventInfo.pid << "ha ricevuto shortcut: " << netEventInfo.additionalInfo.c_str()
				<< std::endl << "ed ha HANDLE: " << model.pidToHwnd(netEventInfo.pid) <<std::endl;
			ResetEvent(eventRecNet);
		break;	
	default:
		break;
	}
	
}

void Controller::Run()
{
	//Smithers libera i thread	
	std::thread hookThread{ &MyHook::StartMonitoringProcesses,&myHookObj };
	std::thread netThread{ &Network::networkTask, &netObj};
	//Aggiungere il monitoraggio processi a 32bit

	//Aggiungere il thread di rete

	HANDLE events[4];
	events[0] = eventX86;
	events[1] = eventX64;
	events[2] = eventClientConNet;
	events[3] = eventRecNet;
	while (true) {
		
		DWORD eventRaised= WaitForMultipleObjects(4, events, FALSE, INFINITE);
		switch (eventRaised)
		{
		case 0:	//Eventi 0 ed 1 sollevati sono quelli degli hook
		case 1: {
			//LEGGO iL MAIL SLOT
			if (ReadSlot()) {
				while (hookMessageQueue.size()) {
					EventInfo hookEventInfo;
					//Estrapolo informazioni dalla message queue
					hookEventInfo = MessageToHandle_Event_Struct(hookMessageQueue.front());
					//Manage the event information retrieved from the mailslot
					ManageHookEvent(hookEventInfo);
					//Delete the top element of the queue
					hookMessageQueue.pop();
				}
			}
			else {
				//@TODO gestire caso di errore
			}

			break;
		}
		case 2: {			//Evento connessione client
			EventInfo netConnEventInfo;
			netConnEventInfo.eventType = NETCLIENTCONNECTED;
			ManageNetworkEvent(netConnEventInfo);
			break;
		}
		case 3: {	//Evento ricezione messaggio da client
				//Controllo se ci sono messaggi di rete
			while (netObj.getNetworkMessagesNumber() != 0) {
				EventInfo netEventInfo;
				netEventInfo = netObj.getNetworkMessage();
				//Gestisco evento di rete
				ManageNetworkEvent(netEventInfo);
			}
			break;
		}					
		default:
			break;
		}

	}
	//hookThread.join(); va messo?
}
