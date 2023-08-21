#pragma once

#include <mutex>
#include <vector>
#include "../core/overlay.h"

namespace Green {

class EchoFight final : public Overlay
{
private:
	bool isRaidLeader;
	std::vector<std::wstring> playerOrder;
	std::vector<std::wstring> volcanicHeartPlayers;
	std::atomic<bool> activeDestruction;
	std::wstring currentPlayer;
	std::mutex listMutex;
	int destructionIndex;

public:
	explicit EchoFight(HWND wowWindow);

protected:
	void draw() override;
	void renderPlayers(const std::vector<std::wstring>& players) const;
	void renderIndex() const;
};

}