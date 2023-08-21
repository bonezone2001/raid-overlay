#include <iostream>

#include <windows.h>
#include <io.h>
#include <fcntl.h>

#include "brain_rot.h"
#include "core/settings.h"
#include "utils/utils.h"
#include "utils/auto_update.h"
#include "log_parser/file_hook.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

    ALLOC_DEBUG_CONSOLE();

    if (Green::Utils::CheckForUpdates())
    {
        if (Green::Utils::UpdateInPlace())
        {
            MessageBox(NULL, L"Updated successfully, press OK to restart.", L"BrainRot", MB_OK);
            Green::Utils::Restart();
        }
    }

    Green::Settings::getInstance().load();
    auto& brainRot = Green::BrainRot::getInstance();
    brainRot.init();
    brainRot.start();

    return 0;
}