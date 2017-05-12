#include "Controller.h"
#include "ProcessModel.h"
#include "CloseObserver.h"
#include <Windows.h>
#include <list>


Controller::Controller()
{
	CloseObserver closeOb(model, *this);
}


Controller::~Controller()
{
}

//Elenco degli oggetti da aggiungere  al model
void Controller::processAdded(HANDLE processAdded)
{
	
}

//Elenco dei processi da rimuovere dal model
void Controller::processRemoved(HANDLE processRemoved)
{
}
