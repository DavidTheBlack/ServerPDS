
#include <iostream>
#include <string>

#include <mutex>
#include <vector>
#include <Windows.h>
#include <list>
#include <algorithm>





struct processInfo
{
	HWND wHandle; //Window Handle 
	std::string windowTitle; //window title
	std::string processPath; //Process path
	bool isFocused; //True if the windows has the user focus
	std::string iconBytes; //Base64 codification of the icon image bytes
};


class SharedData {
private:
	//Struttura dati che contiene le informazioni dei processi sfruttiamo una lista
	std::list<processInfo> processList;
	//mutex per gestire l'accesso thread safe
	std::mutex mut;
public:
	//Restituisce l'attuale numero di processi memorizzati nella struttura dati
	int getNumberOfProcesses() {
		return processList.size();
	}



	//This function is used to return all the active process handle
	std::vector<HWND> getHandleArray() {
		/*
		0) Verificare che la lunghezza dell'array sia sufficiente a contenere tutti gli handle
		1) La funzione deve mette il lock sulla struttura dati.
		2) Visitare tutto il pTree , estrarre gli handle dalle strututre dati e copiarle nel vettore handleArray
		3) Rilasciare il lock sulla struttura dati
		4) Ritornare
		*/


		//Window handles
		std::vector<HWND> hVect;

		std::lock_guard<std::mutex> l(mut);
		for (std::list<processInfo>::iterator it = processList.begin(); it != processList.end(); ++it)
		{
			hVect.push_back(it->wHandle);
		}
		return hVect;
	}

	//This function is used to insert new process data into the process data structure
	//return true if the insert goes well 
	//return false if there is another process with the same hwnd in the data structure
	bool insertNewProcess(processInfo pi) {
		//1) lock della struttura dati
		//2) inserimento nuovo processo
		//3) verifica corretto inserimento
		std::lock_guard<std::mutex> l(mut);
		HWND temp;

		//Verifichiamo che il processo non sia già presente nella struttura dati
		auto it = std::find_if(processList.begin(), processList.end(), [&temp](processInfo const& obj) {return obj.wHandle == temp; });
		if (it == processList.end()) {
			//elemento non trovato iteratore "nullo"
			processList.push_back(pi);
			return true;
		}
		return false;
	}


};




int main()
{
	processInfo pi;
	SharedData sd;

	pi.wHandle = (HWND)1;
	pi.windowTitle = "TitoloFinestra1";
	pi.processPath = "c:\\processo1";
	pi.isFocused = false;
	pi.iconBytes = "ICONA1";
	
	
	///TODO valutare se creare una funzione per la struct che restituisce la versione string del pid
	/*Converting pid numeber to string in order to insert it as ptree node key*/
	//std::ostringstream convert;			// stream used for the conversion
	//convert << pi.wHandle;					// insert the textual representation of 'Number' in the characters in the stream

	if (sd.insertNewProcess(pi)) {
		std::cout << "Inserimento riuscito" << std::endl;
	}
	else {
		std::cout << "fottiti stronzo" << std::endl;
	}
	
	pi.wHandle = (HWND)2;
	pi.windowTitle = "TitoloFinestra2";
	pi.processPath = "c:\\processo2";
	pi.isFocused = false;
	pi.iconBytes = "ICONA2";

	if (sd.insertNewProcess(pi)) {
		std::cout << "Inserimento riuscito" << std::endl;
	}
	else {
		std::cout << "fottiti stronzo" << std::endl;
	}

	std::cout << "Dati inseriti: " << sd.getNumberOfProcesses() << std::endl;

	std::vector<HWND> vMerda = sd.getHandleArray();
	
	std::cout << "Primo elemento: " << vMerda[0] << " secondo elemento: " << vMerda[1] << std::endl;

	
	std::string stringa = std::to_string(1);
	




	/*
	for (boost::property_tree::ptree::iterator it = pTree.begin(); it != pTree.end(); it++)
	{
		processInfo pi;
		pi = pTree.get<processInfo>(it->first);
			//it->second.get<processInfo>;5
		std::cout << "Valore: " << pi.windowTitle << std::endl;
	}
	*/
	std::cin.get();
	
}

