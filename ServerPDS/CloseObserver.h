
#pragma once
class CloseObserver
{

private:
	//Riferimenti Controller e Model
	ProcessModel model;
	Controller controller;


	//Evento di notifica della variazione del model
	HANDLE modelUpdated;
	std::list<HANDLE> pList;
	//bool updateEventSubscribed;
public:
	CloseObserver(const ProcessModel&,const  Controller&);
	void CheckClosedProcesses(std::promise<HANDLE>& p); //std::list<HWND>, std::promise<HANDLE>&
	//Metodo update del pattern Observer
	void update();

	//CloseObserver(ProcessModel m);
	//void subscribeUpdateEvent(HANDLE);
	//void CheckClosedProcesses(std::promise<HANDLE>& p);
	

};

