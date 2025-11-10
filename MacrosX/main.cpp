#include "macros.h"
#include "windows.h"
#include "iostream"

int main()
{
	system("title MacrosX");

	std::cout << "Macros X\nX = 0\nY = 3\nEnable = 0\nAutoPistol = 0\nBunnyHop = 0\n\nKeys\n\nToggleKey:INSERT\nAutoPistol:DELETE\nBunnyHop:HOME\nButtonUp = Y + 1\nButtonDown = Y - 1 \nButtonRight = X + 1\nButtonleft = X - 1\nHold Mouse 4 to use AutoPistol\n" << std::endl;

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
		if (GetAsyncKeyState(VK_HOME) & 1)
		{
			bhop = !bhop;
			std::cout << "Bhop:" << bhop << std::endl;
		}
		if (GetAsyncKeyState(VK_DELETE) & 1)
		{
			autopistol = !autopistol;
			std::cout << "AutoPistol:" << autopistol << std::endl;
		}

		g_Macros.MoveMouse();
		g_Macros.BunnyHop();
		g_Macros.AutoPistol();
	}
	return 0;
}
