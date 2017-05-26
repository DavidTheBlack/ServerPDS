#include "stdafx.h"
#include <thread>
#include <iostream>
#include <string>
#include <list>
#include <mutex>
#include <Windows.h>
#include <queue>
#include <fstream>
#include <strsafe.h>
#include "MsgQueue.h"
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
	//Setto la message queue da usare per le comunicazioni con la dll
	
	MyHook::Instance().SetMessageQueue(&messageQueue);
	

	//@TODO gestire gli errori
	return true;
}



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
	else printf("Mailslot created successfully.\n");
	return TRUE;
}



BOOL Controller::ReadSlot()
{
	DWORD cbMessage, cMessage, cbRead;
	BOOL fResult;
	LPTSTR lpszBuffer;
	TCHAR achID[80];
	DWORD cAllMessages;


	cbMessage = cMessage = cbRead = 0;
	

	fResult = GetMailslotInfo(hSlot, // mailslot handle 
		(LPDWORD)NULL,               // no maximum message size 
		&cbMessage,                  // size of next message 
		&cMessage,                   // number of messages 
		(LPDWORD)NULL);              // no read time-out 

	if (!fResult)
	{
		printf("GetMailslotInfo failed with %d.\n", GetLastError());
		return FALSE;
	}

	if (cbMessage == MAILSLOT_NO_MESSAGE)
	{
		printf("Waiting for a message...\n");
		return TRUE;
	}

	cAllMessages = cMessage;

	while (cMessage != 0)  // retrieve all messages
	{
		// Create a message-number string. 

		StringCchPrintf((LPTSTR)achID,
			80,
			TEXT("\nMessage #%d of %d\n"),
			cAllMessages - cMessage + 1,
			cAllMessages);

		// Allocate memory for the message. 

		lpszBuffer = (LPTSTR)GlobalAlloc(GPTR,
			lstrlen((LPTSTR)achID) * sizeof(TCHAR) + cbMessage);
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
			printf("ReadFile failed with %d.\n", GetLastError());
			GlobalFree((HGLOBAL)lpszBuffer);
			return FALSE;
		}

		// Concatenate the message and the message-number string. 

		StringCbCat(lpszBuffer,
			lstrlen((LPTSTR)achID) * sizeof(TCHAR) + cbMessage,
			(LPTSTR)achID);

		// Display the message. 

		_tprintf(TEXT("Contents of the mailslot: %s\n"), lpszBuffer);

		GlobalFree((HGLOBAL)lpszBuffer);

		fResult = GetMailslotInfo(hSlot,  // mailslot handle 
			(LPDWORD)NULL,               // no maximum message size 
			&cbMessage,                   // size of next message 
			&cMessage,                    // number of messages 
			(LPDWORD)NULL);              // no read time-out 

		if (!fResult)
		{
			printf("GetMailslotInfo failed (%d)\n", GetLastError());
			return FALSE;
		}
	}
	return TRUE;
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

	std::thread hookThread{ &MyHook::StartMonitoringProcesses,&MyHook::Instance() };

	
	
	//std::unique_lock<std::mutex> lock(messageQueue.eventMut);
	////GESTIONE DEGLI EVENTI CHE VENGONO SCRITI NELLA MESSAGE QUEUE
	//while (true) {
	//	std::cout << "Indirizzo MessageQueue:" << &messageQueue << std::endl;
	//	messageQueue.eventCondVar.wait(lock);
	//	std::cout << "tornati dalla condition variable " << std::endl;
	//	lock.unlock();
	//	while (true) {
	//		lock.lock();
	//		if (messageQueue.eventQueue.empty()) {
	//			break;
	//		}
	//		else {
	//			std::cout << "Processo: " << messageQueue.eventQueue.front().processHandle << " evento: " << messageQueue.eventQueue.front().eventTriggered << std::endl;
	//			messageQueue.eventQueue.pop();
	//		}
	//		lock.unlock();
	//	}
	//	
	//
	//	
	//	//todo implementare le ricezioni dei dati dai thread e azioni da intraprendere

	//	
	//}

	while (true) {
		
		ResetEvent(evento0);
		WaitForSingleObject(evento0, INFINITE);

		//LEGGO LA MAIL SLOT
		ReadSlot();
		}
	

	
	//hookThread.join();
}
