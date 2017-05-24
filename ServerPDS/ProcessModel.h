#ifndef PROCESSMODEL_H
#define PROCESSMODEL_H


/**
  * class ProcessModel
  * Contiene al suo interno lo stato corrente del server, mantiene le informazioni
  * sui processi attivi. Fornisce la versione json della struttura dati
  */

class ProcessModel
{

private:
	//Struttura dati che contiene le informazioni dei processi sfruttiamo una lista
	std::list<HWND> processesList;
	//mutex per gestire l'accesso thread safe
	std::mutex mut;
	//Handler del processo che detiene il focus
	HWND focusProcess;



public:



	
	/**
	* Set the value of FocusedProcessPid
	* Pid del processo che attualmente ha il focus
	* @param new_var the new value of FocusedProcessPid
	*/
	void setFocusedProcess(HWND process);

	/**
	* Get the value of FocusedProcessPid
	* Pid del processo che attualmente ha il focus
	* @return the value of FocusedProcessPid
	*/
	HWND getFocusedProcess();

	/*
	*Restituisce l'attuale numero di processi memorizzati nella struttura dati
	*/
	int getNumberOfProcesses();

	/**
	* Restituisce la struttura dati condivisa processList
	* @return std::vector<HWND>
	*/
	std::list<HWND> getProcessesList();

	/**
	* Aggiunta safe di un nuovo processo
	* @return bool
	* @param  processInfo
	*/
	bool addProcess(HWND processHwnd);

	/**
	* Aggiunta safe di una lista di processi metodo richiamato alla prima esecuzione di windowsEnum
	* @return bool
	* @param  list
	*/
	bool setProcessesList(std::list<HWND> list);

	/**
	* Rimozione safe di un processo dalla struttura dati
	* @return bool
	* @param  processHwnd
	*/
	bool removeProcess(HWND processHwnd);

};

#endif // !PROCESSMODEL_H