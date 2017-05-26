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

void savetofile(const char* path, HWND hWnd, char* c) {
	
	
	std::fstream File(path, std::ios::app);
	if (File.is_open())
	{
		TCHAR windtext[255];
		GetWindowText(hWnd, windtext, 255);
		LPSTR result = NULL;
		int len = WideCharToMultiByte(CP_UTF8, 0, windtext, -1, NULL, 0, 0, 0);

		if (len > 0)
		{
			result = new char[len + 1];
			if (result)
			{
				int resLen = WideCharToMultiByte(CP_UTF8, 0, windtext, -1, &result[0], len, 0, 0);
				if (resLen == len)
				{
					File << "window has been" << c << hWnd << ", ";
					File.write(result, len);
					File << std::endl;
				}
				delete[] result;
			}
		}
	}
	File.close();
}

void savetofile2(const char* path, HANDLE codaMessaggi) {


	std::fstream File(path, std::ios::app);
	if (File.is_open())
	{
		File << "Indirizzo coda messaggi" << codaMessaggi << std::endl;
	}
	File.close();
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


LRESULT CALLBACK CBTProc(
	int code,		// hook code
	WPARAM wParam,	// removal flag
	LPARAM lParam 	// address of structure with message
)
{

	CallNextHookEx(SysHook, code, wParam, lParam);

	if (code == HCBT_CREATEWND)
	{
		CREATESTRUCT *pcs = ((CBT_CREATEWND *)lParam)->lpcs;
		HWND hWnd = (HWND)wParam;
		if (pcs->hwndParent == NULL) { // check if it is a top-level window
			if (GetWindow(hWnd, GW_OWNER) == NULL && pcs->style & WS_OVERLAPPEDWINDOW) {  // check if it is an unowned window. i think i cannot know if it is visible because hook is called in a moment which the WS_VISIBLE constant has not been set.

				//savetofile2("C:\\Users\\dar_w\\Desktop\\report.txt", hWnd, " created: ", pcs);
			}
		}
	}
	/*if (code == HSHELL_WINDOWDESTROYED) {
		HWND hWnd = (HWND)wParam;

		savetofile("C:\\Users\\dar_w\\Desktop\\report.txt", hWnd, " destroyed: ");
	}
	if (code == HSHELL_WINDOWACTIVATED) {
		HWND hWnd = (HWND)wParam;

		savetofile("C:\\Users\\dar_w\\Desktop\\report.txt", hWnd, " activated: ");


	}*/
	return 0;
}


LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	char ch;
	if (((DWORD)lParam & 0x40000000) && (HC_ACTION == nCode))
	{
		if ((wParam == VK_SPACE) || (wParam == VK_RETURN) || (wParam >= 0x2f) && (wParam <= 0x100))
		{
			FILE *f1;
			fopen_s(&f1, "C:\\Users\\dar_w\\Desktop\\report.txt", "a+");
			if (f1 != NULL)
			{
				if (wParam == VK_RETURN)
				{
					ch = '\n';
					fwrite(&ch, 1, 1, f1);
				}
				else
				{
					BYTE ks[256];
					GetKeyboardState(ks);

					WORD w;
					UINT scan = 0;
					ToAscii(wParam, scan, ks, &w, 0);
					ch = char(w);
					fwrite(&ch, 1, 1, f1);
				}
				fclose(f1);
			}
		}
	}

	LRESULT RetVal = CallNextHookEx(SysHook, nCode, wParam, lParam);
	return  RetVal;
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
	HANDLE evento0 = OpenEvent(EVENT_ALL_ACCESS, FALSE, L"Evento");
	//Messaggio che verrà inviato tramite mailslot
	std::wstring message;

	//Messaggio di evento indica l'evento che è stato sollevato dalla finestra
	std::wstring eventString;
	
	//Salvo handle del processo come stringa
	std::wstringstream ssTemp;
	//Handle trasformato in stringa
	std::wstring handleString;
	
	ssTemp << hWnd;
	ssTemp >> handleString; //handleString ora contiene il valore di hWnd trasformato in stringa
	

	//Salviamo il tipo di evento lanciato nella variabile eventString
	if (code == HSHELL_WINDOWCREATED)
	{		
		if (IsWindowVisible(hWnd)){
			eventString = L"|Created";
			sendMessage = true;
		}
	}
	if (code == HSHELL_WINDOWDESTROYED) {
		eventString = L"|Destroyed";			
		sendMessage = true;
	}
	if (code == HSHELL_WINDOWACTIVATED) {
		eventString = L"|Focused";		
		sendMessage = true;
	}

	//Se l'evento è di uno dei 3 tipi elencati sopra (creazione distruzione o focus) allora deve essere inviato il messaggio
	if (sendMessage) {
		//Messaggio da inviare tramite mailslot
		message = handleString + eventString;
		//INVIAMO AL MAILSLOT
		if (WriteSlot(hSlot, message.c_str())) {
			SetEvent(evento0);
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
