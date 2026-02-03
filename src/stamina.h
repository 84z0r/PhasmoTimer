#pragma once
#include "singleton.h"
#include "hud_bar.h"
#include <chrono>

class CStamina : public Singleton<CStamina>
{
public:
	void OnFrameStart();
	void StaminaWindow();
	std::chrono::steady_clock::duration GetCurrentStamina() const { return this->tStamina; }

private:
	static constexpr const std::chrono::steady_clock::duration MAX_SPRINT_TIME{ std::chrono::seconds(3LL) };
	static constexpr const std::chrono::steady_clock::duration REGEN_DELAY{ std::chrono::seconds(2LL) };

	std::chrono::steady_clock::duration tStaminaBase = this->MAX_SPRINT_TIME;
	std::chrono::steady_clock::duration tStamina{};
	std::chrono::steady_clock::time_point tSprintStart{};
	std::chrono::steady_clock::time_point tSprintEnd{};
	std::chrono::steady_clock::time_point tRegenStart{};

	CHUDBar StaminaBar{
		std::string("StaminaBar"),
		std::chrono::duration<double>(this->MAX_SPRINT_TIME).count(),
		std::chrono::duration<double>(this->MAX_SPRINT_TIME).count(),
		CConfig::Get().flStaminaBarRounding,
		CConfig::Get().flStaminaBarFillRounding,
		CConfig::Get().flStaminaBarPadding,
		CConfig::Get().imvStaminaBarSize,
		CConfig::Get().imvStaminaBackgroundColor,
		CConfig::Get().imvStaminaBordersColor,
		CConfig::Get().imvStaminaColorTop,
		CConfig::Get().imvStaminaColorBottom,
		CConfig::Get().imvStaminaColorExhaustedTop,
		CConfig::Get().imvStaminaColorExhaustedBottom,
		CConfig::Get().imvStaminaFlashExhaustedColor,
		CConfig::Get().bEnableStaminaBarGloss
	};

	bool bSprinting = false;
	bool bExhausted = false;
	bool bSprintInputActive = false;

private:
	void UpdateMovement();
	void UpdateInternalTransitions();
	void UpdateStaminaValue();

	std::chrono::steady_clock::duration GetStaminaAt(std::chrono::steady_clock::time_point t) const;
};