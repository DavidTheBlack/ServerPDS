#pragma once

/**
  * class Controller
  * Controlla gli eventi del ProcessMonitor e WindowsPolling, chiama i metodi di
  * aggiornamento del modello e notifica i cambiamenti al client tramite la classe
  * socket
  */

class Controller
{
public: 
	//Evento di creazione nuovo processo
	HANDLE creation ;
	//Evento di distruzione processo
	HANDLE destruction;
	//Evento di focus (processo portato in primo piano)
	HANDLE activation ;
	//Evento di ricezione messaggio da rete
	HANDLE network ;
	//Array eventi da passare a WaitForMultipleObject
	HANDLE eventArray[4];


//METHODS
public:


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


