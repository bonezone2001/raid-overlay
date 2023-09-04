#pragma once
// ReSharper disable CppInconsistentNaming

#include <string>
#include "nlohmann/json.hpp"
#include "../utils/utils.h"

#define APP_SETTINGS_FILE_NAME "settings.json"
#define PLAYER_NAME_MIN_LENGTH 3
#define PLAYER_NAME_MAX_LENGTH 12

namespace nlohmann {
    template <>
    struct adl_serializer<std::wstring> {
        static void to_json(json& j, const std::wstring& str) {
            j = Green::Utils::ToUtf8(str);
        }

        static void from_json(const json& j, std::wstring& str) {
            str = Green::Utils::FromUtf8(j.get<std::string>());
        }
    };
}

namespace Green
{

class Settings
{
public:
    std::wstring player_name;
    std::wstring log_file_name;
    std::wstring wow_log_folder_override;
    std::vector<std::wstring> echo_player_order;
	bool raid_lead_mode = false;
    bool sound_alerts = true;

private:
    bool createIfMissing(const std::string& path) const;
    bool loadSettings(const std::string& path);

public:
    Settings() = default;

    // Get the singleton instance
    static Settings& getInstance()
    {
        static Settings instance;
        return instance;
    }

    void load();
    void setInstance(const Settings& settings);
    void save() const;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Settings, player_name, wow_log_folder_override, log_file_name, raid_lead_mode, echo_player_order, sound_alerts);
};

}
