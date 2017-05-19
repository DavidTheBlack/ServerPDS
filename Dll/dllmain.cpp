// dllmain.cpp: definisce il punto di ingresso per l'applicazione DLL.
#include "stdafx.h"
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <Strsafe.h>
#include "dll.h"
#include "Buffer.h"

HHOOK SysHook;
HINSTANCE hInst;



BOOL APIENTRY DllMain(HANDLE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	hInst = (HINSTANCE)hModule;
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

void savetofile2(const char* path, HWND hWnd, char* c, CREATESTRUCT *pcs) {
	std::fstream File(path, std::ios::app);
	if (File.is_open())
	{
		LPSTR result = NULL;
		int len = WideCharToMultiByte(CP_UTF8, 0, pcs->lpszName, -1, NULL, 0, 0, 0);

		if (len > 0)
		{
			result = new char[len + 1];
			if (result)
			{
				int resLen = WideCharToMultiByte(CP_UTF8, 0, pcs->lpszName, -1, &result[0], len, 0, 0);
				if (resLen == len)
				{
					File << "window has been" << c << hWnd << ", ";
					File.write(result, len);
					//File << (pcs->style & WS_CAPTION);
					File << std::endl;
				}
				delete[] result;
			}
		}
	}
	File.close();
}


LRESULT CALLBACK CBTProc(

	int code,	// hook code
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

				savetofile2("C:\\Users\\dar_w\\Desktop\\report.txt", hWnd, " created: ", pcs);
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


LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam,
	LPARAM lParam)
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

	int code,	// hook code
	WPARAM wParam,	// removal flag
	LPARAM lParam, 	// address of structure with message
	Buffer buff
)
{

	CallNextHookEx(SysHook, code, wParam, lParam);

	if (code == HSHELL_WINDOWCREATED)
	{
		HWND hWnd = (HWND)wParam;
		if (IsWindowVisible(hWnd)){
			savetofile("C:\\Users\\David\\Desktop\\report.txt", hWnd, " created: ");
		}
	}
	if (code == HSHELL_WINDOWDESTROYED) {
		HWND hWnd = (HWND)wParam;
		
			savetofile("C:\\Users\\David\\Desktop\\report.txt", hWnd, " destroyed: ");
	}
	if (code == HSHELL_WINDOWACTIVATED) {
		HWND hWnd = (HWND)wParam;

		savetofile("C:\\Users\\David\\Desktop\\report.txt", hWnd, " activated: ");


	}
	return 0;
}




///////////////////////////////////////////////////////////////////

DllExport void RunStopHook(bool State, HINSTANCE hInstance)
{
	if (State)
		SysHook = SetWindowsHookEx(WH_SHELL, &ShellProc, hInst, 0);
	else{
		UnhookWindowsHookEx(SysHook);
	}
		
}