// ServerPDS.cpp : definisce il punto di ingresso dell'applicazione console.
//

#include "stdafx.h"
#include <Windows.h>
#include <iostream>
#include <string>
#include <Psapi.h>
#include <vector>
#include <algorithm>
#include <set>
#include <iterator>




std::set<DWORD> newset;



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


BOOL CALLBACK enumWindowsProc(
	__in  HWND hWnd,
	__in  LPARAM lParam
) {
	if (IsAltTabWindow(hWnd)) {
		DWORD processId;
		GetWindowThreadProcessId(hWnd, &processId);

		/*int length = GetWindowTextLength(hWnd) + 1;
		if (length != 0) {
			std::wstring windowTitle(length, '\0');
			GetWindowText(hWnd, &windowTitle[0], length);
		*/
			//newset.push_back(windowTitle);
			//Inseriamo il pid del processo nel set
			newset.insert(processId);
		}	
	return TRUE;
}


int _tmain(int argc, _TCHAR* argv[]) {
	std::set<DWORD> oldset;
	std::vector<DWORD> diffset;
	std::vector<DWORD> closeSet;
	std::vector<DWORD> openSet;
	std::vector<DWORD>::iterator it;
	


	bool x = false;
	bool isremoved = false;
	bool isadded = false;
	int agg=0;
	int rim = 0;
	
	while (1) {
		
		BOOL enumeratingWindowsSucceeded = EnumWindows(enumWindowsProc, NULL);
		

		
		//Uguaglianza da fare al primo ciclo perchè oldset è vuoto
		if (!x) {
		//	oldset.resize(newset.size());
			oldset = newset;
			x = true;
		}

		std::set_symmetric_difference(oldset.begin(), oldset.end(), newset.begin(), newset.end(), std::back_inserter(diffset));

		//Distinguo i programmi aperti da quelli chiusi
		for (it = diffset.begin(); it != diffset.end(); it++) {

			
			if (oldset.find(*it)!=oldset.end()) {
				std::cout << "Processo Rimosso: " << *it << std::endl;
				rim++;
			}
			else {
				std::cout << "Processo Aggiunto: " << *it << std::endl;
				agg++;
			}
		}
		
		
		
		if (diffset.size() != 0) {
			std::cout << "dimensione oldset " <<oldset.size()<<std::endl;
			std::cout << "dimensione newset " << newset.size() << std::endl;
			std::cout << "Differenze totali: " << diffset.size() << std::endl;
			std::cout << "Aggiunti: "<<agg<<" rimossi:"<<rim << std::endl;
			/*std::cout << "Pid modificati: " << std::endl;
			for (it = diffset.begin(); it != diffset.end(); it++)
				std::cout << ' ' << *it;
			std::cout << '\n';
			*/

		}
		
		

		//std::sort(newset.begin(), newset.end());
		//std::sort(oldset.begin(), oldset.end());

		//if (newset.size() == oldset.size()) {
		//	if (newset != oldset) {
		//		oldset = newset;
		//}		
		//}
		//else if (newset.size() < oldset.size()) {
		//	
		//	isremoved = false;
		//	if (!isremoved) {
		//	std::set_difference(oldset.begin(), oldset.end(), newset.begin(), newset.end(),
		//		back_inserter(diffset));
		//	for (int i = 0; i < diffset.size(); i++){
		//		std::wcout << "element removed: " << diffset[i] << std::endl;
		//		std::vector<std::wstring>::iterator it = find(oldset.begin(), oldset.end(), diffset[i]);
		//		oldset.erase(it);
		//		}
		//		//oldset.erase(std::remove(oldset.begin(), oldset.end(), diffset[0]), oldset.end());  //possibile problema dei doppioni
		//		isremoved = true;
		//	}		
		//}
		//else {
		//	isadded = false;
		//	if (!isadded) {
		//	std::set_difference(newset.begin(), newset.end(), oldset.begin(), oldset.end(),
		//		back_inserter(addset));
		//	for (int i = 0; i < addset.size(); i++) {
		//		std::wcout << "element added: " << addset[i] << std::endl;
		//		//oldset.insert(addset[0]);
		//		oldset.push_back(addset[i]);
		//	
		//	}
		//	
		//	
		//		isadded = true;
		//	}
		//}
			
			oldset = newset;
			newset.clear();
			diffset.clear();
			Sleep(500);
	}
	std::cin.get();
	return 0;

}
