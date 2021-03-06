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
#include "IconExtractor.h"
#include "MyHook.h"
#include "EventInfo.h"
#include "ProcessModel.h"
#include "json\json.h"
#include "JsonSerializer.h"
#include "Network.h"
#include "Controller.h"



Controller::Controller() : myHookObj(L"Dll.dll"), Slot(TEXT("\\\\.\\mailslot\\ms1")), 
	x86ProcessPath(L"32BitCode\\ProcessMonitorX86.exe")
{
	//Inizializzazione strutture dati necessarie alla creazione processo monitor 32 bit
	ZeroMemory(&x86StartupInfo, sizeof(x86StartupInfo));
	x86StartupInfo.cb = sizeof(x86StartupInfo);
	ZeroMemory(&x86ProcessInformation, sizeof(x86ProcessInformation));	
}

Controller::~Controller()
{
	//Terminiamo il processo monitor a 32 bit
	SetEvent(terminateMonitorX86);
	CloseHandle(x86ProcessInformation.hProcess);
	CloseHandle(x86ProcessInformation.hThread);
	
	CloseHandle(eventX64);
	CloseHandle(eventX64);
	CloseHandle(eventClientConNet);
	CloseHandle(eventRecNet);
	CloseHandle(terminateMonitorX86);
	





}

//Metodo di inizializzazione del controller, richiama la enumWindows per fotografare lo stato corrente dei processi attivi
//Salva l alista dei processi attivi nel model
bool Controller::Init()
{

	//Creo il mailslot
	MakeSlot(Slot);

	//Event initialization
	eventX86 = CreateEvent(
		NULL,						// default security attributes
		TRUE,						// manual-reset event
		FALSE,						// initial state is nonsignaled
		TEXT("eventX86")			// object name
	);

	eventX64 = CreateEvent(
		NULL,						// default security attributes
		TRUE,						// manual-reset event
		FALSE,						// initial state is nonsignaled
		TEXT("eventX64")			// object name
	);

	eventClientConNet = CreateEvent(
		NULL,						// default security attributes
		TRUE,						// manual-reset event
		FALSE,						// initial state is nonsignaled
		TEXT("eventClientConNet")	// object name		
	);

	eventRecNet = CreateEvent(
		NULL,						// default security attributes
		TRUE,						// manual-reset event
		FALSE,						// initial state is nonsignaled
		TEXT("eventRecNet")			// object name
	);

	terminateMonitorX86 = CreateEvent(
		NULL,						// default security attributes
		TRUE,						// manual-reset event
		FALSE,						// initial state is nonsignaled
		TEXT("terminateMonitorX86")	// object name
	);
	

	

	WindowsEnum we;
	we.enum_windows();
	model.setProcessesList(we.getData());
	
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
			return false;	

		lpszBuffer[0] = '\0';

		fResult = ReadFile(hSlot,
			lpszBuffer,
			cbMessage,
			&cbRead,
			NULL);

		if (!fResult)
		{			
			GlobalFree((HGLOBAL)lpszBuffer);
			return false;
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

	//Tupla temporanea che contiene le informazioni del processo
	//Inizializzazione della tupla per evitare di inviare dati vuoti
	ProcessModel::processInfo pInfoTmp = {0,0,L"Title",L"Path","Icon"};		
											
	std::wstring pInfoStr;					//Stringa delle informazioni processo serializzate
	

	switch (info.eventType)
	{
	case WINDOWCREATED:		//Processo creato
	{
		std::wcout << "Il processo con Handle: " << info.hWnd << " e' stato creato"<< std::endl;
		if (model.addProcess(info.hWnd)) {
			pInfoTmp = model.getProcessInfo(info.hWnd);
			//Settiamo lo stato del processo nella tupla da inviare
			std::get<1>(pInfoTmp) = WINDOWCREATED;
			//inviare aggiornamento al client
			if (netObj.isConnected()) { //se il client � connesso invio i dati
				pInfoStr =jSer.serializeProcessInfo(pInfoTmp);
				netObj.sendMessage(pInfoStr);
			}			
		}
		
		
		break;
	}
	case WINDOWCLOSED:		//Processo chiuso
	{
		std::wcout << "Il processo con Handle: " << info.hWnd << " e' stato distrutto"<< std::endl;
		DWORD pidTmp = model.hwndToPid(info.hWnd);	//Salvo il pid del processo prima di rimuoverlo dalla struttura dati
		if (model.removeProcess(info.hWnd)) {		//se il processo viene rimosso correttamente invio le informazioni
			if (netObj.isConnected()) {
				//Creare una tupla che contiene solo pid e tipo evento, perch� le informazioni 
				//sul processo non sono reperibili dato che � stato chiuso			
				std::get<0>(pInfoTmp) = pidTmp;
				std::get<1>(pInfoTmp) = WINDOWCLOSED;
				pInfoStr = jSer.serializeProcessInfo(pInfoTmp);
				netObj.sendMessage(pInfoStr);
			}		
		}
		
		
		break;
	}
	case WINDOWFOCUSED:		//Processo ha preso il focus
	{
		std::wcout << "Il processo con Handle: " << info.hWnd << " ha ottenuto focus"<< std::endl;

		if (model.setFocusedProcess(info.hWnd)) {						
			if (netObj.isConnected()) {
				//Creare una tupla che contiene solo pid e tipo evento perch� le informazioni sono gi� a disposizione del client	
				std::get<0>(pInfoTmp) = model.hwndToPid(info.hWnd);
				std::get<1>(pInfoTmp) = WINDOWFOCUSED;
				pInfoStr = jSer.serializeProcessInfo(pInfoTmp);
				netObj.sendMessage(pInfoStr);
			}
		}		
		
		break;
	}
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
	case NETCLIENTCONNECTED:	//Client connesso
	{
		std::cout << "Client connesso" << std::endl;
		std::wstring processInfoStr;
		std::wstring focusedProcessInfoStr;

		//invio delle informazioni sui processi presenti
		processInfoStr = jSer.serializeProcessesInfo(model.getProcessesInfo());
		netObj.sendMessage(processInfoStr);
		
		
		//Invio del processo in focus
		//Creare una tupla che contiene solo pid e tipo evento perch� le informazioni sono gi� a disposizione del client
		ProcessModel::processInfo pInfoTmp = { 0,0,L"Title",L"Path","Icon" };
		std::get<0>(pInfoTmp) = model.hwndToPid(model.getFocusedProcess());
		std::get<1>(pInfoTmp) = WINDOWFOCUSED;
		focusedProcessInfoStr = jSer.serializeProcessInfo(pInfoTmp);
		netObj.sendMessage(focusedProcessInfoStr);		
		ResetEvent(eventClientConNet);		
		break;
	}
	

	case NETWORKMESSAGE:		//messaggio di rete ricevuto
	{
		//Se il processo in focus � lo stesso a cui il client ha mandato la shortcut gli passo la sequenza comandi
	

		//Caso di ricezione di una shortcut da inviare al processo corrente
		if (model.hwndToPid(model.getFocusedProcess()) == netEventInfo.pid) {
			//Abbiamo 2 slash da oindividuare

			size_t tagPos = netEventInfo.additionalInfo.find("/"); //Tag di separazione della descrizione del comando e del codice modificatore
			size_t tagPos2 = netEventInfo.additionalInfo.find("/",tagPos+1); //Tag di separazione della descrizione del comando e del codice tasto		

			std::string keyEventType = netEventInfo.additionalInfo.substr(0, tagPos);					//First part of the message contains the action performed on the key
			size_t modLength = tagPos2 - tagPos - 1;													//length of the modifier substring
			std::string modifierStr = netEventInfo.additionalInfo.substr(tagPos + 1,modLength);		//second part of the message contains the modifiers pressed
			std::string keyStr = netEventInfo.additionalInfo.substr(tagPos2 + 1);						//third part of the message contains the key pressed

			//Inserire eccezione
			int key;
			try{
				key = std::stoi(keyStr, nullptr, 0);
			}
			catch (std::exception e) {
				ResetEvent(eventRecNet);
				break;
			}
			int modifier;
			try {
				modifier = std::stoi(modifierStr, nullptr, 0);
			}
			catch (std::exception e) {
				ResetEvent(eventRecNet);
				break;
			}


			

			INPUT *ip = new INPUT;
			

			if (keyEventType.compare("dw") == 0) {	//If user has pressed the key
				ip->type = INPUT_KEYBOARD;
				ip->ki.dwFlags = 0;					//Key pressed
				ip->ki.wScan = 0;
				ip->ki.time = 0;
				ip->ki.dwExtraInfo = 0;




				//Combinazioni: 1 alt			1
				//				2 ctrl			4
				//				3 ctrl+alt		5		
				//				4 ctrl+shift	6
				//				5 alt+shift		3
				//				6 ctrl+alt+shit	7
				//				7 shift			2
				//Invio eventuale modificatore inserito
				if (modifier != 0) {
					ip->ki.wVk = 0;
					if (modifier & SHIFT) {
						ip->ki.wVk = VK_SHIFT;
						SendInput(1, ip, sizeof(INPUT));
					}
					if (modifier & CTRL) {
						ip->ki.wVk = VK_CONTROL;
						SendInput(1, ip, sizeof(INPUT));
					}
						
					if (modifier & ALT) {
						ip->ki.wVk = VK_MENU;
						SendInput(1, ip, sizeof(INPUT));
					}					
				}
				//Invio il tasto inserito
				ip->ki.wVk = key;
				SendInput(1, ip, sizeof(INPUT));
				

					
			}else if(keyEventType.compare("up") == 0){ //If user has released the key

				ip->type = INPUT_KEYBOARD;
				ip->ki.dwFlags = KEYEVENTF_KEYUP;		//Key released
				ip->ki.wScan = 0;
				ip->ki.time = 0;
				ip->ki.dwExtraInfo = 0;
				
				//Invio il tasto sollevato
				ip->ki.wVk = key;
				SendInput(1, ip, sizeof(INPUT));

				if (modifier != 0) {
					ip->ki.wVk = 0;
					if (modifier & SHIFT) {
						ip->ki.wVk = VK_SHIFT;
						SendInput(1, ip, sizeof(INPUT));
					}
					if (modifier & CTRL) {
						ip->ki.wVk = VK_CONTROL;
						SendInput(1, ip, sizeof(INPUT));
					}

					if (modifier & ALT) {
						ip->ki.wVk = VK_MENU;
						SendInput(1, ip, sizeof(INPUT));
					}
				}							
			}
		}

		//Caso chiusura della connessione da parte del client
		if (netEventInfo.pid == NETWORKEXIT) {
			if (netObj.isConnected()) { //se il client � connesso invio la stringa di uscita				
				netObj.sendMessage(NETWORKEXITCODE);
				netObj.sendMessage(NETWORKEXITCODE);										
			}
		}			
		ResetEvent(eventRecNet);
		break;
	}
	default:
		break;
	}
	
}

void Controller::Run()
{
	//Smithers libera i thread	
	std::thread hookThread{ &MyHook::StartMonitoringProcesses,&myHookObj };

	std::thread netThread{ &Network::networkTask, &netObj};
	
	//Lanciamo processo di hook a 32 bit
	if (!CreateProcess(x86ProcessPath, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &x86StartupInfo, &x86ProcessInformation)) {
		std::cout << "Errore avvio processo monitor 32 bit con errore:" << GetLastError() << std::endl;
	}

	

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
			break;
		}
		case 2: {	//Evento connessione client
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
}
