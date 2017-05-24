#pragma once

class MyHook {
private:

	typedef void (RunStopHookProc)(bool, HINSTANCE);
	RunStopHookProc* RunStopHook;
	
	typedef void (SetMessageQueueProc)(MessageQueue*);
	SetMessageQueueProc* SetDllMsgQueue;

	HINSTANCE hModule;
	MessageQueue* msgQueue;
	MyHook();
	
	


public:
	//singleton
	static MyHook& Instance();
	HHOOK hook; // handle to the hook	
	MSG msg; // struct with information about all messages in our queue
	int StartMonitoringProcesses();
	int Messages(); // function to "deal" with our messages 	
	void InstallHook(); // function to install our hook
	void UninstallHook(); // function to uninstall our hook
	void SetMessageQueue(MessageQueue * msgQueue); //method used to set the model 
	
	
};
