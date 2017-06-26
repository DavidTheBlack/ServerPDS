#ifndef JSONSERIALIZER_H
#define JSONSERIALIZER_H

class JsonSerializer
{
private:	

	//Struttura dati per gestire le informazioni dei processi tupla c++
	//0 PID; 1 Stato processo; 2 titlebar; 3 path; 4 icon information
	
	//Extract single process information into JsonNode
	Json::Value processInfoToJNode(ProcessModel::processInfo);


public:
	//Serialize all processes information
	std::wstring serializeProcessesInfo(std::list<ProcessModel::processInfo>);
	//Serialize single process information
	std::wstring serializeProcessInfo(ProcessModel::processInfo);

};





#endif // !JSONSERIALIZER_H



