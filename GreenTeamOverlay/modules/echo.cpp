#include "echo.h"

#include "../brain_rot.h"
#include "../imgui/imgui.h"
#include "../core/settings.h"
#include "../log_parser/log_parser.h"

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

#define WINDOW_FLAGS (ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize)

namespace Green {

EchoFight::EchoFight(const HWND wowWindow) :
	BaseFight("Echo Overlay", wowWindow),
	activeDestruction(false),
	destructionIndex(0)
{
	const auto& settings = Settings::getInstance();
	playerOrder = settings.echo_player_order;
	isRaidLeader = settings.raid_lead_mode;
	currentPlayer = settings.player_name;
	soundAlerts = settings.sound_alerts;

	// Get the index for the current player in the player order list (for sundering map)
	if (!playerOrder.empty())
	{
		const auto it = std::ranges::find(playerOrder, currentPlayer);
		if (it != playerOrder.end())
			destructionIndex = std::distance(playerOrder.begin(), it) + 1;
	}

	const auto& logParser = BrainRot::getInstance().getLogParser();

	logParser->subscribe(std::vector<std::wstring>{L"ENCOUNTER_START", L"ENCOUNTER_END"}, [this](const EventData&) {
		std::lock_guard lock(listMutex);
		volcanicHeartPlayers.clear();
		activeDestruction = false;
		DEBUG_PRINTLN("Clearing volcanic heart players");
	});

	logParser->subscribe(std::vector<std::wstring>{L"SPELL_AURA_APPLIED", L"SPELL_AURA_REMOVED"}, [this](const EventData& data) {
		std::vector<std::wstring> tokens;
		Utils::RegexSplit(data.eventData, std::wregex(L","), std::back_inserter(tokens));
		if (tokens.size() < 10) return;

		//const auto& casterName = tokens[2];
		const auto& targetName = tokens[6];
		const auto& spellName = tokens[10];

		if (spellName == L"\"Volcanic Heartbeat\"")
		{
			std::lock_guard lock(listMutex);
			if (data.eventType == L"SPELL_AURA_APPLIED")
			{
				WDEBUG_PRINTLN("Adding " << targetName << " to volcanic heart players");
				// Add player and sort by player order (if possible)
				volcanicHeartPlayers.push_back(targetName.substr(1, targetName.find(L"-") - 1));
				if (!playerOrder.empty())
				{
					DEBUG_PRINTLN("Sorting volcanic heart players");
					std::ranges::sort(volcanicHeartPlayers, [this](const std::wstring& a, const std::wstring& b) {
						const auto itA = std::ranges::find(playerOrder, a);
						const auto itB = std::ranges::find(playerOrder, b);

						if (itA != playerOrder.end() && itB != playerOrder.end())
							return std::distance(playerOrder.begin(), itA) < std::distance(playerOrder.begin(), itB);
						if (itA != playerOrder.end())
							return true;
						if (itB != playerOrder.end())
							return false;
						return a < b;
					});
				}
				else
					std::ranges::sort(volcanicHeartPlayers);

				// If there are 5 people, read out which number the current player is
				if (soundAlerts && volcanicHeartPlayers.size() >= 5)
				{
					const auto it = std::ranges::find(volcanicHeartPlayers, currentPlayer);
					if (it != volcanicHeartPlayers.end())
					{
						const auto index = std::distance(volcanicHeartPlayers.begin(), it) + 1;
						std::wstring command = L"play \"";
						command += std::to_wstring(index);
						command += L".wav\" from 0";
						WDEBUG_PRINTLN(command);
						mciSendString(command.c_str(), nullptr, 0, 0);
					}
				}
			}
			else
			{
				// Remove player
				const auto it = std::ranges::find(volcanicHeartPlayers, targetName.substr(1, targetName.find(L"-") - 1));
				if (it != volcanicHeartPlayers.end())
				{
					volcanicHeartPlayers.erase(it);
				}
			}
		}
	});

	// Spell cast start and success
	logParser->subscribe(std::vector<std::wstring>{L"SPELL_CAST_START", L"SPELL_CAST_SUCCESS"}, [this](const EventData& data) {
		std::vector<std::wstring> tokens;
		Utils::RegexSplit(data.eventData, std::wregex(L","), std::back_inserter(tokens));
		if (tokens.size() < 10) return;

		//const auto& casterName = tokens[2];
		const auto& spellName = tokens[10];

		if (spellName == L"\"Ebon Destruction\"")
		{
			if (data.eventType == L"SPELL_CAST_START")
				activeDestruction = true;
			else
				activeDestruction = false;
		}
	});
}

void EchoFight::drawFight()
{
	if (isFocused)
	{
		if (ImGui::Begin("Echo", nullptr, WINDOW_FLAGS))
		{
			renderPlayers({
				currentPlayer,
				currentPlayer + L" A",
				currentPlayer + L" B",
				currentPlayer + L" C",
				currentPlayer + L" D",
			});
		}
		ImGui::End();
	}
	else if (activeDestruction)
	{
		if (ImGui::Begin("Echo", nullptr, WINDOW_FLAGS))
			renderIndex();
		ImGui::End();
	}
	else if (!volcanicHeartPlayers.empty() && (isRaidLeader || std::ranges::find(volcanicHeartPlayers, currentPlayer) != volcanicHeartPlayers.end()))
	{
		std::lock_guard lock(listMutex);
		if (ImGui::Begin("Echo", nullptr, WINDOW_FLAGS))
		{
			renderPlayers(volcanicHeartPlayers);
		}
		ImGui::End();
	}
}

void EchoFight::renderPlayers(const std::vector<std::wstring>& players) const
{
	// Render a card containing the player name and their number in the list
	for (size_t i = 0; i < players.size(); i++)
	{
		const bool isCurrentPlayer = currentPlayer == players[i];
		if (!isCurrentPlayer && !isRaidLeader) continue;

		if (isRaidLeader)
		{
			if (isCurrentPlayer) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
			// format string to 1. utf8encoded name
			ImGui::Text("%llu. %s", i + 1, Utils::ToUtf8(players[i]).c_str());
			if (isCurrentPlayer) ImGui::PopStyleColor();
		}
		else
		{
			ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
			ImGui::Text("%llu", i + 1);
			ImGui::PopFont();
		}
	}
}

void EchoFight::renderIndex() const
{
	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
	ImGui::Text("%llu", destructionIndex);
	ImGui::PopFont();
}

}