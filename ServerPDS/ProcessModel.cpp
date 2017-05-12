#include "ProcessModel.h"
#include <Windows.h>
#include <mutex>
#include <list>
#include <algorithm>
#include <string>
#include <vector>



  /**
  * Set the value of FocusedProcessPid
  * Pid del processo che attualmente ha il focus
  * @param new_var the new value of FocusedProcessPid
  */
  void ProcessModel::setFocusedProcess(HWND process) {
	  focusProcess = process;
  }

  /**
  * Get the value of FocusedProcessPid
  * Pid del processo che attualmente ha il focus
  * @return the value of FocusedProcessPid
  */
  HWND ProcessModel::getFocusedProcess() {
	  return focusProcess;
  }

  /*
  *Restituisce l'attuale numero di processi memorizzati nella struttura dati
  */
  int ProcessModel::getNumberOfProcesses() {
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
	  //Verifichiamo che il processo non sia gi� presente nella struttura dati
	  it = std::find(processesList.begin(), processesList.end(), processHwnd);
	  if (it == processesList.end()) {
		  //elemento non trovato iteratore "nullo"
		  processesList.push_back(processHwnd);
		  return true;
	  }
	  return false;
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
		  //elemento trovato, � possibile rimuoverlo dalla lista processi
		  processesList.erase(it);
		  return true;
	  }
	  return false;
  }

