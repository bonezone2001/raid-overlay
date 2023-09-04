#pragma once
#include <string>
#include "../core/overlay.h"

namespace Green
{

class BaseFight : public Overlay
{
private:
	std::string tempPlayerName;
	bool showEditPlayerOrder;

public:
	BaseFight(const std::string& name, HWND parentHWnd);

protected:
	virtual void drawFight() {}

private:
	void orderFromClipboard();
	void draw() override;
	bool drawBeforeFight();
};

}