#pragma once
class WindowsEnum
{
public:

	struct ProcessInfo {
		DWORD pId;
		std::wstring processPath;
		std::wstring caption;
	} ;

	WindowsEnum& enum_windows();


	std::list<ProcessInfo>& getData();

private:
	std::list<ProcessInfo> localProcessList;

	BOOL IsAltTabWindow(HWND hwnd);

	ProcessInfo getProcessInfo(HWND hwnd);

	BOOL wndProc(HWND hWnd);

	static BOOL CALLBACK enumWindowsProc(__in  HWND hWnd, __in  LPARAM lParam);
};

