#include "macros.h"
#include <windows.h>

bool enable = false;
bool bhop = false;
bool autopistol = false;
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

void Macros::BunnyHop()
{
	if (bhop && GetAsyncKeyState(VK_SPACE) & 0x8000)
	{
		keybd_event(VK_SPACE, 0, 0, 0);            
		keybd_event(VK_SPACE, 0, KEYEVENTF_KEYUP, 0); 
		Sleep(20);
	}
}

void Macros::AutoPistol()
{
	if (autopistol && GetAsyncKeyState(VK_XBUTTON1) & 0x8000)
	{
		mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
		mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		Sleep(10);
	}
}
