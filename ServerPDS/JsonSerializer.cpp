#include <boost\property_tree\ptree.hpp>
#include <boost\property_tree\json_parser.hpp>
#include <tuple>
#include <list>
#include <sstream>
#include <iostream>
#include <string>
#include <Windows.h>
#include <mutex>
#include "IconExtractor.h"
#include "ProcessModel.h"
#include "JsonSerializer.h"


boost::property_tree::wptree JsonSerializer::processInfoToPtree(ProcessModel::processInfo pInfo)
{
	boost::property_tree::wptree pInfoNode;
	///0 PID; 1 Stato processo; 2 titlebar; 3 path; 4 icon information
	pInfoNode.put(L"pid", std::get<0>(pInfo));
	pInfoNode.put(L"state", std::get<1>(pInfo));
	pInfoNode.put(L"title", std::get<2>(pInfo));
	pInfoNode.put(L"path", std::get<3>(pInfo));
	//pInfoNode.put(L"icon", std::get<4>(pInfo));

	return pInfoNode;
}

//std::wstring JsonSerializer::serializeProcessesInfo(std::list<ProcessModel::processInfo> pList)
//{	
//	boost::property_tree::wptree processInfoNode; //nodo singolo processo
//	std::wstringstream tempStream;
//	
//	std::wstring serializedJsonArray = L"[";
//	
//
//
//	for (auto &processInfo : pList)
//	{
//		processInfoNode = processInfoToPtree(processInfo);
//		boost::property_tree::write_json(tempStream, processInfoNode);
//		//processesInfoNode.push_back(std::make_pair(L"", processInfoNode));
//		serializedJsonArray = serializedJsonArray + L"," + tempStream.str();
//	}
//	processesRootNode
//	
//	
//
//
//	return serializedJsonArray;
//}

std::wstring JsonSerializer::serializeProcessInfo(ProcessModel::processInfo pInfo)
{
	std::wstringstream serializedJson;
	boost::property_tree::wptree processInfoNode; //nodo singolo processo

	//Creo un vettore di 1 solo elemento
	processInfoNode = processInfoToPtree(pInfo);

	boost::property_tree::write_json(serializedJson, processInfoNode);
	
	return serializedJson.str();
}


