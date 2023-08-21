#include "log_parser/log_parser.h"
#include "modules/echo.h"
#include "core/settings.h"
#include "brain_rot.h"

namespace Green {

BrainRot::BrainRot() :
	running(false)
{
	// Find wow window
	wowWindow = FindWindowA(NULL, "World of Warcraft");
	if (wowWindow == NULL || !IsWindow(wowWindow))
	{
		MessageBox(NULL, L"World of Warcraft window not found!", L"Error", MB_ICONERROR);
		exit(1);
	}

	// Get the log folder location
	logFilePath = Utils::getLogFileLocation(wowWindow);
	WDEBUG_PRINTLN("Log file: " << logFilePath);
}

void BrainRot::init()
{
	logParser = std::make_shared<LogParser>(logFilePath);
	curFight = std::make_shared<EchoFight>(wowWindow);
}

void BrainRot::start()
{
	running = true;
	curFight->start();
}

void BrainRot::stop()
{
	running = false;
}

}