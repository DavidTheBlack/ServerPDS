#include <tuple>
#include <list>
#include <sstream>
#include <iostream>
#include <string>
#include <Windows.h>
#include <mutex>
#include "IconExtractor.h"
#include "ProcessModel.h"
#include "json\json.h"
#include "JsonSerializer.h"


Json::Value JsonSerializer::processInfoToJNode(ProcessModel::processInfo pInfo)
{

	Json::Value jNode;
	///0 PID; 1 Stato processo; 2 titlebar; 3 path; 4 icon information
	
	char* tmpBuf = NULL; 
	int len;
	


	jNode["pid"] = std::to_string(std::get<0>(pInfo));
	jNode["state"] = std::to_string(std::get<1>(pInfo));

	//Convert the wstring in string to save in the jnode structure	
	
	//Read the length of the wstring 	
	len = WideCharToMultiByte(CP_UTF8, 0, std::get<2>(pInfo).c_str(), -1, NULL, 0, 0, 0);
	tmpBuf = new char[len + 1];
	
	
	
	/*Appunto :: JsonCpp quando assegna un puntatore ad un Jnode dereferenzia in automatico e preleva il contenuto dell'area di memoria
	fino al terminatore di stringa
	Se invece assegnamo una variabile normale memorizza il right value normalmente.
	*/
	WideCharToMultiByte(CP_UTF8, 0, std::get<2>(pInfo).c_str(), -1, tmpBuf, len, 0, 0);	
	jNode["title"] = tmpBuf; 
	delete[] tmpBuf;
	tmpBuf = NULL;


	len = WideCharToMultiByte(CP_UTF8, 0, std::get<3>(pInfo).c_str(), -1, NULL, 0, 0, 0);
	tmpBuf = new char[len + 1];
	WideCharToMultiByte(CP_UTF8, 0, std::get<3>(pInfo).c_str(), -1, tmpBuf, len, 0, 0);
	jNode["path"] = tmpBuf;
	delete[] tmpBuf;
	tmpBuf = NULL;

	//jNode["icon"] = std::get<4>(pInfo);
	
	return jNode;
}

std::wstring JsonSerializer::serializeProcessesInfo(std::list<ProcessModel::processInfo> plist)
{	
	std::string serializedJson;
	
	Json::StyledWriter styledWriter;
	Json::Value jNode;
	Json::Value jArray;

	//Creo array di tutti i processi
	for (auto &processinfo : plist)
	{
		jNode=processInfoToJNode(processinfo);		
		jArray.append(jNode);
	}
	
	serializedJson = styledWriter.write(jArray);
	std::wstring returnStr(serializedJson.begin(), serializedJson.end());
	
	return returnStr;
}

std::wstring JsonSerializer::serializeProcessInfo(ProcessModel::processInfo pInfo)
{
	std::string serializedJson;	
	Json::StyledWriter styledWriter;

	//Creo un vettore di 1 solo elemento
	Json::Value jNode = processInfoToJNode(pInfo);
	Json::Value jArray;
	
	//Create array of a single element
	jArray.append(jNode);
	

	serializedJson = styledWriter.write(jArray);
	
	std::wstring returnStr(serializedJson.begin(),serializedJson.end());
	
	return returnStr;
}


