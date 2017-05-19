#include <Windows.h>
#include "Controller.h"
#include "WindowsEnum.h"
#include "ProcessModel.h"
#include <list>
#include <thread>




//  
// Methods
//  

//Metodo di inizializzazione del controller, richiama la enumWindows per fotografare lo stato corrente dei processi attivi
//Salva l alista dei processi attivi nel model
bool Controller::Init()
{
	//Event Initialization 

	//Evento di creazione nuovo processo
	HANDLE creation = CreateEvent(
		NULL,               // default security attributes
		TRUE,               // manual-reset event
		FALSE,              // initial state is nonsignaled
		TEXT("creation")	// object name
	);
	//Evento di distruzione processo
	HANDLE destruction = CreateEvent(
		NULL,               // default security attributes
		TRUE,               // manual-reset event
		FALSE,              // initial state is nonsignaled
		TEXT("destruction")	// object name
	);
	//Evento di focus (processo portato in primo piano)
	HANDLE activation = CreateEvent(
		NULL,               // default security attributes
		TRUE,               // manual-reset event
		FALSE,              // initial state is nonsignaled
		TEXT("activation")	// object name
	);

	//Evento di ricezione messaggio da rete
	HANDLE network = CreateEvent(
		NULL,               // default security attributes
		TRUE,               // manual-reset event
		FALSE,              // initial state is nonsignaled
		TEXT("network")		// object name
	);
	//Array di eventi da passare a WaitForMultipleObject
	eventArray[0] = creation;
	eventArray[1] = destruction;
	eventArray[2] = activation;
	eventArray[3] = network;

	
	WindowsEnum we;
	we.enum_windows();

	ProcessModel model;
	model.setProcessesList(we.getData());
	
	//@TODO gestire gli errori
	return true;
}


void Controller::Run()
{
	//TROVARE IL MODO DI OTTENERE HWND CHE HA SCATENATO L'EVENTO

	std::thread netThread();
	std::thread hookThread();

	CRITICAL_SECTION CriticalSection;
	InitializeCriticalSection(&CriticalSection);


	// Request ownership of the critical section.
	EnterCriticalSection(&CriticalSection);
		

	while (true) {

		//todo implementare le ricezioni dei dati dai thread e azioni da intraprendere



		WaitForMultipleObjects();

		
		//gestisco evento 

		//Svuoto il buffer

	}

	// Release ownership of the critical section.
	LeaveCriticalSection(&CriticalSection);
	// Release resources used by the critical section object.
	DeleteCriticalSection(&CriticalSection);
}
