#ifndef CONTROLLER_H
#define CONTROLLER_H


/**
  * class Controller
  * Controlla gli eventi del ProcessMonitor e WindowsPolling, chiama i metodi di
  * aggiornamento del modello e notifica i cambiamenti al client tramite la classe
  * socket
  */

class Controller
{
	
private:
	
	ProcessModel model;

	//Informazioni per avviare processo monitor 32bit
	LPCTSTR x86ProcessPath = L"ProcessMonitorX86.exe";
	STARTUPINFO x86StartupInfo;
	PROCESS_INFORMATION x86ProcessInformation;






	HANDLE hSlot;
	LPTSTR Slot;

	//Vector of the messages readed from the mailSlot
	std::queue<std::wstring> hookMessageQueue;
	
	//Hook object
	MyHook	myHookObj;
	

	//Network object
	Network netObj;
	
	
	/*Metodi per gestione mailslot di windows per comunicazione con dll*/
	bool MakeSlot(LPTSTR lpszSlotName);

	bool ReadSlot();

	//Extrapolate information abount event and handle from the mailslot
	EventInfo MessageToHandle_Event_Struct(std::wstring);

	//Take decision starting from the message event passed to the controller
	void ManageHookEvent(EventInfo);

	//Take decision starting from network messages
	void ManageNetworkEvent(EventInfo);


public:
	//System event
	HANDLE eventX64;
	HANDLE eventX86;
	HANDLE eventClientConNet;
	HANDLE eventRecNet;
	HANDLE terminateMonitorX86;	//Event used to terminate monitor of the x86 Processes
	
	Controller();
	~Controller();

  /**
   * Popola per la prima volta la struttura dati con le finestre aperte, invia i dati
   * al client e poi avvia i thread.
   * @return bool
   */
	bool Init();


	/**
	* Metodo che esegue lanciato durante l'esecuzione del programma.
	* Metodo di esecuzione di tutti i comandi e controlli
	*/
	void Run();

  /**
   * Metodo che consegna alla finestra che ha attualmente il focus una combinazione
   * di caratteri specificata dal client
   * @return bool
   * @param  KeyCombo Combinazione caratteri da consegnare alla finestra
   */
  bool SendKeyCombo(std::string);




};
#endif // !CONTROLLER_H


