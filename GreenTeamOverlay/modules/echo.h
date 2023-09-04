#pragma once

#include <mutex>
#include <vector>
#include "base_fight.h"

namespace Green {

class EchoFight final : public BaseFight
{
private:
	std::vector<std::wstring> volcanicHeartPlayers;
	std::atomic<bool> activeDestruction;
	std::mutex listMutex;
	int destructionIndex;

public:
	explicit EchoFight(HWND wowWindow);

protected:
	void drawFight() override;
	void renderPlayers(const std::vector<std::wstring>& players) const;
	void renderIndex() const;
};

}