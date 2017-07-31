// dllmain.cpp: definisce il punto di ingresso per l'applicazione DLL.
#include "stdafx.h"
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <Strsafe.h>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <sstream>
#include "dll.h"



HHOOK SysHook;
HINSTANCE hInst;

//Nome ed handle del mailslot usato per la comunicazione tra processi
LPTSTR Slot;	//Nome mailslot
HANDLE hSlot;	//handle mailslot



BOOL APIENTRY DllMain(HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved){
	hInst = (HINSTANCE)hModule;

	Slot = TEXT("\\\\.\\mailslot\\ms1");
	hSlot = CreateFile(Slot, GENERIC_WRITE,
		FILE_SHARE_READ,
		(LPSECURITY_ATTRIBUTES)NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
		(HANDLE)NULL);

	return TRUE;
}



//Method used to send message via messageslot
bool WriteSlot(HANDLE hSlot, LPCTSTR lpszMessage)
{
	DWORD cbWritten;
	BOOL fResult = WriteFile(hSlot, lpszMessage,
		(DWORD)(lstrlen(lpszMessage) + 1) * sizeof(TCHAR),
		&cbWritten, (LPOVERLAPPED)NULL);
	if (!fResult)
	{
		return false;
	}
	return true;
}



LRESULT CALLBACK ShellProc(
	int code,		// hook code
	WPARAM wParam,	// removal flag
	LPARAM lParam 	// address of structure with message
)
{	
	//True if we have to send a message to mailSlot
	bool sendMessage = false;

	HWND hWnd = (HWND)wParam;
	HANDLE processEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, L"eventX64");
	//Messaggio che verrà inviato tramite mailslot
	std::wstring message;

	//Informazioni aggiuntive sull'evento 
	std::wstring infoString;
	
	//Salvo handle del processo come stringa
	std::wstringstream ssTemp;
	//Handle trasformato in stringa
	std::wstring handleString;
	
	ssTemp << hWnd;
	ssTemp >> handleString; //handleString ora contiene il valore di hWnd trasformato in stringa
	

	//Salviamo il tipo di evento lanciato nella variabile eventString
	//Il token di separazione della stringa è lo spazio " " per questo prima del tipo di evento c'è uno spazio!
	if (code == HSHELL_WINDOWCREATED)
	{		
		if (IsWindowVisible(hWnd)){
			infoString = L" 1 null";
			sendMessage = true;
		}
	}
	if (code == HSHELL_WINDOWDESTROYED) {
		infoString = L" 2 null";
		sendMessage = true;
	}
	if (code == HSHELL_WINDOWACTIVATED) {
		infoString = L" 3 null";
		sendMessage = true;
	}

	//Se l'evento è di uno dei 3 tipi elencati sopra (creazione distruzione o focus) allora deve essere inviato il messaggio
	if (sendMessage) {
		//Messaggio da inviare tramite mailslot
		message = handleString + infoString;
		//INVIAMO AL MAILSLOT
		if (WriteSlot(hSlot, message.c_str())) {
			Sleep(200);
			SetEvent(processEvent);
		}
	}
	return CallNextHookEx(SysHook, code, wParam, lParam);;
}


//METODI ESPOSTI DALLA DLL

DllExport void RunStopHook(bool State, HINSTANCE hInstance)
{	
	if (State){
		SysHook = SetWindowsHookEx(WH_SHELL, &ShellProc, hInst, 0);		
	}
	else{
		UnhookWindowsHookEx(SysHook);
	}
}
