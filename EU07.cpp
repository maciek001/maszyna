/*
This Source Code Form is subject to the
terms of the Mozilla Public License, v.
2.0. If a copy of the MPL was not
distributed with this file, You can
obtain one at
http://mozilla.org/MPL/2.0/.
*/
/*
    MaSzyna EU07 locomotive simulator
    Copyright (C) 2001-2004  Marcin Wozniak, Maciej Czapkiewicz and others

*/
/*
Authors:
MarcinW, McZapkie, Shaxbee, ABu, nbmx, youBy, Ra, winger, mamut, Q424,
Stele, firleju, szociu, hunter, ZiomalCl, OLI_EU and others
*/

#include "stdafx.h"

#include "Globals.h"
#include "Logs.h"
#include "Console.h"
#include "PyInt.h"
#include "World.h"
#include "Mover.h"

#pragma comment (lib, "glu32.lib")
#pragma comment (lib, "dsound.lib")
#pragma comment (lib, "winmm.lib")
#pragma comment (lib, "setupapi.lib")
#pragma comment (lib, "dbghelp.lib")

TWorld World;

void window_resize_callback(GLFWwindow *window, int w, int h)
{
	Global::ScreenWidth = w;
	Global::ScreenHeight = h;
	glViewport(0, 0, w, h);
}

void cursor_pos_callback(GLFWwindow *window, double x, double y)
{
	World.OnMouseMove(x * 0.005, y * 0.01);
	glfwSetCursorPos(window, 0.0, 0.0);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	Global::shiftState = (mods & GLFW_MOD_SHIFT) ? true : false;
	Global::ctrlState = (mods & GLFW_MOD_CONTROL) ? true : false;

	if (action == GLFW_PRESS || action == GLFW_REPEAT)
	{
		World.OnKeyDown(key);

		switch (key)
		{
		case GLFW_KEY_PAUSE:
			if (Global::iPause & 1)
				Global::iPause &= ~1;
			else if (!(Global::iMultiplayer & 2) &&
				     (mods & GLFW_MOD_CONTROL))
				Global::iPause ^= 2;
			if (Global::iPause)
				Global::iTextMode = GLFW_KEY_F1;
			break;

		case GLFW_KEY_F7:
			Global::bWireFrame = !Global::bWireFrame;
			++Global::iReCompile;
			break;
		}
	}
	else if (action == GLFW_RELEASE)
	{
		World.OnKeyUp(key);
	}
}

void focus_callback(GLFWwindow *window, int focus)
{
	if (Global::bInactivePause) // jeśli ma być pauzowanie okna w tle
		if (focus)
			Global::iPause &= ~4; // odpauzowanie, gdy jest na pierwszym planie
		else
			Global::iPause |= 4; // włączenie pauzy, gdy nieaktywy
}

#ifdef _WINDOWS
LONG CALLBACK unhandled_handler(::EXCEPTION_POINTERS* e);
#endif

int main(int argc, char *argv[])
{
#ifdef _WINDOWS
	::SetUnhandledExceptionFilter(unhandled_handler);
#endif

	if (!glfwInit())
		return -1;

    DeleteFile("errors.txt");
    Global::LoadIniFile("eu07.ini");
    Global::InitKeys();

	// hunter-271211: ukrywanie konsoli
    if (Global::iWriteLogEnabled & 2)
    {
        AllocConsole();
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN);
    }

	for (int i = 1; i < argc; i++)
	{
		std::string token(argv[i]);

		if (token == "-modifytga")
			Global::iModifyTGA = -1;
		else if (token == "-e3d")
		{
			if (Global::iConvertModels > 0)
				Global::iConvertModels = -Global::iConvertModels;
			else
				Global::iConvertModels = -7; // z optymalizacją, bananami i prawidłowym Opacity
		}
		else if (i + 1 < argc && token == "-s")
			Global::SceneryFile = std::string(argv[++i]);
		else if (i + 1 < argc && token == "-v")
		{
			std::string v(argv[++i]);
			std::transform(v.begin(), v.end(), v.begin(), ::tolower);
			Global::asHumanCtrlVehicle = v;
		}
		else
		{
			std::cout << "usage: " << std::string(argv[0]) << " [-s sceneryfilepath] "
				      << "[-v vehiclename] [-modifytga] [-e3d]" << std::endl;
			return -1;
		}
	}

	// match requested video mode to current to allow for
	// fullwindow creation when resolution is the same
	GLFWmonitor *monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode *vmode = glfwGetVideoMode(monitor);

	glfwWindowHint(GLFW_RED_BITS, vmode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, vmode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, vmode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, vmode->refreshRate);

	glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_FALSE);
	glfwWindowHint(GLFW_SAMPLES, Global::iMultisampling);

	GLFWwindow *window =
		glfwCreateWindow(Global::iWindowWidth, Global::iWindowHeight,
		"EU07++NG", Global::bFullScreen ? monitor : nullptr, nullptr);

	if (!window)
	{
		std::cout << "failed to create window" << std::endl;
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); //vsync
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); //capture cursor
	glfwSetCursorPos(window, 0.0, 0.0);
	glfwSetFramebufferSizeCallback(window, window_resize_callback);
	glfwSetCursorPosCallback(window, cursor_pos_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetWindowFocusCallback(window, focus_callback);
	{
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		window_resize_callback(window, width, height);
	}

	if (glewInit() != GLEW_OK)
	{
		std::cout << "failed to init GLEW" << std::endl;
		return -1;
	}

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	Global::pWorld = &World; // Ra: wskaźnik potrzebny do usuwania pojazdów
	if (!World.Init(window))
	{
		std::cout << "failed to init TWorld" << std::endl;
		return -1;
	}

    Console *pConsole = new Console(); // Ra: nie wiem, czy ma to sens, ale jakoś zainicjowac trzeba

    if (!joyGetNumDevs())
        WriteLog("No joystick");
    if (Global::iModifyTGA < 0)
    { // tylko modyfikacja TGA, bez uruchamiania symulacji
        Global::iMaxTextureSize = 64; //żeby nie zamulać pamięci
        World.ModifyTGA(); // rekurencyjne przeglądanie katalogów
    }
    else
    {
        if (Global::iConvertModels < 0)
        {
            Global::iConvertModels = -Global::iConvertModels;
            World.CreateE3D("models\\"); // rekurencyjne przeglądanie katalogów
            World.CreateE3D("dynamic\\", true);
        } // po zrobieniu E3D odpalamy normalnie scenerię, by ją zobaczyć

        Console::On(); // włączenie konsoli
        while (!glfwWindowShouldClose(window) && World.Update())
        {
			glfwSwapBuffers(window);
			glfwPollEvents();
        }
        Console::Off(); // wyłączenie konsoli (komunikacji zwrotnej)
    }

    delete pConsole;
	TPythonInterpreter::killInstance();

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
