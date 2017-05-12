#include <Windows.h>
#include "Controller.h"
#include "WindowsEnum.h"
#include "ProcessModel.h"
#include <list>




//  
// Methods
//  

//Metodo di inizializzazione del controller, richiama la enumWindows per fotografare lo stato corrente dei processi attivi
//Salva l alista dei processi attivi nel model
bool Controller::Init()
{
	WindowsEnum we;
	we.enum_windows();

	ProcessModel model;
	model.setProcessesList(we.getData());
	


	return false;
}
