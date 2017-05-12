#include "stdafx.h"
#include <Windows.h>
#include <fstream>
#include <string>
#include <iostream>
#include "dll.h"

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

LRESULT CALLBACK SysMsgProc(

	int code,	// hook code
	WPARAM wParam,	// removal flag
	LPARAM lParam 	// address of structure with message
)
{
	//ѕередать сообщение другим ловушкам в системе
	CallNextHookEx(SysHook, code, wParam, lParam);

	if (code == HCBT_ACTIVATE)
	{
		TCHAR windtext[255];
		HWND Wnd = ((tagMSG*)lParam)->hwnd;
		GetWindowText(Wnd, windtext, 255);

		//You can save active file title
	}

	if (code == HCBT_CREATEWND)
	{
		TCHAR windtext[255];
		HWND Wnd = ((tagMSG*)lParam)->hwnd;
		GetWindowText(Wnd, windtext, 255);

		//You can save active file title
	}
	return 0;
}

///////////////////////////////////////////////////////////////////

DllExport void RunStopHook(bool State, HINSTANCE hInstance)
{
	if (true)
		SysHook = SetWindowsHookEx(WH_CBT, &SysMsgProc, hInst, 0);
	else
		UnhookWindowsHookEx(SysHook);
}

DllExport void Cazzo(int i) {
	std::cout << "Cazzo " << i << " volte" << std::endl;
}

DllExport LRESULT CALLBACK MouseCBK(int nCode, WPARAM wParam, LPARAM lParam) {
	MSLLHOOKSTRUCT * pMouseStruct = (MSLLHOOKSTRUCT *)lParam; // WH_MOUSE_LL struct
															  /*
															  nCode, this parameters will determine how to process a message
															  This callback in this case only have information when it is 0 (HC_ACTION): wParam and lParam contain info

															  wParam is about WINDOWS MESSAGE, in this case MOUSE messages.
															  lParam is information contained in the structure MSLLHOOKSTRUCT
															  */

	if (nCode == 0) { // we have information in wParam/lParam ? If yes, let's check it:
		if (pMouseStruct != NULL) { // Mouse struct contain information?			
			printf_s("Mouse Coordinates: x = %i | y = %i \n", pMouseStruct->pt.x, pMouseStruct->pt.y);
		}

		switch (wParam) {

		case WM_LBUTTONUP: {
			printf_s("LEFT CLICK UP\n");
		}break;
		case WM_LBUTTONDOWN: {
			printf_s("LEFT CLICK DOWN\n");
		}break;
		default:
			printf_s("CAZZO NE SO \n");
			break;
		}

	}

	/*
	Every time that the nCode is less than 0 we need to CallNextHookEx:
	-> Pass to the next hook
	MSDN: Calling CallNextHookEx is optional, but it is highly recommended;
	otherwise, other applications that have installed hooks will not receive hook notifications and may behave incorrectly as a result.
	*/
	return 0;
}

DllExport LRESULT CALLBACK WinCreateCBK(int nCode, WPARAM wParam, LPARAM lParam) {
	
	
	//if (nCode == HSHELL_WINDOWCREATED) {
	
	std::string path = "C:\\Users\\David\\Desktop\\test.txt";
		std::fstream File(path, std::ios::app);
		File << "Finestra qualcosa" << std::endl;
	//}
	
	

	

	/*
	Every time that the nCode is less than 0 we need to CallNextHookEx:
	-> Pass to the next hook
	MSDN: Calling CallNextHookEx is optional, but it is highly recommended;
	otherwise, other applications that have installed hooks will not receive hook notifications and may behave incorrectly as a result.
	*/
		return CallNextHookEx(SysHook, nCode, wParam, lParam);
	
}

DllExport void setHook(HINSTANCE hinstDLL) {
	SysHook = SetWindowsHookEx(WH_KEYBOARD, &WinCreateCBK, hinstDLL, 0);
	

}