#include <mutex>
#include <Windows.h>
#include "Buffer.h"



bool Buffer::Push(HWND handle)
{
	std::lock_guard<std::mutex> lock(m);
	if (buffer == NULL) {
		buffer=handle;
		return true;
	}
	else {
		return false;
	}
}

HWND Buffer::Pop()
{
	std::lock_guard<std::mutex> lock(m);
	if (buffer != NULL) {
		return buffer;
	}
	else {
		return NULL;
	}
}


