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
#include "ProcessModel.h"
#include "Controller.h"




//  
// Methods
//  

//Metodo di inizializzazione del controller, richiama la enumWindows per fotografare lo stato corrente dei processi attivi
//Salva l alista dei processi attivi nel model
bool Controller::Init()
{
	WindowsEnum we;
	we.enum_windows();
	model.setProcessesList(we.getData());
	

	
	
	

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
			//TODO SCRIVERE IL MESSAGGIO IN UN FILE DI LOG
			//printf("ReadFile failed with %d.\n", GetLastError());
			GlobalFree((HGLOBAL)lpszBuffer);
			return FALSE;
		}

		
		// Save the message into the messageList. 
		std::wstring mex(lpszBuffer);
		messageQueue.push(mex);

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

//Extrapolate the event info and process handle from the message stored in the message queue
Controller::Handle_Event_Str Controller::MessageToHandle_Event_Struct(std::wstring message)
{
	Handle_Event_Str hwndEventInfo;

	std::wstringstream ss(message);
	std::wstring vec[2]; //vettore che contiene hwnd del processo (prima posizione) ed evento relativo (seconda posizione)	
	int i = 0;
	while (ss.good() && i<2) {
		ss >> vec[i];
		++i;
	}	

	std::wstring hwndString = vec[0];	//handle processo che ha generato evento
	std::wstring eventTypeStr = vec[1];	//evento generato dal processo 				

	//Converto la stringa in hwnd										
	intptr_t hwndAddress;
	std::wstringstream ssTemp;
	ssTemp << std::hex << hwndString;
	ssTemp >> hwndAddress;
	hwndEventInfo.hWnd = reinterpret_cast<HWND>(hwndAddress);

	hwndEventInfo.eventType = std::stoi(eventTypeStr);


	return hwndEventInfo;
}

void Controller::Run()
{

	HANDLE evento0 = CreateEvent(
		NULL,               // default security attributes
		TRUE,               // manual-reset event
		FALSE,              // initial state is nonsignaled
		TEXT("Evento")		// object name
	);

	//Creo il mailslot
	MakeSlot(Slot);

	std::thread hookThread{ &MyHook::StartMonitoringProcesses,&myHookObj };

	

	while (true) {
		
		ResetEvent(evento0);
		WaitForSingleObject(evento0, INFINITE);

		//LEGGO LA MAIL SLOT
		if (ReadSlot()) {
			while (messageQueue.size()) {						
				Controller::Handle_Event_Str info;
				//Estrapolo informazioni dalla message queue
				info=MessageToHandle_Event_Struct(messageQueue.front());
				
				//Message elaboration
				//Processo creato eventType==1
				if(info.eventType==1) {
					std::wcout << "Il processo con Handle: " << info.hWnd << " e' stato creato" << info.eventType << std::endl;
					if (!model.addProcess(info.hWnd)) {
						//@TODO sollevare eccezione impossibilità inserire dato nel model
					}
					//@TODO inviare dato al client

				}
				//Processo eliminato eventType==2
				else if (info.eventType == 2) {
					std::wcout << "Il processo con Handle: " << info.hWnd << " e' stato distrutto " << info.eventType << std::endl;
					if (!model.removeProcess(info.hWnd)) {
						//@TODO sollevare eccezione impossibilità inserire dato nel model
					}
					//@TODO inviare dato al client
				}
				//Processo ha focus eventType==3
				else if (info.eventType == 3) {
					std::wcout << "Il processo con Handle: " << info.hWnd << " prese fiamme" << info.eventType << std::endl;
					if (!model.setFocusedProcess(info.hWnd)) {
						//@TODO sollevare eccezione impossibilità settare focus
					}
				//@TODO inviare dato al client
				}
				//Delete the top element of the queue
				messageQueue.pop();				
			}
		}
		else {
			//@TODO gestire caso di errore
		}



	}
	

	
	//hookThread.join(); va messo?
}
