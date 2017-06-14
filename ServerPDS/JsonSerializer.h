#pragma once

namespace pt = boost::property_tree;
class JsonSerializer
{
private:
	pt::ptree root;				//boost ptree root
	std::ostringstream buf;		//Output stream of the boost library
	std::string jsonStr;		// json string
	char* data;					//Data to send over tcp

	//Struttura dati per gestire le informazioni dei processi
	//tupla c++
	typedef std::tuple<DWORD, int, std::string, std::string> processInfo; //PID,ProcessState,ProcessPath,IconInfo

	

public:


};

