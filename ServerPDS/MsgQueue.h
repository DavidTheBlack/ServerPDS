#pragma once

typedef struct EventInfo
{
	HWND processHandle;
	int eventTriggered;
}EventInfo;

class MessageQueue
{
public:	
	//Coda messaggi e condition variable e mutex relativi
	std::queue<EventInfo> eventQueue;
	std::condition_variable eventCondVar;
	std::mutex eventMut;
};




