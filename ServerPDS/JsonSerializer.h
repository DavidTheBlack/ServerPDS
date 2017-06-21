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

	//Extract single process information into ptree
	boost::property_tree::wptree processInfoToPtree(ProcessModel::processInfo);

public:
	
	//Serialize all processes information
	std::wstring serializeProcessesInfo(std::list<ProcessModel::processInfo>);
	//Serialize single process information
	std::wstring serializeProcessInfo(ProcessModel::processInfo);

};

