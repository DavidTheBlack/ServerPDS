#include "stdafx.h"
#include <mutex>
#include <algorithm>
#include <list>
#include <Windows.h>
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
  * @return the value of FocusedProcessPid
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
	  
	  //Verifichiamo che il processo non sia già presente nella struttura dati
	  it = std::find_if(processesList.begin(), processesList.end(),
		  [&pid](const std::pair<HWND, DWORD>& element) {return element.second == pid; }
	  );

	  //Se trova elemento restituisce handle processo
	  if (it != processesList.end()) {
		  return (*it).first;
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


