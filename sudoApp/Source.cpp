#include "header/SudoApp.h"
#include <tchar.h>
#include <filesystem>
#include <shlobj.h>

int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nCmdShow)
{
	unsigned winWidth = 1920;
	unsigned winHeight = 1080;
	try
	{
		SudoApp appInstance(winWidth, winHeight);
		return appInstance.Go();
	}
	catch (const SudoException& e)
	{
		MessageBox(nullptr, e.what(), e.GetType(), MB_OK | MB_ICONEXCLAMATION);
	}
	catch (const std::exception& e)
	{
		MessageBox(nullptr, e.what(), "Standard Exception", MB_OK | MB_ICONEXCLAMATION);
	}
	catch (...)
	{
		MessageBox(nullptr, "No details available", "Unknown Exception", MB_OK | MB_ICONEXCLAMATION);
	}
	return -1;
}