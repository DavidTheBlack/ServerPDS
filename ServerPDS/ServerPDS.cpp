// ServerPDS.cpp : definisce il punto di ingresso dell'applicazione console.
//

#include "stdafx.h"
#include <Windows.h>
#include <iostream>
#include <string>
#include <list>
#include <mutex>
#include <queue>
#include <condition_variable>
#include "MyHook.h"
#include "ProcessModel.h"
#include "Controller.h"



int _tmain(int argc, _TCHAR* argv[]) {

	Controller controller;
	controller.Init();
	controller.Run();
	return 0;

}
