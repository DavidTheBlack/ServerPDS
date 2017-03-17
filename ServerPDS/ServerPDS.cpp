// ServerPDS.cpp : definisce il punto di ingresso dell'applicazione console.
//

#include "stdafx.h"
#include <Windows.h>
#include <iostream>
#include <string>


BOOL CALLBACK enumWindowsProc(
	__in  HWND hWnd,
	__in  LPARAM lParam
) {
	if (IsWindowVisible(hWnd)) {
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
		return true;
	}
	else {
		//ShowWindow(hWnd, SW_SHOW);
		//std::wcout << hWnd << TEXT(": Icona") << std::endl;
		return true;
	}
	
	/*if (!IsIconic(hWnd)) {
		int length = GetWindowTextLength(hWnd);
		//if (0 == length) return TRUE;

		TCHAR* buffer;
		buffer = new TCHAR[length + 1];
		memset(buffer, 0, (length + 1) * sizeof(char));

		GetWindowText(hWnd, buffer, length + 1);
		std::wstring windowTitle = std::wstring(buffer);
		delete[] buffer;

		std::wcout << hWnd << TEXT(": ") << windowTitle << std::endl;
		*/
	}
	

int _tmain(int argc, _TCHAR* argv[]) {
	std::wcout << TEXT("Enumerating Windows...") << std::endl;
	BOOL enumeratingWindowsSucceeded = EnumWindows(enumWindowsProc, NULL);
	std::cin.get();
	return 0;
	
}
