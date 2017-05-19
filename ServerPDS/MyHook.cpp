#include "stdafx.h"
#include <Windows.h>
#include <iostream>
#include <fstream>
#include "MyHook.h"


MyHook::MyHook() {
	//Dll loading and connection
	hModule = LoadLibrary(L"dll.dll");
	if (!hModule){
		std::cout << "no module loaded!";
	}	
	//Initialization of the pointer
	RunStopHook = NULL;
	RunStopHook = (RunStopHookProc*)::GetProcAddress((HMODULE)hModule, "RunStopHook");
	
};

//Singleton
MyHook& MyHook::Instance() {
		static MyHook myHook;
		return myHook;
	}


int MyHook::Messages() {
	while (msg.message != WM_QUIT) { //while we do not close our application
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		Sleep(1);
	}

	MyHook::Instance().UninstallHook();

	return (int)msg.wParam; //return the messages
}

void MyHook::InstallHook() {	
	(*RunStopHook)(true, GetModuleHandle(0));	
}

void MyHook::UninstallHook()
{
	(*RunStopHook)(false, GetModuleHandle(0));
	FreeLibrary(hModule);
	

	std::string path = "C:\\Users\\David\\Desktop\\exit.txt";
	std::fstream File(path, std::ios::app);
	File << "Chiuso!";
	File.close();
	
	
	
	
}

//Method used to start monitoring processes 
int MyHook::StartMonitoringProcesses() {
	MyHook::Instance().InstallHook();
	return MyHook::Instance().Messages();
}