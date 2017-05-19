#pragma once

class MyHook {
private:

	typedef void (RunStopHookProc)(bool, HINSTANCE);
	RunStopHookProc* RunStopHook;

	typedef void (SetDllParameters)(HANDLE, HANDLE, HANDLE, HANDLE, Buffer);
	SetDllParameters* SetDllParam;

	HINSTANCE hModule;
	MyHook();
	Buffer* buffer;
	//Evento di creazione nuovo processo
	HANDLE creation;
	//Evento di distruzione processo
	HANDLE destruction;
	//Evento di focus (processo portato in primo piano)
	HANDLE activation;
	//Evento di ricezione messaggio da rete
	HANDLE network;


public:
	//singleton
	static MyHook& Instance();
	HHOOK hook; // handle to the hook	
	MSG msg; // struct with information about all messages in our queue
	int StartMonitoringProcesses();
	int Messages(); // function to "deal" with our messages 	
	void InstallHook(); // function to install our hook
	void UninstallHook(); // function to uninstall our hook
	void SetParameters(HWND, HWND, HWND, HWND, Buffer*);	//method used to pass the event and buffer to MyHook Classes
	
};
