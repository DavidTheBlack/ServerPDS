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

std::wstring JsonSerializer::serializeProcessesInfo(std::list<ProcessModel::processInfo> pList)
{	
	std::wstringstream serializedJson;
	
	boost::property_tree::wptree processInfoNode; //nodo singolo processo
	boost::property_tree::wptree processesInfoNode; //nodi più processi
	boost::property_tree::wptree pListInfoRoot;

	for (auto &processInfo : pList)
	{
		processInfoNode = processInfoToPtree(processInfo);
		processesInfoNode.push_back(std::make_pair(L"", processInfoNode));
	}
	pListInfoRoot.add_child(L"Processes", processesInfoNode);	
	
	boost::property_tree::write_json(serializedJson, pListInfoRoot);

	

	return serializedJson.str();
}

std::wstring JsonSerializer::serializeProcessInfo(ProcessModel::processInfo pInfo)
{
	std::wstringstream serializedJson;
	std::wstring outString;
	boost::property_tree::wptree processInfoNode; //nodo singolo processo

	processInfoNode = processInfoToPtree(pInfo);

	boost::property_tree::write_json(serializedJson, processInfoNode);	

	serializedJson >> outString;
	
	return outString;
}


