#include "base_fight.h"

#include <iostream>

#include "../imgui/imgui.h"
#include "../imgui/imgui_stdlib.h"
#include "../core/settings.h"

namespace Green
{

BaseFight::BaseFight(const std::string& name, HWND parentHWnd) :
	Overlay(name, parentHWnd),
	showEditPlayerOrder(false)
{
	tempPlayerName.reserve(PLAYER_NAME_MAX_LENGTH);
}

void BaseFight::draw()
{
	if (!drawBeforeFight()) return;
	drawFight();
}

void BaseFight::orderFromClipboard()
{
	// Get clipboard text
	const std::wstring clipboardText = Utils::GetClipboardText();
	if (clipboardText.empty())
	{
		MessageBox(nullptr, L"Clipboard is empty!", L"Error", MB_OK | MB_ICONERROR);
	}

	// Split by newline
	std::vector<std::wstring> lines;
	Utils::RegexSplit(clipboardText, std::wregex(L"\r\n|\n|\r"), std::back_inserter(lines));

	// Remove empty lines
	lines.erase(std::ranges::remove_if(lines, [](const std::wstring& line)
	{
		return line.empty();
	}).begin(), lines.end());

	// Remove MRT formatting tags (e.g |cFF00FF00, |r, {something}, etc)
	for (std::wstring& line : lines) {
		line = std::regex_replace(line, std::wregex(L"\\|c[0-9A-Fa-f]{8}|\\|r|\\{[^}]*\\}"), L"");
	}

	// Trim whitespace
	std::ranges::for_each(lines, [](std::wstring& line)
	{
		line = std::regex_replace(line, std::wregex(L"^ +| +$|( ) +"), L"$1");
	});

	// Remove lines that are too long or too short
	lines.erase(std::ranges::remove_if(lines, [](const std::wstring& line)
	 {
		return line.length() > PLAYER_NAME_MAX_LENGTH || line.length() < PLAYER_NAME_MIN_LENGTH;
	}).begin(), lines.end());

	// Remove liquidStart and liquidEnd
	lines.erase(std::ranges::remove_if(lines, [](const std::wstring& line)
	{
		return line == L"liquidStart" || line == L"liquidEnd";
	}).begin(), lines.end());

	// Set the player order
	auto& settings = Settings::getInstance();
	settings.echo_player_order.clear();
	for (const std::wstring& line : lines)
	{
		settings.echo_player_order.emplace_back(line);
	}
	settings.save();
}

bool BaseFight::drawBeforeFight()
{
	auto& settings = Settings::getInstance();

	// If we're missing the player name from settings, show a text box to enter it
	if (settings.player_name.empty())
	{
		// Make window appear in the middle of the screen
		ImGui::SetNextWindowPos(ImVec2((ImGui::GetIO().DisplaySize.x / 2), (ImGui::GetIO().DisplaySize.y / 2)), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		if (isFocused) {
			// Allow to enter the name
			if (ImGui::Begin("Missing Character Name!", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove))
			{
				ImGui::Text("Please enter your character name:");
				if (ImGui::InputTextWithHint("##player_name", "OrcLoserAbc", &tempPlayerName, ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
				{
					settings.player_name = std::wstring(tempPlayerName.begin(), tempPlayerName.end());
					settings.save();
					tempPlayerName.clear();
				}
				ImGui::SameLine();
				if (ImGui::Button("Done"))
				{
					settings.player_name = std::wstring(tempPlayerName.begin(), tempPlayerName.end());
					settings.save();
					tempPlayerName.clear();
				}
			}
		}
		else
		{
			// Show alert that we need them to focus the window and enter the name
			if (ImGui::Begin("Missing Character Name!", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove))
			{
				ImGui::Text("Please click onto the overlay and enter your character name!");
				ImGui::Text("Meaning alt-tab into the program <3");
			}
		}
		ImGui::End();
		return false;
	}

	if (isFocused) {
		// Show edit player order window
		if (showEditPlayerOrder)
		{
			ImGui::SetNextWindowPos(ImVec2((ImGui::GetIO().DisplaySize.x / 2), (ImGui::GetIO().DisplaySize.y / 2)), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
			if (ImGui::Begin("Player Order", &showEditPlayerOrder, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove))
			{
				if (ImGui::Button("Copy from Clipboard (Supports MRT tags)"))
				{
					orderFromClipboard();
				}
				ImGui::Text("Drag and drop players to change the order:");
				ImGui::Separator();
				for (int i = 0; i < settings.echo_player_order.size(); i++)
				{
					auto& player = settings.echo_player_order[i];
					const std::string playerName = Utils::ToUtf8(player.c_str());
					ImGui::Text("%s", playerName.c_str());
					if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
					{
						ImGui::SetDragDropPayload("DND_PLAYER_ORDER", &i, sizeof(int));
						ImGui::Text("%s", playerName.c_str());
						ImGui::EndDragDropSource();
					}
					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_PLAYER_ORDER"))
						{
							int newIndex = *(const int*)payload->Data;
							if (newIndex != i)
							{
								// Swap the players
								std::swap(settings.echo_player_order[i], settings.echo_player_order[newIndex]);
								settings.save();
							}
						}
						ImGui::EndDragDropTarget();
					}
				}
			}
			ImGui::End();
			return false;
		}


		// Show settings window
		if (ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			// Show the player name and allow to reset it
			const std::string playerName = Utils::ToUtf8(settings.player_name.c_str());
			ImGui::Text("Player Name: %s", (settings.player_name.empty() ? "None" : playerName.c_str()));
			if (ImGui::Button("Edit Player Name"))
			{
				settings.player_name.clear();
				settings.save();
			}

			// Allow toggle raid leader mode
			if (ImGui::Checkbox("Raid Leader Mode", &settings.raid_lead_mode))
			{
				std::cout << "Raid Leader Mode: " << settings.raid_lead_mode << std::endl;
				settings.save();
			}

			// Allow toggle sound alerts
			if (ImGui::Checkbox("Sound Alerts", &settings.sound_alerts))
			{
				std::cout << "Sound Alerts: " << settings.sound_alerts << std::endl;
				settings.save();
			}

			// Allow editing player order
			if (ImGui::Button("Edit Player Order"))
			{
				showEditPlayerOrder = !showEditPlayerOrder;
			}
		}
		ImGui::End();
	}

	return true;
}

}
