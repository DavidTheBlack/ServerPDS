// ServerPDS.cpp : definisce il punto di ingresso dell'applicazione console.
//

#include "stdafx.h"
#include <Windows.h>
#include <iostream>
#include <string>
#include <list>
#include <mutex>
#include <condition_variable>
#include <queue>
#include "MsgQueue.h"
#include "MyHook.h"
#include "ProcessModel.h"
#include "Controller.h"



int _tmain(int argc, _TCHAR* argv[]) {
	MyHook::Instance();
	Controller controller;
	controller.Init();
	controller.Run();
	return 0;

}
