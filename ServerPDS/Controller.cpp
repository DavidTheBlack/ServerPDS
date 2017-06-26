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
	x86ProcessPath(L"D:\\PoliTo\\Materie\\PdS\\Progetto\\32BitCode\\ProcessMonitorX86.exe")
{
	//Inizializzazione strutture dati necessarie alla creazione processo monitor 32 bit
	ZeroMemory(&x86StartupInfo, sizeof(x86StartupInfo));
	x86StartupInfo.cb = sizeof(x86StartupInfo);
	ZeroMemory(&x86ProcessInformation, sizeof(x86ProcessInformation));	
}

Controller::~Controller()
{
	//Terminiamo il processo monitor a 32 bit
	//SetEvent(terminateMonitorX86);
	//CloseHandle(x86ProcessInformation.hProcess);
	//CloseHandle(x86ProcessInformation.hThread);

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
	//model.setProcessesList(we.enum_windows().getData());
	
	
	////Trying to start the network
	//std::thread netInitThread{ &Network::initNetwork,&netObj,"4444" };

	/*if (!netObj.initNetwork("4444")) {
		std::cout << "Impossibile avviare la connessione di rete" << std::endl;
	}*/

	
	
	//@TODO gestire gli errori Gestire anche il caso in cui non avvenga connessione da client - la funzione deve comunque partire
	//l'inizializzazione della rete deve avvenire su thread separato

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

	ProcessModel::processInfo pInfoTmp;		//Tupla temporanea che contiene le informazioni del processo
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
			if (netObj.isConnected()) { //se il client è connesso invio i dati
				pInfoStr =jSer.serializeProcessInfo(pInfoTmp);
				netObj.sendMessage(pInfoStr);
			}			
		}
		else {
			//@TODO sollevare eccezione impossibilità inserire dato nel model
		}
		
		break;
	}
	case WINDOWCLOSED:		//Processo chiuso
	{
		std::wcout << "Il processo con Handle: " << info.hWnd << " e' stato distrutto"<< std::endl;
		DWORD pidTmp = model.hwndToPid(info.hWnd);	//Salvo il pid del processo prima di rimuoverlo dalla struttura dati
		if (model.removeProcess(info.hWnd)) {		//se il processo viene rimosso correttamente invio le informazioni
			if (netObj.isConnected()) {
				//Creare una tupla che contiene solo pid e tipo evento, perchè le informazioni 
				//sul processo non sono reperibili dato che è stato chiuso			
				std::get<0>(pInfoTmp) = pidTmp;
				std::get<1>(pInfoTmp) = WINDOWCLOSED;
				pInfoStr = jSer.serializeProcessInfo(pInfoTmp);
				netObj.sendMessage(pInfoStr);
			}		
		}
		else {
			//@TODO sollevare eccezione impossibilità inserire dato nel model
		}
		
		break;
	}
	case WINDOWFOCUSED:		//Processo ha preso il focus
	{
		std::wcout << "Il processo con Handle: " << info.hWnd << " ha ottenuto focus"<< std::endl;

		if (model.setFocusedProcess(info.hWnd)) {			
			//@TODO inviare dato al client
			if (netObj.isConnected()) {
				//Creare una tupla che contiene solo pid e tipo evento perchè le informazioni sono già a disposizione del client	
				std::get<0>(pInfoTmp) = model.hwndToPid(info.hWnd);
				std::get<1>(pInfoTmp) = WINDOWFOCUSED;
				pInfoStr = jSer.serializeProcessInfo(pInfoTmp);
				netObj.sendMessage(pInfoStr);
			}
		}
		else {
			//@TODO sollevare eccezione impossibilità settare focus
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
		//invio delle informazioni sui processi presenti

		processInfoStr = jSer.serializeProcessesInfo(model.getProcessesInfo());

		netObj.sendMessage(processInfoStr);

		ResetEvent(eventClientConNet);		
		break;
	}
		
	case NETWORKMESSAGE:		//messaggio di rete ricevuto
	{
		//@TODO girare shortcut al processo in focus
		std::cout << "Processo con PID" << netEventInfo.pid << "ha ricevuto shortcut: " << netEventInfo.additionalInfo.c_str()
			<< std::endl << "ed ha HANDLE: " << model.pidToHwnd(netEventInfo.pid) << std::endl;
		ProcessModel::processInfo pInfo = model.getProcessInfo(model.pidToHwnd(netEventInfo.pid));



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
		//@TODO gestire errore
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
