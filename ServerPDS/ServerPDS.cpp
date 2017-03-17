// ServerPDS.cpp : definisce il punto di ingresso dell'applicazione console.
//

#include "stdafx.h"
#include <Windows.h>
#include <iostream>
#include <string>


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
		if (GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_TOOLWINDOW) {
			return false;
		}
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

	
	

int _tmain(int argc, _TCHAR* argv[]) {
	std::wcout << TEXT("Enumerating Windows...") << std::endl;
	BOOL enumeratingWindowsSucceeded = EnumWindows(enumWindowsProc, NULL);
	std::cin.get();
	return 0;
	
}
