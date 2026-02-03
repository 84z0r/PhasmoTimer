#pragma once
#include "singleton.h"
#include "input.h"
#include <imgui/imgui.h>
#include <filesystem>

class CConfig : public Singleton<CConfig>
{
public:
	CConfig();
	bool Load(bool bFirstLoad = false);
	bool Save();
	void OnFrameEnd();
	bool IsKeyAlreadyBound(int vk, int* except) const;

	inline bool IsConfigUpdated() const { return this->bConfigUpdated; }

	bool bCheckActiveWindow = true;
	bool bCheckUpdates = true;
	bool bEnableSplitMode = false;
	bool bEnableSplitObambo = false;
	bool bEnableSplitHunt = false;
	bool bEnableSplitCandle = false;
	bool bScanSystemFonts = true;
	bool bScanUserFonts = true;
	bool bScanAppFonts = true;
	bool bEnableStaminaBar = false;
	bool bEnableStaminaBarGloss = true;
	bool bStaminaFlashOnExhausted = true;

	//Timer binds
	int vkSmudgeTimerBind = 0x52;
	int vkSwitchSmudgeTimerModeBind = 0x32;
	int vkHuntTimerBind = 0x31;
	int vkCandleTimerBind = 0x54;
	int vkFullResetBind = 0x35;
	int vkResetBind = 0x33;

	//Game binds
	int vkTouchBind = VK_LBUTTON;
	int vkUseBind = VK_RBUTTON;
	int vkSprintBind = VK_SHIFT;
	int vkForwardBind = 0x57;
	int vkBackwardBind = 0x53;
	int vkLeftBind = 0x41;
	int vkRightBind = 0x44;

	int64_t iStartSmudgeTimerAt = 1000LL;
	int64_t iStartHuntTimerAt = 1000LL;
	int64_t iMaxMsSmudge = 180000LL;
	int64_t iMaxMsHunt = 215000LL;

	float flSize = 80.f;
	float flSmudgeTimerSize = 56.f;
	float flObamboTimerSize = 56.f;
	float flHuntTimerSize = 56.f;
	float flCandleTimerSize = 56.f;
	float flRounding = 5.f;
	float flInactiveAlpha = 0.25f;
	float flStaminaBarRounding = 6.f;
	float flStaminaBarFillRounding = 3.f;
	float flStaminaBarPadding = 6.f;

	ImVec2 imvTimerWindowPos = ImVec2(0.f, 0.f);
	ImVec2 imvSmudgeTimerWindowPos = ImVec2(0.f, 0.f);
	ImVec2 imvObamboTimerWindowPos = ImVec2(0.f, 55.f);
	ImVec2 imvHuntTimerWindowPos = ImVec2(0.f, 110.f);
	ImVec2 imvCandleTimerWindowPos = ImVec2(0.f, 165.f);

	ImVec2 imvStaminaBarPos = ImVec2(0.f, 0.f);
	ImVec2 imvStaminaBarSize = ImVec2(300.f, 40.f);

	ImVec4 imvBackgroundColor = ImVec4(0.06f, 0.06f, 0.06f, 0.65f);
	ImVec4 imvBordersColor = ImVec4(0.7f, 0.7f, 0.7f, 0.5f);
	ImVec4 imvGlowColor1 = ImVec4(0.60f, 0.90f, 1.00f, 1.f);
	ImVec4 imvGlowColor2 = ImVec4(0.10f, 0.55f, 0.75f, 1.f);

	ImVec4 imvSafeTimeColor1 = ImVec4(0.65f, 1.00f, 0.65f, 1.0f);
	ImVec4 imvSafeTimeColor2 = ImVec4(0.15f, 0.60f, 0.20f, 1.0f);
	ImVec4 imvDemonTimeColor1 = ImVec4(1.00f, 0.80f, 0.45f, 1.0f);
	ImVec4 imvDemonTimeColor2 = ImVec4(0.85f, 0.35f, 0.05f, 1.0f);
	ImVec4 imvHuntTimeColor1 = ImVec4(1.00f, 0.55f, 0.55f, 1.0f);
	ImVec4 imvHuntTimeColor2 = ImVec4(0.70f, 0.05f, 0.05f, 1.0f);

	ImVec4 imvObamboCalmColor1 = ImVec4(0.60f, 0.90f, 1.00f, 1.0f);
	ImVec4 imvObamboCalmColor2 = ImVec4(0.10f, 0.55f, 0.75f, 1.0f);
	ImVec4 imvObamboAggressiveColor1 = ImVec4(1.00f, 0.55f, 0.55f, 1.0f);
	ImVec4 imvObamboAggressiveColor2 = ImVec4(0.70f, 0.05f, 0.05f, 1.0f);

	ImVec4 imvHuntTimerColor1 = ImVec4(1.00f, 1.00f, 0.70f, 1.0f);
	ImVec4 imvHuntTimerColor2 = ImVec4(0.85f, 0.70f, 0.10f, 1.0f);

	ImVec4 imvCandleTimerColor1 = ImVec4(0.55f, 0.95f, 0.85f, 1.0f);
	ImVec4 imvCandleTimerColor2 = ImVec4(0.10f, 0.55f, 0.50f, 1.0f);

	ImVec4 imvStaminaBackgroundColor = ImVec4(0.06f, 0.06f, 0.06f, 0.65f);
	ImVec4 imvStaminaBordersColor = ImVec4(0.7f, 0.7f, 0.7f, 0.5f);
	ImVec4 imvStaminaColorTop = ImVec4(0.65f, 1.00f, 0.35f, 0.9f);
	ImVec4 imvStaminaColorBottom = ImVec4(0.3f, 0.65f, 0.1f, 0.9f);
	ImVec4 imvStaminaColorExhaustedTop = ImVec4(0.95f, 0.65f, 0.35f, 0.9f);
	ImVec4 imvStaminaColorExhaustedBottom = ImVec4(0.80f, 0.25f, 0.05f, 0.9f);
	ImVec4 imvStaminaFlashExhaustedColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);

	std::string strGameProcessName = "Phasmophobia.exe";

	std::filesystem::path fontFileName = "Default";

private:
	std::vector<const int*> GetAllKeybinds() const;

	std::filesystem::path configFilePath;
	bool bConfigUpdated = false;
};