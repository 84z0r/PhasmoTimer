#pragma once
#include "singleton.h"
#include <imgui/imgui.h>
#include "timer.h"

struct FadeAnim
{
	float from = 1.0f;
	float to = 1.0f;
	float duration = 0.3f;
	float elapsed = 0.0f;
	bool  active = false;
};

class CGui : public Singleton<CGui>
{
public:
	bool Init();
	void Cleanup();
	void OnFrameStart();
	void OnRender();

	bool bShowAbout = false;
	bool bShowSettings = false;
	
private:
	void TimerWindows();
	void SettingsWindow();
	void AboutWindow();
	void UpdaterWindow();
	void PerformanceWindow();
	bool KeyBindButton(const char* label, int* vk_key);
	void DrawTimerWindow(const char* window_name, const char* time_value, const ImVec4* colors, float flSize, ImVec2& imvPos, bool bGlow);
	void DrawTimerSection(const char* window_name, const char* time_value, ImVec2 windowSize, const ImVec4* colors, bool bGlow);
	void DrawTextOutline(ImDrawList* draw, ImFont* font, float size, const ImVec2& delta, const ImVec2& pos, ImU32 color, const char* text, float thickness = 1.0f);
	void DrawSoftGlow(ImDrawList* draw, ImFont* font, float size, const ImVec2& delta, const ImVec2& pos, const char* text, ImVec4 color, float flAlpha, float radius);
	void UpdateTimers();
	void StartFade(bool transparent);
	void UpdateFade();
	void ProcessFade();

	//Helper functions
	ImVec2 CalculateTextCenterDelta(ImFont* font, const char* text, float size, const ImVec2& windowPos, const ImVec2& windowSize);
	void ApplyTextCenterDelta(int vtx_start, int vtx_end, const ImVec2& delta, ImDrawList* draw);
	void TextColorGradient(int vtx_start, int vtx_end, const ImVec4* colors, ImDrawList* draw);
	
	CSmudgeTimer SmudgeTimerManual;
	CSmudgeTimer SmudgeTimerAuto;
	CObamboTimer ObamboTimer;
	CHuntTimer HuntTimer;
	CCandleTimer CandleTimer;
	FadeAnim fade;

	ImDrawList* fakeDrawList;

	bool bAutoSmudgeTimer = false;
	bool bShowPerformanceWindow = false;
};