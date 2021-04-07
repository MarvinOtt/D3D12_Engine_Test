#pragma once
#include <Windows.h>

class Window
{
public:
	HWND hwnd;
	HINSTANCE hInstance;
	bool IsCreated;

private:
	HRESULT hr;

public:
	Window(HINSTANCE hInstance);
	bool Create(bool fullscreen, LPCWSTR name, int sizex, int sizey, int offsetx = 0, int offsety = 0);
	bool Show(int state);
};

