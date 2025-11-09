#include "macros.h"
#include "windows.h"
#include "iostream"

int main()
{
	system("title MacrosX");

	std::cout << "Macros X\nX = 0\nY = 3\nEnable = 0\n\nKeys\n\nToggleKey:INSERT\nButtonUp = Y + 1\nButtonDown = Y - 1 \nButtonRight = X + 1\nButtonleft = X - 1" << std::endl;

	while (true) 
	{
		if (GetAsyncKeyState(VK_UP) & 1)
		{
			movey++;
			std::cout << "Y:" << movey << std::endl;
		}
		if (GetAsyncKeyState(VK_DOWN) & 1)
		{
			movey--;
			std::cout << "Y:" << movey << std::endl;
		}
		if (GetAsyncKeyState(VK_RIGHT) & 1)
		{
			movex++;
			std::cout << "X:" << movex << std::endl;
		}
		if (GetAsyncKeyState(VK_LEFT) & 1)
		{
			movex--;
			std::cout << "X:" << movex << std::endl;
		}
		if (GetAsyncKeyState(VK_INSERT) & 1)
		{
			enable = !enable;
			std::cout << "Enable:" << enable << std::endl;
		}

		g_Macros.MoveMouse();
	}
	return 0;
}