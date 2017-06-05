#pragma once
class WindowsEnum
{
public:
	WindowsEnum& enum_windows();
	std::list<HWND>& getData();

private:
	std::list<HWND> localProcessList;

	BOOL IsAltTabWindow(HWND hwnd);	

	BOOL wndProc(HWND hWnd);

	static BOOL CALLBACK enumWindowsProc(__in  HWND hWnd, __in  LPARAM lParam);
};

