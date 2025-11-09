#include "macros.h"
#include <windows.h>

bool enable = false;
int movex = 0;
int movey = 3;

Macros g_Macros;

void Macros::MoveMouse()
{
	if (enable && (GetAsyncKeyState(VK_LBUTTON) & 0x8000))
	{
		POINT p;
		GetCursorPos(&p);
		mouse_event(MOUSEEVENTF_MOVE, movex, movey, 0, 0);
		Sleep(10);
	}
}

