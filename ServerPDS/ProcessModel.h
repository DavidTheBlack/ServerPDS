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
	std::list<std::pair<HWND,DWORD>> processesList;
	//mutex per gestire l'accesso thread safe
	std::mutex mut;
	//Handler del processo che detiene il focus
	HWND focusProcess;

	//Icon extractor Object
	CIconExtractor iconExtrObj;


public:	

	/* Struttura dati per gestire le informazioni dei processi, tupla c++ */
	typedef std::tuple<DWORD, int, std::wstring ,std::wstring, std::string> processInfo;
	//0 PID; 1 Stato processo; 2 titlebar; 3 path; 4 icon information



	/**
	* Set the value of FocusedProcessPid
	* Pid del processo che attualmente ha il focus
	* @param new_var the new value of FocusedProcessPid
	*/
	bool setFocusedProcess(HWND process);

	/**
	* Get the value of FocusedProcessPid
	* Pid del processo che attualmente ha il focus
	* @return the value of FocusedProcessPid
	*/
	HWND getFocusedProcess();

	/*
	*Restituisce l'attuale numero di processi memorizzati nella struttura dati
	*/
	size_t getNumberOfProcesses();

	/**
	* Restituisce la struttura dati condivisa processList
	* @return std::list<std::pair<HWND,DWORD>>
	*/
	std::list<std::pair<HWND, DWORD>> getProcessesList();

	/**
	* Aggiunta safe di un nuovo processo
	* @return bool
	* @param  processInfo
	*/
	bool addProcess(HWND processHwnd);

	/**Restituisce il HWND del processo noto il pid*/
	HWND pidToHwnd(DWORD pid);

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

	/**
	* Retrieve 1 process information starting from HWND
	* @return processInfo tupel structure
	* @param HWND of the process
	*/
	processInfo getProcessInfo(HWND);	

	/**
	* retrieve all processes iformation 
	* @return processInfo tupel structure
	* @param HWND of the process
	*/
	std::list<processInfo> getProcessesInfo();
};

#endif // !PROCESSMODEL_H