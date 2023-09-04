#include <iostream>

#include <windows.h>
#include <fcntl.h>

#include "brain_rot.h"
#include "core/settings.h"
#include "utils/utils.h"
#include "utils/auto_update.h"
#include "log_parser/file_hook.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

    ALLOC_DEBUG_CONSOLE();

#ifndef _DEBUG
    if (Green::Utils::CheckForUpdates())
    {
        if (Green::Utils::UpdateInPlace())
        {
            MessageBox(NULL, L"Updated successfully, press OK to restart.", L"BrainRot", MB_OK);
            Green::Utils::Restart();
        }
    }
#endif

    // Settings loading and sanity checking
	try
    {
	    Green::Settings::getInstance().load();
    }
    catch (...)
    {
		MessageBox(NULL, L"Failed to load settings, if app has updated, please recreate settings.json", L"BrainRot", MB_OK);
        return 1;
	}

    auto& brainRot = Green::BrainRot::getInstance();
    brainRot.init();
    brainRot.start();

    return 0;
}