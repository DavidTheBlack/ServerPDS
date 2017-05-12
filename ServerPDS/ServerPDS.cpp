// ServerPDS.cpp : definisce il punto di ingresso dell'applicazione console.
//

#include "stdafx.h"
#include <Windows.h>
#include <iostream>
#include <string>

#include "MyHook.h"

/*	
	Check if the window identified from hwnd 
	handler is a visible window that can be 
	selected with ALT+TAB key combination
	*/
BOOL IsAltTabWindow(HWND hwnd)
{
		
		TITLEBARINFO ti;
		HWND hwndTry, hwndWalk = NULL;
		
		//If the window is not visible then is not a window of interest
		if (!IsWindowVisible(hwnd))
			return false;
	
		
		//Check and ignore the PopUp and dialog windows 
		//For each visible window, walk up its owner chain until you find the root owner.
		//Then walk back down the visible last active popup chain until you find a visible window. 
		//If you're back to where you're started, then put the window in the Alt+Tab list.
		hwndTry = GetAncestor(hwnd, GA_ROOTOWNER);
		while (hwndTry != hwndWalk)
		{
			hwndWalk = hwndTry;
			//Se la finestra non ha richiamato altre sottofinestre la GetLastActivePopup restituisce 
			//l'handler della finestra stessa quindi il ciclo si interrompe
			hwndTry = GetLastActivePopup(hwndWalk);
			if (IsWindowVisible(hwndTry))
				break;
		}

		//If hwnd is an handler from a "root" window (ancestor) the analysis proceeds
		//otherwise the window is ignored
		if (hwndWalk != hwnd)
			return false;
		

		// Get the title bar information for current window and check if is invisible
		// in this way removes some task tray programs and "Program Manager"
		ti.cbSize = sizeof(ti);
		GetTitleBarInfo(hwnd, &ti);
		if (ti.rgstate[0] & STATE_SYSTEM_INVISIBLE) {
			return false;
		}
			
		// Check if the window is a ToolWindow -> ALT+SPACE or Right Click Menu 
		// Tool windows should not be displayed either - these do not appear in the taskbar
		if (GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_TOOLWINDOW) 
			return false;
		
		//The windows is an active window that can be selected with ALT+TAB
		return true;
	}


BOOL CALLBACK enumWindowsProc(
	__in  HWND hWnd,
	__in  LPARAM lParam
) {
		if (IsAltTabWindow(hWnd)) {
			int length = GetWindowTextLength(hWnd);
			if (length != 0) {
				TCHAR* buffer;
				buffer = new TCHAR[length + 1];
				memset(buffer, 0, (length + 1) * sizeof(char));

				GetWindowText(hWnd, buffer, length + 1);
				std::wstring windowTitle = std::wstring(buffer);
				delete[] buffer;
				std::wcout << hWnd << TEXT(": ") << windowTitle << std::endl;
				

			}
			else {
				std::wcout << hWnd << TEXT(": Finestra senza titolo") << std::endl;
			}
		}

		
	
	return true;
}



//
//	
//	
//
//int _tmain(int argc, _TCHAR* argv[]) {
//	/*std::wcout << TEXT("Enumerating Windows...") << std::endl;
//	BOOL enumeratingWindowsSucceeded = EnumWindows(enumWindowsProc, NULL);*/
//	//int pid= std::cin.get();
//	/*
//	int pid;
//	while (1) {
//		std::cout << "Inserisci il PID del processo da controllare: ";
//		std::cin >> pid ;
//
//
//		HANDLE hnd = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
//		DWORD lpExitCode;
//		bool rt = GetExitCodeProcess(hnd, &lpExitCode);
//		if (rt) {
//			std::cout << "Valore ritornato TRUE ";
//		}
//		else {
//			std::cout << "Valore ritornato FALSE ";
//		}
//
//
//		std::cout << " lpExitCode: " << lpExitCode << std::endl;
//		bool condition = true;
//		while (!condition) {
//			rt = GetExitCodeProcess(hnd, &lpExitCode);
//
//			if (lpExitCode != STILL_ACTIVE) {
//				std::cout << "PROCESSO CHIUSO! - Avvio nuova instanza di notepad" << std::endl;
//				system("notepad.exe");
//				condition = false;
//			}
//		}
//	}
//		
//	std::cin >> pid;
//	*/
//
//	HANDLE  processList[10];
//	HANDLE uscita;
//	HWND prova;
//
//	uscita = CreateEvent(NULL, TRUE, FALSE, NULL);
//	processList[0] = uscita;
//
//	WaitForMultipleObjects(std::size(processList), processList, FALSE, INFINITE);
//
//	/*
//	1) thread2.evento=uscita;
//	1.5) WHILE(TRUE)
//	2) startThread( thread2.observe());
//	3) quando succede qualcosa uscita=true;
//	4) elaboro le mie cose
//	5) evento=false;
//
//	
//	*/
//
//
//
//	return 0;
//	
//}

int _tmain(int argc, _TCHAR* argv[]) {
	MyHook::Instance().StartMonitoringProcesses();

}
