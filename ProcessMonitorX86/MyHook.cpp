//Myhook del processo 32Bit

#include "stdafx.h"

#include <Windows.h>
#include <iostream>
#include <fstream>
#include "MyHook.h"




MyHook::MyHook(LPCTSTR path): RunStopHook(NULL) {
	//Dll loading and connection
	hModule = LoadLibrary(path);
	if (!hModule){
		std::cout << "no module loaded!" << path << std::endl;
	}	
	//Initialization of the pointer to RunStopHook method	
	RunStopHook = (RunStopHookProc*)::GetProcAddress((HMODULE)hModule, "RunStopHook32");	
}

MyHook::~MyHook()
{
	UninstallHook();
	FreeLibrary(hModule);
}

int MyHook::Messages() {
	while (msg.message != WM_QUIT) { //while we do not close our application
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		Sleep(1);
	}

	

	return (int)msg.wParam; //return the messages
}

//Method used to start monitoring processes 
int MyHook::StartMonitoringProcesses() {	
	InstallHook();
	return Messages();
}

void MyHook::InstallHook() {
	//Chiamo la funzione di aggancio hook	
	(*RunStopHook)(true, GetModuleHandle(0));
	
}

void MyHook::UninstallHook()
{
	(*RunStopHook)(false, GetModuleHandle(0));
	FreeLibrary(hModule);

	
}

