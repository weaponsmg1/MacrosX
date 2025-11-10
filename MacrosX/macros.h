#pragma once

class Macros
{
public:
	void MoveMouse();
	void BunnyHop();
	void AutoPistol();
};

extern bool enable;
extern bool bhop;
extern bool autopistol;
extern int movex;
extern int movey;

extern Macros g_Macros;
