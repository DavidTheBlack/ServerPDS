#pragma once


class JsonSerializer
{
private:
	boost::property_tree::ptree root;				//boost ptree root
	std::ostringstream buf;		//Output stream of the boost library
	std::string jsonStr;		//json string

	//Struttura dati per gestire le informazioni dei processi
	//tupla c++
	
	//0 PID; 1 Stato processo; 2 titlebar; 3 path; 4 icon information

	//Serialize a process information
	boost::property_tree::wptree serializeProcessInfo(ProcessModel::processInfo);

public:
	
	//Serialize all processes information
	std::wstring serializeProcessesInfo(std::list<ProcessModel::processInfo>);

};

