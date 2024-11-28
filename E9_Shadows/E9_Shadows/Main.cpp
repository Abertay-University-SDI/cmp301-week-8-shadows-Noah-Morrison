// Main.cpp
#include "System.h"
#include "App1.h"
#include "WheatApp.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	App1* app = new App1();
	WheatApp* wheatApp = new WheatApp();
	System* system;

	// Create the system object.
	// Lab Application
	//system = new System(app, 1200, 675, true, false);
	// Wheat Instance Position Calculator
	system = new System(wheatApp, 1200, 675, true, false);

	// Initialize and run the system object.
	system->run();

	// Shutdown and release the system object.
	delete system;
	system = 0;

	return 0;
}