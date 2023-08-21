#pragma once

#include <atomic>
#include <string>
#include <Windows.h>
#include <GLFW/glfw3.h>

namespace Green
{
	
class Overlay
{
private:
	std::string name;
	GLFWwindow* window;
	HWND parentHWnd;
	int width, height;

protected:
	std::atomic<bool> isFocused;

public:
	Overlay(const std::string& name, HWND parentHWnd);
	virtual ~Overlay();

	void start();
	void stop();

protected:
	virtual void draw() {}
	virtual void onStart() {}
	virtual void onStop() {}

private:
	void drawInternal();
	static void initStyle();
};

}