#include "stdafx.h"
#include <Windows.h>
#include <iostream>
#include <fstream>
#include "MyHook.h"
#include "Buffer.h"


MyHook::MyHook() {
	//Dll loading and connection
	hModule = LoadLibrary(L"dll.dll");
	if (!hModule){
		std::cout << "no module loaded!";
	}	
	//Initialization of the pointer to RunStopHook function
	RunStopHook = NULL;
	RunStopHook = (RunStopHookProc*)::GetProcAddress((HMODULE)hModule, "RunStopHook");
	//Initialization of the pointer to the set dll parameters function
	SetDllParam = NULL;
	SetDllParam = (SetDllParameters*)::GetProcAddress((HMODULE)hModule, "SetDllParameters");
	
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
	//Setto i parametri della nela dll e poi chiamo la funzione di avvio aggancio hook
	(*SetDllParam)(creation,destruction,activation,network,buffer);
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
void MyHook::SetParameters(HWND creation, HWND destruction, HWND activation, HWND network, Buffer *b)
{
	this->creation = creation;
	this->destruction = destruction;
	this->activation = activation;
	this->network = network;
	this->buffer = b;
}





//Method used to start monitoring processes 
int MyHook::StartMonitoringProcesses() {


	MyHook::Instance().InstallHook();
	return MyHook::Instance().Messages();
}