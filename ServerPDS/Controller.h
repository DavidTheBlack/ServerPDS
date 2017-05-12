#pragma once
class Controller
{
public:
	ProcessModel model;
	CloseObserver closeOb;

public:
	Controller();
	~Controller();

	void processAdded(HANDLE);
	void processRemoved(HANDLE);
};

