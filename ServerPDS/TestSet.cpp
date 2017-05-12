#include "stdafx.h"
#include <Windows.h>
#include <iostream>
#include <string>
#include <set>
#include <iterator>

struct processInfo
{
	int pid; //Process ID 
	std::string windowTitle; //window title
	std::string processPath; //Process path
	bool isFocused; //True if the windows has the user focus
	std::string iconBytes; //Base64 codification of the icon image bytes
};
int _tmain(int argc, _TCHAR* argv[]) {
	processInfo pInfo,pInfo2;


	


	pInfo.iconBytes = "";
	pInfo.windowTitle = "Firefox";
	pInfo.isFocused = false;
	pInfo.processPath = "C:\\";
	pInfo.pid = 1;


	pInfo2.iconBytes = "";
	pInfo2.windowTitle = "Firefox";
	pInfo2.isFocused = false;
	pInfo2.processPath = "C:\\David";
	pInfo2.pid = 3;

	std::set<processInfo> mySet;
	std::pair<std::set<processInfo>::iterator, bool> ret;
	
	/*ret=*/mySet.insert(pInfo);
	if (ret.second ) {
		std::cout << "Primo Inserimento andato a buon fine" << std::endl;
	}

	//Rendo pInfo2 = pInfo1;
	pInfo2 = pInfo;
	ret = mySet.insert(pInfo2);
	if (ret.second) {
		std::cout << "Secondo Inserimento andato a buon fine" << std::endl;
	}
	else {
		std::cout << "Secondo Inserimento NON è andato a buon fine" << std::endl;
	}

	pInfo2.pid = 123;

	ret = mySet.insert(pInfo2);
	if (ret.second) {
		std::cout << "Terzo Inserimento andato a buon fine" << std::endl;
	}
	else {
		std::cout << "Terzo Inserimento NON è andato a buon fine" << std::endl;
	}

	std::cin.get();

}


