#include "stdafx.h"
#include <thread>
#include <iostream>
#include <string>
#include <list>
#include <mutex>
#include <Windows.h>
#include <queue>
#include <fstream>
#include "MsgQueue.h"
#include "WindowsEnum.h"
#include "MyHook.h"
#include "ProcessModel.h"
#include "Controller.h"




//  
// Methods
//  

//Metodo di inizializzazione del controller, richiama la enumWindows per fotografare lo stato corrente dei processi attivi
//Salva l alista dei processi attivi nel model
bool Controller::Init()
{
	WindowsEnum we;
	we.enum_windows();
	model.setProcessesList(we.getData());
	//Setto la message queue da usare per le comunicazioni con la dll
	
	MyHook::Instance().SetMessageQueue(&messageQueue);
	

	//@TODO gestire gli errori
	return true;
}


void Controller::Run()
{
	//TROVARE IL MODO DI OTTENERE HWND CHE HA SCATENATO L'EVENTO

	//std::thread netThread();
	std::thread hookThread{ &MyHook::StartMonitoringProcesses,&MyHook::Instance() };

	//Lanciare in un altro thread
	
	
	std::unique_lock<std::mutex> lock(messageQueue.eventMut);
	//GESTIONE DEGLI EVENTI CHE VENGONO SCRITI NELLA MESSAGE QUEUE
	while (true) {
		std::cout << "SIamo Nel while" << std::endl;
		messageQueue.eventCondVar.wait(lock);
		std::cout << "tornati dalla condition variable " << std::endl;
		lock.unlock();
		while (true) {
			lock.lock();
			if (messageQueue.eventQueue.empty()) {
				break;
			}
			else {
				std::cout << "Processo: " << messageQueue.eventQueue.front().processHandle << " evento: " << messageQueue.eventQueue.front().eventTriggered << std::endl;
				messageQueue.eventQueue.pop();
			}
			lock.unlock();
		}
		

		
		//todo implementare le ricezioni dei dati dai thread e azioni da intraprendere

		
	}

	
}
