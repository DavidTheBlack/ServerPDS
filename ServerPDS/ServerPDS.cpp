// ServerPDS.cpp : definisce il punto di ingresso dell'applicazione console.
//

#include "stdafx.h"
#include <Windows.h>
#include <iostream>
#include <string>

#include "MyHook.h"
#include "WindowsEnum.h"


int _tmain(int argc, _TCHAR* argv[]) {

	MyHook::Instance().StartMonitoringProcesses();

}
