#include "stamina.h"
#include "input.h"
#include "config.h"
#include "render.h"
#include "tools.h"
#include "imgui_wrappers.h"

constexpr const ImGuiWindowFlags stamina_window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground;

void CStamina::OnFrameStart()
{
    if (!CConfig::Get().bEnableStaminaBar)
        return;

    this->UpdateMovement();
    this->UpdateInternalTransitions();
    this->UpdateStaminaValue();
}

void CStamina::UpdateMovement()
{
    auto& input = CInput::Get();
    const InputKeyData& sprintKey = input.GetKeyData(CConfig::Get().vkSprintBind);
    const InputKeyData& forwardKeyData = input.GetKeyData(CConfig::Get().vkForwardBind);
    const InputKeyData& backwardKeyData = input.GetKeyData(CConfig::Get().vkBackwardBind);
    const InputKeyData& leftKeyData = input.GetKeyData(CConfig::Get().vkLeftBind);
    const InputKeyData& rightKeyData = input.GetKeyData(CConfig::Get().vkRightBind);

    this->bSprintInputActive = CRender::Get().IsGameWindowActive() && sprintKey.bIsDown && (forwardKeyData.bIsDown || backwardKeyData.bIsDown || leftKeyData.bIsDown || rightKeyData.bIsDown);

    if (this->bSprintInputActive && !this->bSprinting && !this->bExhausted)
    {
        const std::chrono::steady_clock::time_point tPress =
            std::max({ sprintKey.tLastPressTime,
                       forwardKeyData.tLastPressTime,
                       backwardKeyData.tLastPressTime,
                       leftKeyData.tLastPressTime,
                       rightKeyData.tLastPressTime });

        this->tStaminaBase = this->GetStaminaAt(tPress);
        this->tSprintStart = tPress;
        this->bSprinting = true;
    }

    if (!this->bSprintInputActive && this->bSprinting)
    {
        const std::chrono::steady_clock::time_point tRelease =
            std::max({ sprintKey.tLastReleaseTime,
                       forwardKeyData.tLastReleaseTime,
                       backwardKeyData.tLastReleaseTime,
                       leftKeyData.tLastReleaseTime,
                       rightKeyData.tLastReleaseTime });

        this->tStaminaBase = this->GetStaminaAt(tRelease);
        this->tSprintEnd = tRelease;
        this->tRegenStart = this->tSprintEnd + this->REGEN_DELAY;

        this->bSprinting = false;

        if (this->tStaminaBase == std::chrono::steady_clock::duration::zero())
            this->bExhausted = true;
    }
}

void CStamina::UpdateInternalTransitions()
{
    const auto now = CRender::Get().GetNowTime();

    if (this->bSprinting)
    {
        auto staminaNow = this->GetStaminaAt(now);
        if (staminaNow == std::chrono::steady_clock::duration::zero())
        {
            std::chrono::steady_clock::time_point tExhaust = this->tSprintStart + this->tStaminaBase;
            this->tStaminaBase = std::chrono::steady_clock::duration::zero();
            this->tSprintEnd = tExhaust;
            this->tRegenStart = tExhaust + this->REGEN_DELAY;
            this->bSprinting = false;
            this->bExhausted = true;
        }
    }

    if (this->bExhausted)
    {
        std::chrono::steady_clock::time_point tFullRegen = this->tRegenStart + this->MAX_SPRINT_TIME;

        if (now >= tFullRegen)
        {
            this->tStaminaBase = this->MAX_SPRINT_TIME;
            this->bExhausted = false;

            if (this->bSprintInputActive)
            {
                this->bSprinting = true;
                this->tSprintStart = tFullRegen;
            }
        }
    }
}

std::chrono::steady_clock::duration CStamina::GetStaminaAt(std::chrono::steady_clock::time_point t) const
{
    if (this->bSprinting)
    {
        auto spent = t - this->tSprintStart;
        if (spent >= this->tStaminaBase)
            return std::chrono::steady_clock::duration::zero();

        return this->tStaminaBase - spent;
    }
    else
    {
        if (t < this->tRegenStart)
            return this->tStaminaBase;

        auto regen = t - this->tRegenStart;
        auto result = this->tStaminaBase + regen;

        return std::min(result, this->MAX_SPRINT_TIME);
    }
}

void CStamina::UpdateStaminaValue()
{
	this->tStamina = this->GetStaminaAt(CRender::Get().GetNowTime());
}

void CStamina::StaminaWindow()
{
    if (!CConfig::Get().bEnableStaminaBar)
		return;

	ImGui::SetNextWindowPos(CConfig::Get().imvStaminaBarPos, ImGuiCond_Once);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);

    if (ImGui::BeginSnap("Stamina Window", nullptr, stamina_window_flags))
    {
        if (CConfig::Get().IsConfigUpdated())
            ImGui::SetWindowPos(CConfig::Get().imvStaminaBarPos);
        else
            CConfig::Get().imvStaminaBarPos = ImGui::GetWindowPos();

        this->StaminaBar.bCriticalFlash = CConfig::Get().bStaminaFlashOnExhausted ? this->bExhausted : false;
        this->StaminaBar.bUseAltFill = this->bExhausted;
        this->StaminaBar.flValue = std::chrono::duration<double>(this->tStamina).count();
        this->StaminaBar.Draw();
    }

    ImGui::End();
    ImGui::PopStyleVar(3);
}