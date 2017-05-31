#include "stdafx.h"
#include <mutex>
#include <condition_variable>
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
	  std::list<HWND>::iterator it;
	  it = std::find(processesList.begin(), processesList.end(), processHwnd);
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
  int ProcessModel::getNumberOfProcesses() {
	  std::lock_guard<std::mutex> l(mut);
	  return processesList.size();
  }

  /**
   * Restituisce la struttura dati condivisa processList
   * @return std::vector<HWND>
   */
  std::list<HWND> ProcessModel::getProcessesList()
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
	  std::list<HWND>::iterator it;
	  //Verifichiamo che il processo non sia già presente nella struttura dati
	  it = std::find(processesList.begin(), processesList.end(), processHwnd);
	  if (it == processesList.end()) {
		  //elemento non trovato iteratore "nullo"
		  processesList.push_back(processHwnd);
		  return true;
	  }
	  return false;
  }

  bool ProcessModel::setProcessesList(std::list<HWND> list) {
	  if (processesList.empty()) {
		  std::lock_guard<std::mutex> l(mut);
		  processesList = list;
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
	  std::list<HWND>::iterator it;
	  it = std::find(processesList.begin(), processesList.end(), processHwnd);
	  if (it != processesList.end()) {
		  //elemento trovato, è possibile rimuoverlo dalla lista processi
		  processesList.erase(it);
		  return true;
	  }
	  return false;
  }


