// ProcessMonitorX86.cpp : definisce il punto di ingresso dell'applicazione console.
//

#include "stdafx.h"
#include <string>
#include <thread>
#include <Windows.h>
#include "MyHook.h"




int main()
{
	HANDLE terminateMonitorX86Event = OpenEvent(EVENT_ALL_ACCESS, FALSE, L"terminateMonitorX86");
	MyHook myHookObj32(L"Dll32Bit.dll");
	
	std::thread hookThread32{ &MyHook::StartMonitoringProcesses,&myHookObj32 };
	

	WaitForSingleObject(terminateMonitorX86Event, INFINITE);

	
	return 0;
}



