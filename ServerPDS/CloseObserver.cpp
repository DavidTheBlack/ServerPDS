#include "CloseObserver.h"
#include "ProcessModel.h"
#include "Controller.h"
#include <Windows.h>
#include <list>
#include <algorithm>
#include <future>



	//Inizializzo la lista dei processi da osservare 
	CloseObserver::CloseObserver(const ProcessModel& m, const Controller& c)
	{
		model = m;
		controller = c;
		modelUpdated = CreateEvent(NULL, TRUE, FALSE, NULL);
		ResetEvent(modelUpdated);
	}

	//Metodo da richiamare per settare l'evento che dovrà far ritornare in maniera forzata la WFMO
	/*void CloseObserver::subscribeUpdateEvent(HANDLE updateEv)
	{
		updateEvent = updateEv;
		updateEventSubscribed = true;
	}*/

	//Metodo che andrà in loop su thread separato che segnalerà la chiusura dei processi
	void CloseObserver::CheckClosedProcesses(std::promise<HANDLE>& p)
	{
		
		/*
		* 1) Popola la lista dei processi in esecuzione
		* 2) Lancia la waitForMultipleObject
		* 3) In caso la wfmo torni segnala la chiusura al chiamante
		*/
		//Copying processes list into array c style
		

		HANDLE *processesArray =new HANDLE[model.getProcessesList().size() + 1];
		

		//Indice della posizione nel vettore dell'evento di segnalazione per uscita forzata dalla WFMO
		int updateEventIndex = model.getProcessesList().size();

		std::copy(model.getProcessesList().begin(), model.getProcessesList().end(), processesArray);
		processesArray[updateEventIndex] = modelUpdated;

		if (model.getProcessesList().size() > 63) {
			//TODO solleva eccezione numero processi da monitorare eccessivo
		}

		//Rimaniamo in attesa 
		DWORD dwEvent= WaitForMultipleObjects(model.getProcessesList().size()+1, processesArray, FALSE, INFINITE);

		//Se wfmo tornata a causa del set evento model update
		//aggiorno la lista dei processi e riparto ad osservare

		//Capisco quale processo è stato chiuso
		dwEvent = dwEvent - WAIT_OBJECT_0;

		HANDLE processRem = processesArray[dwEvent];
		
		//Cancello memoria usata dal puntatore 
		delete[] processesArray;

		//Notifico al controller i processi rimossi
		p.set_value(processesArray[dwEvent]);



		
	}

	//Metodo che viene richiamato dal controller per notificare che il model è cambiato
	void CloseObserver::update()
	{
		/* 
		1) Uscire dalla wait for multiple object settando evento update
		*/
		SetEvent(modelUpdated);
	}




