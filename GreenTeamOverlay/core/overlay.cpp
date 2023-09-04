#include "overlay.h"
#include <iostream>
#include "../utils/utils.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "font.h"
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_impl_opengl3.h"

namespace Green {

Overlay::Overlay(const std::string& name, HWND parentHWnd) :
	name(name),
	parentHWnd(parentHWnd),
	isFocused(true)
{
	// Check if parent window is valid
	if (parentHWnd == NULL || !IsWindow(parentHWnd))
	{
		DEBUG_PRINTLN("Invalid parent window");
		return;
	}

	// Get location and size of parent window client area
	// Get top left window position
	RECT clientRect;
	POINT parentPos;
	GetClientRect(parentHWnd, &clientRect);
	parentPos.x = clientRect.left;
	parentPos.y = clientRect.top;
	ClientToScreen(parentHWnd, &parentPos);

	// Get width and height of window
	width = clientRect.right - clientRect.left - 1;
	height = clientRect.bottom - clientRect.top - 1;

	// Initialize GLFW if not already initialized
	if (!glfwInit())
	{
		DEBUG_PRINTLN("Failed to initialize GLFW");
		return;
	}

	// Setup window hints
	glfwWindowHint(GLFW_FLOATING, true);
	glfwWindowHint(GLFW_DECORATED, false);
	glfwWindowHint(GLFW_RESIZABLE, false);
	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, true);
	glfwWindowHint(GLFW_SAMPLES, 2);

	// Setup OpenGL version
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	// Create window
	window = glfwCreateWindow(width, height, name.c_str(), NULL, NULL);
	if (window == NULL)
	{
		DEBUG_PRINTLN("Failed to create GLFW window");
		glfwTerminate();
		return;
	}

	// Set window position
	glfwSetWindowPos(window, parentPos.x, parentPos.y);

	glfwMakeContextCurrent(window);
	DEBUG_PRINTLN("GLFW window created");
	glfwSwapInterval(1);

	// Set OpenGL window attributes
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, width - 1, 0, height - 1, -1, 1);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Set style of window from window handle
	const HWND hWnd = glfwGetWin32Window(window);
	LONG exStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
	exStyle |= WS_EX_LAYERED;
	SetWindowLong(hWnd, GWL_EXSTYLE, exStyle);
	SetWindowLong(hWnd, GWL_EXSTYLE, exStyle | WS_EX_LAYERED);

	// Detect if window is focused at any point
	glfwSetWindowUserPointer(window, this);
	glfwSetWindowFocusCallback(window, [](GLFWwindow* window, int focused)
	{
		// If window is focused, set transparent to false
		const auto overlay = static_cast<Overlay*>(glfwGetWindowUserPointer(window));
		overlay->isFocused = focused;
		if (focused)
		{
			const HWND hWnd = glfwGetWin32Window(window);
			SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) & ~WS_EX_TRANSPARENT);
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
		else
		{
			const HWND hWnd = glfwGetWin32Window(window);
			SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_TRANSPARENT);
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
		DEBUG_PRINTLN("Window focus changed");
	});

	// Initialize ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330 core");

	// Set custom ImGui settings file name
	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = "gui_settings.ini";

	initStyle();
}

Overlay::~Overlay()
{
	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(window);
	glfwTerminate();
}

void Overlay::drawInternal()
{
	// Clear to transparent
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	draw();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Overlay::start()
{
	onStart();
	while (!glfwWindowShouldClose(window))
	{
		drawInternal();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

void Overlay::stop()
{
	glfwSetWindowShouldClose(window, true);
	onStop();
}

void Overlay::initStyle()
{
	ImGuiStyle& style = ImGui::GetStyle();

	// Customize colors
	style.Colors[ImGuiCol_Text] = ImVec4(0.85f, 0.85f, 0.85f, 1.0f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
	style.Colors[ImGuiCol_ChildBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.4f, 0.4f, 0.4f, 1.0f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.1f, 0.1f, 0.1f, 0.5f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.4f, 0.4f, 0.4f, 1.0f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.4f, 0.4f, 0.4f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);

	// Customize sizing
	style.WindowRounding = 5.0f;
	style.FrameRounding = 3.0f;
	style.GrabRounding = 3.0f;
	style.ScrollbarRounding = 3.0f;

	// Load verdana font in memory at VerdanaFont_compressed_data
	const auto& io = ImGui::GetIO();

	ImFontConfig config;
	config.PixelSnapH = true;
	config.OversampleH = 2;
	config.OversampleV = 2;

	// Setup Glyph Range Builder, these will be static for now because of laziness
	static ImVector<ImWchar> ranges;
	static ImFontGlyphRangesBuilder builder;
	builder.AddRanges(io.Fonts->GetGlyphRangesDefault());
	builder.AddRanges(io.Fonts->GetGlyphRangesCyrillic());
	builder.BuildRanges(&ranges);

	io.Fonts->AddFontFromMemoryCompressedTTF(VerdanaFont_compressed_data, VerdanaFont_compressed_size, 18.0f, &config, ranges.Data);
	io.Fonts->AddFontFromMemoryCompressedTTF(VerdanaFont_compressed_data, VerdanaFont_compressed_size, 60.0f, &config, ranges.Data);

}
}
