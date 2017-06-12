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

	HANDLE hSlot;
	LPTSTR Slot = TEXT("\\\\.\\mailslot\\ms1");

	//Vector of the messages readed from the mailSlot
	std::queue<std::wstring> hookMessageQueue;

	//Hook object
	MyHook myHookObj;

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
   * Thread dormiente che viene svegliato dal thread socket per ricevere comandi dal
   * client
   */
  void RiceviComandiDaRete();


  /**
   * Metodo che consegna alla finestra che ha attualmente il focus una combinazione
   * di caratteri specificata dal client
   * @return bool
   * @param  KeyCombo Combinazione caratteri da consegnare alla finestra
   */
  bool SendKeyCombo(std::string);


  /**
   * Questo metodo analizza il messaggio ricevuto dal client e ne elabora la logica
   * @return bool
   * @param  receivedMessage Messaggio ricevuto dal client
   */
  bool CheckMessageReceived(std::string);


  /**
   * Lancia thread di MyHook ed aspetta che questo produca un dato, quando avviane
   * una variazione, aggiorna la base dati e segnala la modifica alla classe di rete
   * che provvederà ad aggiornare il client
   */
  void MonitorProcesses();





};
#endif // !CONTROLLER_H


