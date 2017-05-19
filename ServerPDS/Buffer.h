#pragma once
class Buffer
{
private:
	HWND buffer;
	std::mutex m;
public:
	
	bool Push(HWND handle);
	HWND Pop();

};


