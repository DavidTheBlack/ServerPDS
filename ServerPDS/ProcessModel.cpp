#include "stdafx.h"
#include <mutex>
#include <algorithm>
#include <list>
#include <iostream>
#include <string>
#include <Windows.h>
#include "IconExtractor.h"

#include "ProcessModel.h"




  /**
  * Set the value of FocusedProcessPid
  * Pid del processo che attualmente ha il focus
  * @param new_var the new value of FocusedProcessPid
  */
  bool ProcessModel::setFocusedProcess(HWND processHwnd) {	 	
	  std::lock_guard<std::mutex> l(mut);
	  std::pair<HWND, DWORD> pairTemp;
	  pairTemp.first = processHwnd;
	  pairTemp.second = 0;
	  std::list<std::pair<HWND, DWORD>>::iterator it;
	  it = std::find_if(processesList.begin(), processesList.end(), 
		  [&pairTemp](const std::pair<HWND, DWORD>& element) {return element.first == pairTemp.first; }
		  );

	  if (it != processesList.end()) {
		  //elemento trovato, è possibile assegnargli il focus
		  focusProcess = processHwnd;

		  return true;
	  }
	  return false;
  }

  /**
  * Get the value of FocusedProcessPid
  * Pid del processo che attualmente ha il focus
  * @return the value of FocusedProcess HWND
  */
  HWND ProcessModel::getFocusedProcess() {
	  std::lock_guard<std::mutex> l(mut);
	  return focusProcess;
  }

  /*
  *Restituisce l'attuale numero di processi memorizzati nella struttura dati
  */
  size_t ProcessModel::getNumberOfProcesses() {
	  std::lock_guard<std::mutex> l(mut);
	  return processesList.size();
  }

  /**
   * Restituisce la struttura dati condivisa processList
   * @return std::list<std::pair<HWND,DWORD>>
   */
  std::list<std::pair<HWND,DWORD>> ProcessModel::getProcessesList()
  {
	std::lock_guard<std::mutex> l(mut);
    return processesList;
  }

  /**
   * Aggiunta safe di un nuovo processo
   * @return bool
   * @param  processInfo
   */
  bool ProcessModel::addProcess (HWND processHwnd)
  {
	  //1) lock della struttura dati
	  //2) inserimento nuovo processo
	  //3) verifica corretto inserimento
	  std::lock_guard<std::mutex> l(mut);
	  std::list<std::pair<HWND,DWORD>>::iterator it;
	  //Verifichiamo che il processo non sia già presente nella struttura dati
	  std::pair<HWND, DWORD> pairTemp;
	  pairTemp.first = processHwnd;
	  GetWindowThreadProcessId(processHwnd, &pairTemp.second);
	  	 	  
	  it = std::find_if(processesList.begin(), processesList.end(),
		  [&pairTemp](const std::pair<HWND, DWORD>& element) {return element==pairTemp; }
	  );

	  if (it == processesList.end()) {
		  //elemento non trovato iteratore "nullo"
		  processesList.push_back(pairTemp);
		  return true;
	  }
	  return false;
  }

  HWND ProcessModel::pidToHwnd(DWORD pid)
  {
	  //1) lock della struttura dati
	  //2) ricerca del processo
	  //3) restituisce hwnd
	  std::lock_guard<std::mutex> l(mut);
	  std::list<std::pair<HWND, DWORD>>::iterator it;
	  
	  //Verifichiamo che il processo sia presente nella struttura dati
	  it = std::find_if(processesList.begin(), processesList.end(),
		  [&pid](const std::pair<HWND, DWORD>& element) {return element.second == pid; }
	  );

	  //Se trova elemento restituisce handle processo
	  if (it != processesList.end()) {
		  return (*it).first;
	  }
	  return 0;
  }

  DWORD ProcessModel::hwndToPid(HWND hWnd)
  {
	  //1) lock della struttura dati
	  //2) ricerca del processo
	  //3) restituisce pid
	  std::lock_guard<std::mutex> l(mut);
	  std::list<std::pair<HWND, DWORD>>::iterator it;
	  //Verifichiamo che il processo sia presente nella struttura dati
	  it = std::find_if(processesList.begin(), processesList.end(),
		  [&hWnd](const std::pair<HWND, DWORD>& element) {return element.first == hWnd; }
	  );

	  //Se trova elemento restituisce il pid processo
	  if (it != processesList.end()) {
		  return (*it).second;
	  }

	  return 0;

  }

  bool ProcessModel::setProcessesList(std::list<HWND> list) {	  
	  if (processesList.empty()) {
		  //Elaboriamo la lista passata dalla window enum e la aggiungiamo alla struttura dati corrente
		  std::pair<HWND, DWORD> tempPair;
		  std::list<HWND>::iterator it;
		  std::lock_guard<std::mutex> l(mut);
		  
		  for (it = list.begin(); it != list.end(); ++it) {
			  tempPair.first = *it;
			  GetWindowThreadProcessId(*it, &tempPair.second);
			  processesList.push_back(tempPair);
		  }		  		  		  		 	
		  return true;
	  }
	  else {
		  return false;
	  }
  }

  /**
   * Rimozione safe di un processo dalla struttura dati
   * @return bool
   * @param  processHwnd
   */
  bool ProcessModel::removeProcess (HWND processHwnd)
  {
	  /**
	  * 1) richiede il lock sul processo
	  * 2) ricerca l'elemento nella lista processi
	  * 3) se lo trova lo rimuove dalla lista

	  */
	  std::lock_guard<std::mutex> l(mut);
	  std::list<std::pair<HWND,DWORD>>::iterator it;
	  std::pair<HWND, DWORD> pairTemp;
	  pairTemp.first = processHwnd;
	  pairTemp.second = 0;
	  it = std::find_if(processesList.begin(), processesList.end(), 
		  [&pairTemp](const std::pair<HWND, DWORD>& element) {return element.first == pairTemp.first; }		  		  
		  );
	  if (it != processesList.end()) {
		  //elemento trovato, è possibile rimuoverlo dalla lista processi
		  processesList.erase(it);
		  return true;
	  }
	  return false;
  }

  ProcessModel::processInfo ProcessModel::getProcessInfo(HWND hWnd)
  {
	  //1) Lock the data structure
	  //2) Retrieve the process information
	  std::lock_guard<std::mutex> l(mut);
	  
	  HANDLE hProcess;
	  ProcessModel::processInfo pI;



	GetWindowThreadProcessId(hWnd, &std::get<0>(pI)); //retrieves the identifier of the process that created the window



	if ((hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,
		FALSE,
		std::get<0>(pI))) == NULL) // open an existing local process object returning an open handle to the specified process
	{
		std::cerr << "OpenProcess() failed: " << GetLastError() << "\n " << std::get<0>(pI) << std::endl;
	}
	else
	{
		TCHAR name[MAX_PATH];
		DWORD nameLength = MAX_PATH;
		if (!QueryFullProcessImageName(hProcess, 0, name, &nameLength))  //retrieves the full name of the executable image for the specified process
		{
			std::cerr << "QueryFullProcessImageName() failed: " <<
				GetLastError() << "\n";
		}
		else
		{
			std::get<3>(pI) = std::wstring(name);
			CloseHandle(hProcess);
		}
	}

	//Grab the titlebar information
	int length = GetWindowTextLength(hWnd) + 1;
	if (length >1 )  { //We count also the terminator character
		TCHAR* titleBar = new TCHAR[length];		  
		GetWindowText(hWnd, titleBar, length);
		std::wstring windowTitle(titleBar);		 
		std::get<2>(pI) = windowTitle;
		//Delete the dinamic buffer
		delete[] titleBar;
	}
	else {
		std::get<2>(pI) = L"Finestra senza titolo";
	}
	  

		//Extract the icon information
		std::string stringIcon;
	
		if (iconExtrObj.ExtracttIcon(std::get<3>(pI), stringIcon) == NO_ERROR) {
			std::get<4>(pI) = stringIcon;
		}
		else {
			//If there is no icon save NoIcon String
			std::get<4>(pI) = "NoIcon";
		}
	  
	  return pI;

	  	  
  }

  //Restituisce una lisa di tutti i processi presenti nel pc con le informazioni relative ad ogni processo
  std::list<ProcessModel::processInfo> ProcessModel::getProcessesInfo()
  {	 	  	  
	  std::list<ProcessModel::processInfo> processesInfoList;

	  std::list<std::pair<HWND, DWORD>>::iterator it;
	  //Popolo la lista delle informazioni per ogni processo
	  for (it = processesList.begin(); it != processesList.end(); ++it) {
		  processesInfoList.push_back( getProcessInfo(it->first));
	  }

	  return processesInfoList;		 
  }


