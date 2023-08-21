#pragma once

// Auto updater for my project, using a private update server.
// This is a very simple implementation.

namespace Green::Utils
{

void DeleteOldFiles();
std::string GetCurrentVersion();
bool CheckForUpdates();
bool UpdateInPlace();

}
