#include "stdafx.h"
#include <Windows.h>
#include <list>
#include <iostream>
#include <algorithm>
#include "WindowsEnum.h"





WindowsEnum & WindowsEnum::enum_windows()
{
	if (!EnumWindows(enumWindowsProc, reinterpret_cast<LPARAM>(this))) {
		std::cout << "enumwindows failed!" << GetLastError() << "\n";
	}
	return *this;       //returns a reference
}

std::list<HWND>& WindowsEnum::getData()
{
	return localProcessList;   //returns a reference
}

BOOL WindowsEnum::IsAltTabWindow(HWND hwnd)
{
	TITLEBARINFO ti;
	HWND hwndTry, hwndWalk = NULL;

	//If the window is not visible then is not a window of interest
	if (!IsWindowVisible(hwnd))
		return FALSE;


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
		return FALSE;


	// Get the title bar information for current window and check if is invisible
	// in this way removes some task tray programs and "Program Manager"
	ti.cbSize = sizeof(ti);
	GetTitleBarInfo(hwnd, &ti);
	if (ti.rgstate[0] & STATE_SYSTEM_INVISIBLE)
		return FALSE;


	// Check if the window is a ToolWindow -> ALT+SPACE or Right Click Menu 
	// Tool windows should not be displayed either - these do not appear in the taskbar
	if (GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_TOOLWINDOW)
		return FALSE;

	//The windows is an active window that can be selected with ALT+TAB
	return TRUE;
}

BOOL WindowsEnum::wndProc(HWND hWnd)
{

	//Se è un processo con finestra salva nella lista locale.
	if (IsAltTabWindow(hWnd)) {
		localProcessList.push_back(hWnd);
	}
	return TRUE;
}

BOOL WindowsEnum::enumWindowsProc(HWND hWnd, LPARAM lParam)
{
	WindowsEnum* pThis = reinterpret_cast<WindowsEnum*>(lParam);
	return pThis->wndProc(hWnd);
}




