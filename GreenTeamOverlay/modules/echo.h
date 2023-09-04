#pragma once

#include <mutex>
#include <vector>
#include "base_fight.h"

namespace Green {

class EchoFight final : public BaseFight
{
private:
	bool isRaidLeader;
	std::vector<std::wstring> playerOrder;
	std::vector<std::wstring> volcanicHeartPlayers;
	std::atomic<bool> activeDestruction;
	std::wstring currentPlayer;
	std::mutex listMutex;
	int destructionIndex;
	bool soundAlerts;

public:
	explicit EchoFight(HWND wowWindow);

protected:
	void drawFight() override;
	void renderPlayers(const std::vector<std::wstring>& players) const;
	void renderIndex() const;
};

}