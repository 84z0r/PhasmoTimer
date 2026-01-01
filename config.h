#pragma once
#include "singleton.h"
#include "input.h"
#include "imgui.h"

class CConfig : public Singleton<CConfig>
{
public:
	bool Load();
	bool Save();

	bool bCheckActiveWindow = true;
	bool bCheckUpdates = true;

	int vkSmudgeTimerBind = 0x52;
	int vkSwitchSmudgeTimerModeBind = 0x32;
	int vkHuntTimerBind = 0x31;
	int vkFullResetBind = 0x35;
	int vkResetBind = 0x33;
	int vkTouchBind = VK_LBUTTON;
	int vkUseBind = VK_RBUTTON;

	int64_t iStartSmudgeTimerAt = 1000i64;
	int64_t iStartHuntTimerAt = 1000i64;
	int64_t iMaxMsSmudge = 180000i64;
	int64_t iMaxMsHunt = 215000i64;

	float flSize = 80.f;
	float flRounding = 5.f;
	float flInactiveAlpha = 0.25f;

	ImVec2 imvTimerWindowPos = ImVec2(0.f, 0.f);

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

	std::string strGameProcessName = "Phasmophobia.exe";
};