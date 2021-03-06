//Myhook del processo 32Bit

#ifndef MYHOOK_H
#define MYHOOK_H

class MyHook {
protected:

	typedef void (RunStopHookProc)(bool, HINSTANCE);
	RunStopHookProc* RunStopHook;
	
	HINSTANCE hModule;


public:
	MyHook(LPCTSTR);
	~MyHook();
	HHOOK hook;						// handle to the hook	
	MSG msg;						// struct with information about all messages in our hook queue
	int StartMonitoringProcesses();
	int Messages();					// function to "deal" with our messages 	
	void InstallHook();				// function to install our hook
	void UninstallHook();			// function to uninstall our hook
	

};
#endif // !MYHOOK_H
