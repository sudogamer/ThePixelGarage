#pragma once
#include "factoryUtils/Window.h"
#include "factoryUtils/SudoTimer.h"

class SudoApp
{
public:
	SudoApp();
	// master frame / message loop
	int Go();
private:
	void DoFrame();
private:
	Window wnd;
	SudoTimer timer;
};