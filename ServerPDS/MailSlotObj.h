#ifndef MAILSLOTOBJ_H
#define MAILSLOTOBJ_H

class MailSlotObj
{
private:
	//MailSlot used to retrieve information about icon for 32bit process
	HANDLE hSlot;
	LPTSTR SlotName;
	/*Metodi per gestione mailslot di windows per comunicazione con dll*/
	bool MakeSlot(LPTSTR lpszSlotName);
	//True if the object can read from a mail slot false if can send to a mail slot
	bool canReceive;
	std::wstring message;



public:
	/*Input:  mail slot name AND Type-> true to receive and false to send to a mailslot*/
	MailSlotObj(LPTSTR mailSlotName, bool Type);

	~MailSlotObj();

	bool ReadSlot();
	bool WriteSlot(LPCTSTR lpszMessage);
	std::string getMessage();
};

#endif 