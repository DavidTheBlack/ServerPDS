// ProcessMonitorX86.cpp : definisce il punto di ingresso dell'applicazione console.
//

#include "stdafx.h"
#include <string>
#include <thread>
#include <Windows.h>
#include "IconExtractor.h"
#include "MailSlotObj.h"
#include "MyHook.h"


void iconThreadFunc();
std::wstring string2wstring(const std::string& s);

int main()
{
	HANDLE terminateMonitorX86Event = OpenEvent(EVENT_ALL_ACCESS, FALSE, L"terminateMonitorX86");
	MyHook myHookObj32(L"..\\x64\\Debug\\32BitCode\\Dll32Bit.dll");
	
	std::thread hookThread32{ &MyHook::StartMonitoringProcesses,&myHookObj32 };
	std::thread iconThread32{ iconThreadFunc };

	WaitForSingleObject(terminateMonitorX86Event, INFINITE);

	
	return 0;
}

void iconThreadFunc() {
	/* 1 Creare tutte le variabili del caso
	2 entrare in un while
	3 attendere segnalazione eventi
	4 estrarre icona
	4.5 resettare evento richiesta icona
	4.6 inserire nel mail slot del server
	5 segnalare con evento
	6 quando esce dal while deallocare tutti gli handle
	*/

	CIconExtractor iconExtrObj;


	//Eventi di comunicazione icone
	HANDLE eventAskIcon = OpenEvent(EVENT_ALL_ACCESS, FALSE, L"eventAskIcon");

	//Signaled when icon is ready to be read from the mailslot
	HANDLE eventReadyIcon = OpenEvent(EVENT_ALL_ACCESS, FALSE, L"eventReadyIcon");

	//Terminate process event
	HANDLE terminateMonitorX86Event = OpenEvent(EVENT_ALL_ACCESS, FALSE, L"terminateMonitorX86");


	LPTSTR mailSlotServer = L"\\\\.\\mailslot\\ServerSlot";
	LPTSTR mailSlotX86Process = L"\\\\.\\mailslot\\X86Slot";

	MailSlotObj serverMailSlot(mailSlotServer, false);
	MailSlotObj processX86MailSlot(mailSlotX86Process, true);

	HANDLE events[2];
	events[0] = eventAskIcon;	
	events[1] = terminateMonitorX86Event;

	DWORD eventRaised;
	bool terminateProcess = false;
	//Icon data string
	std::string stringIcon;
	std::wstring iconMex;

	//Process Path
	std::wstring path;


	while (!terminateProcess) {
		eventRaised = WaitForMultipleObjects(2, events, FALSE, INFINITE);
		switch (eventRaised)
		{
		
		case 0: { //richiesta icona
			if (processX86MailSlot.ReadSlot()) {
				path = processX86MailSlot.getMessageW();
				if (iconExtrObj.ExtracttIcon(path, stringIcon) == NO_ERROR) {
					iconMex = string2wstring(stringIcon);
					serverMailSlot.WriteSlot(iconMex.c_str());
				}
				else {
					//If there is no icon save NoIcon String
					serverMailSlot.WriteSlot(L"NoIcon");					
				}
			}
			else {
				serverMailSlot.WriteSlot(L"NoIcon");
			}
			ResetEvent(eventAskIcon);
			SetEvent(eventReadyIcon);
			break;
		}
		case 1: { //terminazione del processo
			terminateProcess = true;
			break;
		}
		default:
			break;
		}
	}

	CloseHandle(eventAskIcon);
	CloseHandle(eventReadyIcon);
}

std::wstring string2wstring(const std::string& s)
{
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}





