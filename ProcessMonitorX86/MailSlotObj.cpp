#include "stdafx.h"
#include <sstream>
#include <string>
#include <Windows.h>
#include "MailSlotObj.h"






MailSlotObj::MailSlotObj(LPTSTR mailSlotName, bool Type)
{
	SlotName = mailSlotName;
	canReceive = Type;
	//Se posso ricevere creo il mail slot 
	if (canReceive) {
		MakeSlot(SlotName);
	}
	else { 
		//altrimenti 
		//mi limito a salvare le informazioni per inviare i dati ad un mailslot
		hSlot = CreateFile(SlotName, GENERIC_WRITE,
			FILE_SHARE_READ,
			(LPSECURITY_ATTRIBUTES)NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
			(HANDLE)NULL);
	}

	


}

MailSlotObj::~MailSlotObj()
{
	CloseHandle(hSlot);
}

bool MailSlotObj::MakeSlot(LPTSTR lpszSlotName)
{
	hSlot = CreateMailslot(lpszSlotName,
		0,                             // no maximum message size 
		MAILSLOT_WAIT_FOREVER,         // no time-out for operations 
		(LPSECURITY_ATTRIBUTES)NULL); // default security

	if (hSlot == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	return true;
}

bool MailSlotObj::ReadSlot()
{

	if (canReceive) {
		DWORD cbMessage, cMessage, cbRead;
		BOOL fResult;
		LPTSTR lpszBuffer;
		DWORD cAllMessages;

		cbMessage = cMessage = cbRead = 0;


		fResult = GetMailslotInfo(hSlot, // mailslot handle 
			(LPDWORD)NULL,               // no maximum message size 
			&cbMessage,                  // size of next message 
			&cMessage,                   // number of messages 
			(LPDWORD)NULL);              // no read time-out 

		if (!fResult)
		{
			//@TODO scrivere gli errori in un file di log
			/*printf("GetMailslotInfo failed with %d.\n", GetLastError());*/

			return false;
		}


		//Se il mailslot non ha messaggi in coda esco dal metodo e ritorno true
		if (cbMessage == MAILSLOT_NO_MESSAGE)
		{
			return true;
		}

		cAllMessages = cMessage;

		while (cMessage != 0)  // retrieve all messages
		{

			// Allocate memory for the message. 
			lpszBuffer = (LPTSTR)GlobalAlloc(GPTR, cbMessage);
			if (NULL == lpszBuffer)
				return FALSE;

			lpszBuffer[0] = '\0';

			fResult = ReadFile(hSlot,
				lpszBuffer,
				cbMessage,
				&cbRead,
				NULL);

			if (!fResult)
			{
				//@TODO SCRIVERE IL MESSAGGIO IN UN FILE DI LOG
				//printf("ReadFile failed with %d.\n", GetLastError());
				GlobalFree((HGLOBAL)lpszBuffer);
				return FALSE;
			}


			// Save the message into the message variable. 
			std::wstring mex(lpszBuffer);
			message = mex;

			GlobalFree((HGLOBAL)lpszBuffer);


			//Check if there are other messages in the mailSlot
			fResult = GetMailslotInfo(hSlot,	// mailslot handle 
				(LPDWORD)NULL,					// no maximum message size 
				&cbMessage,						// size of next message 
				&cMessage,						// number of messages 
				(LPDWORD)NULL);					// no read time-out 

			if (!fResult)
			{
				//@TODO scrivere il messaggio di errore in un file di log
				//printf("GetMailslotInfo failed (%d)\n", GetLastError()); 
				return false;
			}
		}
		return true;

	}else{
		return false;
	}

}

//Method used to send message via messageslot
bool MailSlotObj::WriteSlot(LPCTSTR lpszMessage)
{
	DWORD cbWritten;
	BOOL fResult = WriteFile(hSlot, lpszMessage,
		(DWORD)(lstrlen(lpszMessage) + 1) * sizeof(TCHAR),
		&cbWritten, (LPOVERLAPPED)NULL);
	if (!fResult)
	{
		return false;
	}
	return true;
}

//Return the message contained in the mail slot
std::string MailSlotObj::getMessage()
{	
	
	//Convert the wstring in string to save in the jnode structure	
	//Read the length of the wstring 	
	int len = WideCharToMultiByte(CP_UTF8, 0, message.c_str(), -1, NULL, 0, 0, 0);
	char* tmpBuf = new char[len + 1];
	
	WideCharToMultiByte(CP_UTF8, 0, message.c_str(), -1, tmpBuf, len, 0, 0);
	std::string retStr(tmpBuf);
	delete[] tmpBuf;
	tmpBuf = NULL;

	return retStr;
}


std::wstring MailSlotObj::getMessageW() {
	return message;
}
