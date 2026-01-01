#include "gui.h"
#include "config.h"
#include "foregroundtracker.h"
#include "tools.h"
#include "imgui_wrappers.h"
#include "version.h"
#include "font_comicz.h"
#include "logo.h"
#include "resource.h"
#include "updater.h"
#include <algorithm>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.hpp"

constexpr const ImGuiWindowFlags timer_window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse;
constexpr const float top_ratio = 0.65f;
constexpr const float bottom_ratio = (1.f - top_ratio) * 0.5f;

void CGui::RenderLoop()
{
    while (!this->bWantExit)
    {
        if (this->StartNewFrame())
        {
            this->UpdaterWindow();
            this->TimerWindow();
            this->SettingsWindow();
            this->AboutWindow();
            this->EndNewFrame();
        }
    }
}

void CGui::TimerWindow()
{
    ImVec2 size((CConfig::Get().flSize * 2.7f), CConfig::Get().flSize);

    ImGui::SetNextWindowSize(size);
    ImGui::SetNextWindowPos(CConfig::Get().imvTimerWindowPos, ImGuiCond_Once);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, CConfig::Get().flRounding);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, CConfig::Get().imvBackgroundColor);
    ImGui::PushStyleColor(ImGuiCol_Border, CConfig::Get().imvBordersColor);
    ImGui::PushStyleColor(ImGuiCol_Separator, CConfig::Get().imvBordersColor);

    bool bOpened = ImGui::Begin("TimerWindow", nullptr, timer_window_flags);
    ImGui::PopStyleVar(2);

    if (bOpened)
    {
        CConfig::Get().imvTimerWindowPos = ImGui::GetWindowPos();

        if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
            ImGui::OpenPopup("TimerContextMenu");

        if (ImGui::BeginPopup("TimerContextMenu"))
        {
            if (ImGui::MenuItem("Settings")) this->bShowSettings = true;
            if (ImGui::MenuItem("About")) this->bShowAbout = true;
            if (ImGui::MenuItem("Exit")) this->bWantExit = true;
            ImGui::EndPopup();
        }

        ImVec2 available_size = ImGui::GetContentRegionAvail();

        if (this->bAutoSmudgeTimer)
            DrawTimerSection("Main", this->SmudgeTimerAuto.GetString(), top_ratio, available_size, this->SmudgeTimerAuto.GetColors(), true);
        else
            DrawTimerSection("Main", this->SmudgeTimerManual.GetString(), top_ratio, available_size, this->SmudgeTimerManual.GetColors(), true);

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
        ImGui::Separator();

        float bottom_total_height = available_size.y * (bottom_ratio * 2);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        if (ImGui::BeginChild("BottomContainer", ImVec2(available_size.x, bottom_total_height), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground))
        {
            ImVec2 bottom_size = ImGui::GetContentRegionAvail();

            ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0.0f, 0.0f));

            if (ImGui::BeginTable("BottomTimersTable", 2, ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_NoBordersInBodyUntilResize | ImGuiTableFlags_SizingFixedFit))
            {
                ImGui::TableSetupColumn("Left", ImGuiTableColumnFlags_WidthFixed, bottom_size.x * 0.5f);
                ImGui::TableSetupColumn("Right", ImGuiTableColumnFlags_WidthFixed, bottom_size.x * 0.5f);

                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                DrawTimerSection("Obambo", this->ObamboTimer.GetString(), 1.0f, ImVec2(bottom_size.x * 0.5f, bottom_size.y), this->ObamboTimer.GetColors(), false);

                ImGui::TableSetColumnIndex(1);
                DrawTimerSection("Hunt", this->HuntTimer.GetString(), 1.0f, ImVec2(bottom_size.x * 0.5f, bottom_size.y), this->HuntTimer.GetColors(), false);

                ImGui::EndTable();
            }
            ImGui::PopStyleVar();

            ImVec2 win_pos = ImGui::GetWindowPos();

            float x = win_pos.x + bottom_size.x * 0.5f;
            float y2 = win_pos.y + bottom_size.y;

            ImGui::GetWindowDrawList()->AddLine(ImVec2(x, win_pos.y), ImVec2(x, y2), ImGui::GetColorU32(ImGuiCol_Separator), 1.f);
        }
        ImGui::EndChild();
        ImGui::PopStyleVar(2);  
    }
    ImGui::End();
    ImGui::PopStyleColor(3);
}

void CGui::SettingsWindow()
{
    if (!this->bShowSettings)
        return;

    const auto& Style = ImGui::GetStyle();
    const ImVec2 window_size(640 * Style.FontScaleDpi, 360 * Style.FontScaleDpi);
    ImGui::SetNextWindowSize(window_size, ImGuiCond_Once);
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 center((io.DisplaySize.x - window_size.x) * 0.5f, (io.DisplaySize.y - window_size.y) * 0.5f);
    ImGui::SetNextWindowPos(center, ImGuiCond_Once);

    if (ImGui::Begin("Settings", &this->bShowSettings, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
    {
        if (ImGui::BeginTabBar("SettingsTabs", ImGuiTabBarFlags_None))
        {
            if (ImGui::BeginTabItem("Main"))
            {
                ImGui::Columns(2, nullptr, false);

                ImGui::SeparatorText("Timer Window");
                ImGui::PushItemWidth(215.f * Style.FontScaleDpi);
                ImGui::SliderFloat("Size", &CConfig::Get().flSize, 30.f, 500.f);
                ImGui::SliderFloat("Rounding", &CConfig::Get().flRounding, 0.f, 20.f);
                ImGui::PopItemWidth();

                ImGui::SeparatorText("Timer Settings");
                ImGui::PushItemWidth(145.f * Style.FontScaleDpi);
                ImGui::InputScalar("Smudge Timer Start Value", ImGuiDataType_S64, &CConfig::Get().iStartSmudgeTimerAt);
                ImGui::SetItemTooltip("Initial timer value in milliseconds");
                ImGui::InputScalar("Smudge Timer Max Value", ImGuiDataType_S64, &CConfig::Get().iMaxMsSmudge);
                ImGui::SetItemTooltip("Maximum timer value in milliseconds");
                ImGui::InputScalar("Hunt Timer Start Value", ImGuiDataType_S64, &CConfig::Get().iStartHuntTimerAt);
                ImGui::SetItemTooltip("Initial timer value in milliseconds");
                ImGui::InputScalar("Hunt Timer Max Value", ImGuiDataType_S64, &CConfig::Get().iMaxMsHunt);
                ImGui::SetItemTooltip("Maximum timer value in milliseconds");
                ImGui::PopItemWidth();

                ImGui::SeparatorText("Misc");
                ImGui::Checkbox("Check Active Window", &CConfig::Get().bCheckActiveWindow);
                ImGui::SetItemTooltip("When the game window is inactive all binds will\nbe ignored and the global alpha will be lowered");
                if (CConfig::Get().bCheckActiveWindow)
                {
                    ImGui::PushItemWidth(215.f * Style.FontScaleDpi);
                    if (ImGui::InputText("Process Name", CConfig::Get().strGameProcessName))
                        CGameForegroundTracker::Get().SetProcessName(CConfig::Get().strGameProcessName);

                    ImGui::SliderFloat("Inactive Alpha", &CConfig::Get().flInactiveAlpha, 0.f, 1.f);
                    ImGui::PopItemWidth();
                }

                ImGui::NextColumn();

                ImGui::SeparatorText("Keybinds");
                this->KeyBindButton("Smudge Timer", &CConfig::Get().vkSmudgeTimerBind);
                this->KeyBindButton("Smudge Timer Mode", &CConfig::Get().vkSwitchSmudgeTimerModeBind);
                this->KeyBindButton("Hunt Timer", &CConfig::Get().vkHuntTimerBind);
                this->KeyBindButton("Full Reset", &CConfig::Get().vkFullResetBind);
                this->KeyBindButton("Reset", &CConfig::Get().vkResetBind);
                ImGui::SetItemTooltip("Hold \"Reset\" bind and any timer\nbind to reset the timer");
                this->KeyBindButton("Touch", &CConfig::Get().vkTouchBind);
                this->KeyBindButton("Use", &CConfig::Get().vkUseBind);
                ImGui::SetItemTooltip("WARNING: The \"Use\" bind in game\nshould be the save as the \"Use Hold\"!");

                ImGui::SeparatorText("Update");
                ImGui::Checkbox("Check For Updates", &CConfig::Get().bCheckUpdates);

                ImGui::Columns(1);
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Colors"))
            {
                ImGui::Columns(2, nullptr, false);
                ImGui::SeparatorText("Timer Window##timer_window_colors");

                ImGui::ColorEdit4("Background", reinterpret_cast<float*>(&CConfig::Get().imvBackgroundColor), ImGuiColorEditFlags_NoInputs);
                ImGui::ColorEdit4("Borders", reinterpret_cast<float*>(&CConfig::Get().imvBordersColor), ImGuiColorEditFlags_NoInputs);

                ImGui::SeparatorText("Smudge Timer##smudge_colors");

                ImGui::ColorEdit4("Glow 1 Layer", reinterpret_cast<float*>(&CConfig::Get().imvGlowColor1), ImGuiColorEditFlags_NoInputs);
                ImGui::ColorEdit4("Glow 2 Layer", reinterpret_cast<float*>(&CConfig::Get().imvGlowColor2), ImGuiColorEditFlags_NoInputs);
                ImGui::ColorEdit4("Safe Time Top", reinterpret_cast<float*>(&CConfig::Get().imvSafeTimeColor1), ImGuiColorEditFlags_NoInputs);
                ImGui::ColorEdit4("Safe Time Bottom", reinterpret_cast<float*>(&CConfig::Get().imvSafeTimeColor2), ImGuiColorEditFlags_NoInputs);
                ImGui::ColorEdit4("Demon Time Top", reinterpret_cast<float*>(&CConfig::Get().imvDemonTimeColor1), ImGuiColorEditFlags_NoInputs);
                ImGui::ColorEdit4("Demon Time Bottom", reinterpret_cast<float*>(&CConfig::Get().imvDemonTimeColor2), ImGuiColorEditFlags_NoInputs);
                ImGui::ColorEdit4("Hunt Time Top", reinterpret_cast<float*>(&CConfig::Get().imvHuntTimeColor1), ImGuiColorEditFlags_NoInputs);
                ImGui::ColorEdit4("Hunt Time Bottom", reinterpret_cast<float*>(&CConfig::Get().imvHuntTimeColor2), ImGuiColorEditFlags_NoInputs);

                ImGui::NextColumn();

                ImGui::SeparatorText("Hunt Timer##hunt_timer_colors");

                ImGui::ColorEdit4("Hunt Timer Top", reinterpret_cast<float*>(&CConfig::Get().imvHuntTimerColor1), ImGuiColorEditFlags_NoInputs);
                ImGui::ColorEdit4("Hunt Timer Bottom", reinterpret_cast<float*>(&CConfig::Get().imvHuntTimerColor2), ImGuiColorEditFlags_NoInputs);

                ImGui::SeparatorText("Obambo Timer##obambo_timer_colors");

                ImGui::ColorEdit4("Obambo Calm Top", reinterpret_cast<float*>(&CConfig::Get().imvObamboCalmColor1), ImGuiColorEditFlags_NoInputs);
                ImGui::ColorEdit4("Obambo Calm Bottom", reinterpret_cast<float*>(&CConfig::Get().imvObamboCalmColor2), ImGuiColorEditFlags_NoInputs);
                ImGui::ColorEdit4("Obambo Aggressive Top", reinterpret_cast<float*>(&CConfig::Get().imvObamboAggressiveColor1), ImGuiColorEditFlags_NoInputs);
                ImGui::ColorEdit4("Obambo Aggressive Bottom", reinterpret_cast<float*>(&CConfig::Get().imvObamboAggressiveColor2), ImGuiColorEditFlags_NoInputs);

                ImGui::Columns(1);
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImVec2 btn_size(90 * Style.FontScaleDpi, 25 * Style.FontScaleDpi);
        ImVec2 content_max = ImGui::GetWindowContentRegionMax();
        ImVec2 content_min = ImGui::GetWindowContentRegionMin();
        ImVec2 window_pos = ImGui::GetWindowPos();
        ImVec2 button_pos;
        button_pos.x = window_pos.x + content_max.x - (btn_size.x * 2 + 10.f);
        button_pos.y = window_pos.y + content_max.y - btn_size.y - 5.f;

        ImGui::SetCursorScreenPos(button_pos);
        if (ImGui::Button("Load", btn_size)) CConfig::Get().Load();
        ImGui::SetItemTooltip("Load the last saved config");
        ImGui::SameLine();
        if (ImGui::Button("Save", btn_size)) CConfig::Get().Save();
        ImGui::SetItemTooltip("Save current configuration\nand timer window position");
    }
    ImGui::End();
}

void CGui::AboutWindow()
{
    static bool bPrevShow = false;
    static const ImVec4 name_color(0.50f, 1.00f, 0.00f, 1.00f);

    if (this->bShowAbout)
    {
        const auto& Style = ImGui::GetStyle();
        const ImVec2 windowSize(300 * Style.FontScaleDpi, 165 * Style.FontScaleDpi);
        ImGui::SetNextWindowSize(windowSize, ImGuiCond_Once);

        if (this->bShowAbout != bPrevShow)
            ImGui::OpenPopup("About##modal_about");

        if (ImGui::BeginPopupModal("About##modal_about", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
        {
            ImGui::Columns(2, nullptr, false);
            ImGui::PushStyleColor(ImGuiCol_Text, name_color);
            ImGui::Text("PhasmoTimer\n");
            ImGui::PopStyleColor();
            ImGui::Text("Version: %s\n\n", Version::APP_VERSION);
            ImGui::Text("Made by");
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, name_color);
            ImGui::Text("atlas2884");
            ImGui::PopStyleColor();
            ImGui::NextColumn();
            float column_width = ImGui::GetColumnWidth();
            float image_width = 80.0f;
            float offset = (column_width - image_width) * 0.5f;
            if (offset > 0.0f) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
            ImGui::Image(reinterpret_cast<ImTextureID>(this->logo_texture), ImVec2(image_width, 80.f));
            ImGui::Columns(1);
            ImGui::Text("Links: ");
            ImGui::SameLine();
            if (ImGui::Link("GitHub", L"https://github.com/84z0r/PhasmoTimer")) { ImGui::CloseCurrentPopup(); this->bShowAbout = false; }
            ImGui::SameLine();
            if (ImGui::Link("Twitch", L"https://twitch.tv/atlas2884")) { ImGui::CloseCurrentPopup(); this->bShowAbout = false; }
            ImGui::SameLine();
            if (ImGui::Link("YouTube", L"https://www.youtube.com/@phasmonoevidence")) { ImGui::CloseCurrentPopup(); this->bShowAbout = false; }
            ImGui::SameLine();
            if (ImGui::Link("Telegram", L"https://t.me/atlas2884")) { ImGui::CloseCurrentPopup(); this->bShowAbout = false; }
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            float flButtonSize = 80.f * Style.FontScaleDpi;
            ImGui::SetCursorPosX((windowSize.x - flButtonSize) * 0.5f);
            if (ImGui::Button("OK", ImVec2(flButtonSize, 0)))
            {
                ImGui::CloseCurrentPopup();
                this->bShowAbout = false;
            }

            ImGui::EndPopup();
        }
    }

    bPrevShow = this->bShowAbout;
}

void CGui::UpdaterWindow()
{
    if (!CConfig::Get().bCheckUpdates)
        return;

    static bool bPrevShow = false;
    static const ImVec4 name_color(0.50f, 1.00f, 0.00f, 1.00f);
    bool bShowUpdate = CUpdater::Get().bUpdateAvailable.load(std::memory_order::relaxed);

    if (bShowUpdate)
    {
        const auto& Style = ImGui::GetStyle();
        const ImVec2 windowSize(300 * Style.FontScaleDpi, 125 * Style.FontScaleDpi);
        ImGui::SetNextWindowSize(windowSize, ImGuiCond_Once);

        if (bShowUpdate != bPrevShow)
            ImGui::OpenPopup("Update##modal_update");

        if (ImGui::BeginPopupModal("Update##modal_update", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
        {
            const float contentWidth = ImGui::GetContentRegionAvail().x;

            ImGui::Spacing();
            ImGui::Spacing();

            const char* textLeft = "A new version of";
            const char* textName = "PhasmoTimer";
            const char* textRight = "is available!";

            float textWidth = ImGui::CalcTextSize(textLeft).x + ImGui::CalcTextSize(textName).x + ImGui::CalcTextSize(textRight).x;

            ImGui::SetCursorPosX((contentWidth - textWidth) * 0.5f);
            ImGui::Text("%s", textLeft);
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, name_color);
            ImGui::Text("%s", textName);
            ImGui::PopStyleColor();
            ImGui::SameLine();
            ImGui::Text("%s", textRight);

            ImGui::Spacing();
            ImGui::Spacing();

            const char* textDownload = "You can download it on";
            float textWidthDownload = ImGui::CalcTextSize(textDownload).x + ImGui::CalcTextSize("GitHub").x;

            ImGui::SetCursorPosX((contentWidth - textWidthDownload) * 0.5f);
            ImGui::Text("%s", textDownload);
            ImGui::SameLine();
            if (ImGui::Link("GitHub", L"https://github.com/84z0r/PhasmoTimer/releases"))
            {
                ImGui::CloseCurrentPopup();
                bShowUpdate = false;
                CUpdater::Get().bUpdateAvailable.store(bShowUpdate, std::memory_order::relaxed);
            }

            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::Spacing();

            float flButtonSize = 80.f * Style.FontScaleDpi;

            ImGui::SetCursorPosX((windowSize.x - flButtonSize) * 0.5f);
            if (ImGui::Button("OK", ImVec2(flButtonSize, 0)))
            {
                ImGui::CloseCurrentPopup();
                bShowUpdate = false;
                CUpdater::Get().bUpdateAvailable.store(bShowUpdate, std::memory_order::relaxed);
            }

            ImGui::EndPopup();
        }
    }

    bPrevShow = bShowUpdate;
}

bool CGui::KeyBindButton(const char* label, int* vk_key)
{
    static bool waiting = false;
    static int* active_key = nullptr;
    const auto& Style = ImGui::GetStyle();

    char button_text[64];

    if (waiting && active_key == vk_key) strcpy_s(button_text, "Press a key...");
    else
    {
        if (*vk_key) sprintf_s(button_text, "[ %s ]##%s", CTools::Get().GetKeyNameVK(*vk_key), label);   
        else strcpy_s(button_text, "[ None ]");
    }

    if (ImGui::Button(button_text, ImVec2(140.f * Style.FontScaleDpi, 0.f)))
    {
        waiting = true;
        active_key = vk_key;
    }

    ImGui::SameLine();
    ImGui::Text("%s", label);

    if (waiting && active_key == vk_key)
    {
        const auto& keymap = CInput::Get().GetKeyDataMap();
        auto key = std::find_if(keymap.cbegin(), keymap.cend(), [](const std::pair<int, InputKeyData>& o)->bool { return o.second.bWasPressedThisFrame; });
        if (key != keymap.cend())
        {
            waiting = false;
            active_key = nullptr;

            if (key->first == VK_ESCAPE) return false;
            else
            {
                *vk_key = key->first;
                return true;
            }
        }
    }

    return false;
}

void CGui::DrawTimerSection(const char* window_name, const char* time_value, float height_ratio, const ImVec2& available_size, const ImVec4* colors, bool bMain)
{
    float font_scale_factor = 1.26f, xCorrection = 0.005f, yCorrection = 0.06f;
    if (bMain)
    {
        font_scale_factor = 1.4f;
        xCorrection = 0.01f;
        yCorrection = 0.01f;
    }

    float section_height = available_size.y * height_ratio;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::BeginChild(window_name, ImVec2(available_size.x, section_height), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground);

    ImVec2 child_size = ImGui::GetWindowSize();
    float timer_font_size = child_size.y * font_scale_factor;

    ImFont* large_font = ImGui::GetIO().Fonts->Fonts[1];

    ImGui::PushFont(large_font);

    float large_font_base_size = ImGui::GetFontSize();

    float font_scale = timer_font_size / large_font_base_size;
    ImGui::SetWindowFontScale(font_scale);

    ImVec2 text_size_scaled = ImGui::CalcTextSize(time_value);

    float text_pos_x = (child_size.x - text_size_scaled.x) * 0.5f - text_size_scaled.x * xCorrection;
    float centered_y = (child_size.y - text_size_scaled.y) * 0.5f;
    float visual_correction = text_size_scaled.y * yCorrection;
    float text_pos_y = centered_y - visual_correction;

    ImVec2 text_pos = ImGui::GetCursorScreenPos();
    text_pos.x += text_pos_x;
    text_pos.y += text_pos_y;

    ImDrawList* draw = ImGui::GetWindowDrawList();

    float size = (large_font_base_size * font_scale);

    const ImGuiStyle& Style = ImGui::GetStyle();

    if (bMain && this->bAutoSmudgeTimer)
    {
        const ImVec4 glow_color1(CConfig::Get().imvGlowColor1.x, CConfig::Get().imvGlowColor1.y, CConfig::Get().imvGlowColor1.z, CConfig::Get().imvGlowColor1.w * Style.Alpha);
        const ImVec4 glow_color2(CConfig::Get().imvGlowColor2.x, CConfig::Get().imvGlowColor2.y, CConfig::Get().imvGlowColor2.z, CConfig::Get().imvGlowColor2.w * Style.Alpha);
        this->DrawSoftGlow(draw, large_font, size, text_pos, time_value, glow_color1, 0.2f, (CConfig::Get().flSize / 14.f));
        this->DrawSoftGlow(draw, large_font, size, text_pos, time_value, glow_color2, 0.1f, (CConfig::Get().flSize / 8.f));  
    }

    if (Style.Alpha == 1.f && colors[0].w == 1.f && colors[1].w == 1.f)
        this->DrawTextOutline(draw, large_font, size, text_pos, IM_COL32(0, 0, 0, 180), time_value);

    int vtx_start = draw->VtxBuffer.Size;
    draw->AddText(text_pos, IM_COL32_WHITE, time_value);
    int vtx_end = draw->VtxBuffer.Size;

    float min_y = FLT_MAX;
    float max_y = -FLT_MAX;

    for (int i = vtx_start; i < vtx_end; i++)
    {
        min_y = min(min_y, draw->VtxBuffer[i].pos.y);
        max_y = max(max_y, draw->VtxBuffer[i].pos.y);
    }

    float height = max(1.0f, max_y - min_y);

    for (int i = vtx_start; i < vtx_end; i++)
    {
        float t = (draw->VtxBuffer[i].pos.y - min_y) / height;
        ImVec4 col = CTools::Get().Lerp(colors[0], colors[1], t);
        draw->VtxBuffer[i].col = ImGui::GetColorU32(col);
    }

    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopFont();

    ImGui::EndChild();
    ImGui::PopStyleVar();
}

void CGui::DrawTextOutline(ImDrawList* draw, ImFont* font, float size, const ImVec2& pos, ImU32 color, const char* text, float thickness)
{
    static const ImVec2 offsets[] =
    {
        { -1,  0 }, { 1,  0 },
        {  0, -1 }, { 0,  1 }
    };

    for (const ImVec2& o : offsets)
        draw->AddText(font, size, ImVec2(pos.x + o.x * thickness, pos.y + o.y * thickness), color, text);
}

void CGui::DrawSoftGlow(ImDrawList* draw, ImFont* font, float size, const ImVec2& pos, const char* text, ImVec4 color, float flAlpha, float radius)
{
    static const int rings = 3;
    static const int steps = 16;

    for (int r = 1; r <= rings; r++)
    {
        float t = (float)r / rings;
        float alpha = (1.0f - t) * flAlpha;
        color.w *= alpha;

        for (int i = 0; i < steps; i++)
        {
            float angle = (2.0f * IM_PI / steps) * i;
            ImVec2 offset(cosf(angle) * radius * t, sinf(angle) * radius * t);

            draw->AddText(font, size, ImVec2(pos.x + offset.x, pos.y + offset.y), ImGui::GetColorU32(color), text);
        }
    }
}

void CGui::UpdateClickThroughState()
{
    if (this->hwnd == nullptr)
        return;

    const ImGuiIO& io = ImGui::GetIO();

    LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);

    if (io.WantCaptureMouse)
    {
        if (exStyle & WS_EX_TRANSPARENT)
            SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle & ~WS_EX_TRANSPARENT);
    }
    else
    {
        if (!(exStyle & WS_EX_TRANSPARENT))
            SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle | WS_EX_TRANSPARENT);
    }
}

void CGui::ApplyStyle(ImGuiStyle& style)
{
    ImVec4* colors = style.Colors;
    const ImVec4 border_col = ImVec4(0.7f, 0.7f, 0.7f, 0.55f);
    const ImVec4 separator_col = ImVec4(0.7f, 0.7f, 0.7f, 0.45f);
    const ImVec4 Lime = ImVec4(0.55f, 0.95f, 0.25f, 1.00f);
    const ImVec4 LimeHover = ImVec4(0.65f, 1.00f, 0.35f, 1.00f);
    const ImVec4 LimeActive = ImVec4(0.45f, 0.85f, 0.20f, 1.00f);

    colors[ImGuiCol_Border] = border_col;
    colors[ImGuiCol_Separator] = separator_col;
    colors[ImGuiCol_SeparatorHovered] = border_col;
    colors[ImGuiCol_SeparatorActive] = border_col;

    colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);

    colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);

    colors[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0);

    colors[ImGuiCol_FrameBg] = ImVec4(0.18f, 0.25f, 0.15f, 0.54f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(Lime.x, Lime.y, Lime.z, 0.40f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(Lime.x, Lime.y, Lime.z, 0.67f);

    colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.18f, 0.30f, 0.12f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0, 0, 0, 0.51f);

    colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);

    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.30f, 0.45f, 0.25f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = LimeHover;
    colors[ImGuiCol_ScrollbarGrabActive] = LimeActive;

    colors[ImGuiCol_CheckMark] = Lime;
    colors[ImGuiCol_SliderGrab] = Lime;
    colors[ImGuiCol_SliderGrabActive] = LimeActive;

    colors[ImGuiCol_Button] = ImVec4(Lime.x, Lime.y, Lime.z, 0.40f);
    colors[ImGuiCol_ButtonHovered] = LimeHover;
    colors[ImGuiCol_ButtonActive] = LimeActive;

    colors[ImGuiCol_Header] = ImVec4(Lime.x, Lime.y, Lime.z, 0.30f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(Lime.x, Lime.y, Lime.z, 0.80f);
    colors[ImGuiCol_HeaderActive] = Lime;

    colors[ImGuiCol_ResizeGrip] = ImVec4(Lime.x, Lime.y, Lime.z, 0.20f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(Lime.x, Lime.y, Lime.z, 0.67f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(Lime.x, Lime.y, Lime.z, 0.95f);

    colors[ImGuiCol_Tab] = CTools::Get().Lerp(colors[ImGuiCol_Header], colors[ImGuiCol_TitleBgActive], 0.80f);
    colors[ImGuiCol_TabHovered] = colors[ImGuiCol_HeaderHovered];
    colors[ImGuiCol_TabSelected] = CTools::Get().Lerp(colors[ImGuiCol_HeaderActive], colors[ImGuiCol_TitleBgActive], 0.60f);
    colors[ImGuiCol_TabSelectedOverline] = Lime;
    colors[ImGuiCol_TabDimmed] = CTools::Get().Lerp(colors[ImGuiCol_Tab], colors[ImGuiCol_TitleBg], 0.80f);
    colors[ImGuiCol_TabDimmedSelected] = CTools::Get().Lerp(colors[ImGuiCol_TabSelected], colors[ImGuiCol_TitleBg], 0.40f);

    colors[ImGuiCol_TextSelectedBg] = ImVec4(Lime.x, Lime.y, Lime.z, 0.35f);
    colors[ImGuiCol_DragDropTarget] = Lime;
    colors[ImGuiCol_NavCursor] = Lime;

    style.FrameRounding = 12.f;
    style.FrameBorderSize = 1.f;
    style.WindowPadding = ImVec2(7.f, 7.f);
    style.FramePadding = ImVec2(6.f, 2.f);
    style.ItemSpacing = ImVec2(6.f, 3.f);
    style.GrabMinSize = 16.f;
    style.WindowRounding = 5.f;
    style.PopupRounding = 4.f;
    style.GrabRounding = 9.f;
    style.ScrollbarSize = 12.f;
    style.TabBarBorderSize = 1.f;
    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    style.WindowMenuButtonPosition = ImGuiDir::ImGuiDir_None;
    style.SeparatorTextBorderSize = 1.f;
}

bool CGui::Init(HINSTANCE hInstance)
{
    this->wc = { sizeof(this->wc), CS_CLASSDC, this->WndProc, 0L, 0L, hInstance, nullptr, nullptr, nullptr, nullptr, L"PT", nullptr };
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    ::RegisterClassExW(&this->wc);

    ImGui_ImplWin32_EnableDpiAwareness();
    float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));

    RECT desktop;
    const HWND hDesktop = GetDesktopWindow();
    GetWindowRect(hDesktop, &desktop);
    int width = desktop.right;
    int height = desktop.bottom;

    this->hwnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT, this->wc.lpszClassName, L"PhasmoTimer", WS_POPUP, 0, 0, width, height, NULL, NULL, this->wc.hInstance, NULL);

    if (!CreateDeviceD3D(width, height))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(this->wc.lpszClassName, this->wc.hInstance);
        return false;
    }

    DCompositionCreateDevice(nullptr, IID_PPV_ARGS(&g_dcompDevice));
    g_dcompDevice->CreateTargetForHwnd(this->hwnd, TRUE, &g_dcompTarget);
    g_dcompDevice->CreateVisual(&g_dcompVisual);
    g_dcompVisual->SetContent(g_pSwapChain);
    g_dcompTarget->SetRoot(g_dcompVisual);
    g_dcompDevice->Commit();

    ::ShowWindow(this->hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(this->hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.IniFilename = nullptr;

    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);
    style.FontScaleDpi = main_scale;
    this->ApplyStyle(style);

    ImGui_ImplWin32_Init(this->hwnd);
    ImGui_ImplDX11_Init(this->g_pd3dDevice, this->g_pd3dDeviceContext);

    io.Fonts->AddFontFromMemoryCompressedTTF(Font::FONT_COMICZ_BIN, sizeof(Font::FONT_COMICZ_BIN), 16.f);
    io.Fonts->AddFontFromMemoryCompressedTTF(Font::FONT_COMICZ_BIN, sizeof(Font::FONT_COMICZ_BIN), 96.f);
    io.Fonts->Build();

    if (!this->LoadTextureFromMemory(Logo::LOGO_BIN, sizeof(Logo::LOGO_BIN), &this->logo_texture, &this->logo_width, &this->logo_height))
        MessageBox(nullptr, L"Failed to load logo texture", L"Error", MB_OK | MB_ICONERROR);

    CGameForegroundTracker::Get().SetOwnHwnd(this->hwnd);
    CGameForegroundTracker::Get().SetProcessName(CConfig::Get().strGameProcessName);

    return true;
}

void CGui::Cleanup()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    this->CleanupDeviceD3D();
    ::DestroyWindow(this->hwnd);
    ::UnregisterClassW(this->wc.lpszClassName, this->wc.hInstance);
}

void CGui::StartFade(bool transparent)
{
    this->fade.from = ImGui::GetStyle().Alpha;
    this->fade.to = transparent ? CConfig::Get().flInactiveAlpha : 1.0f;
    this->fade.elapsed = 0.0f;
    this->fade.active = true;
}

void CGui::UpdateFade()
{
    if (!this->fade.active)
        return;

    this->fade.elapsed += ImGui::GetIO().DeltaTime;
    auto& Style = ImGui::GetStyle();

    float t = this->fade.elapsed / this->fade.duration;
    t = std::clamp(t, 0.0f, 1.0f);

    Style.Alpha = std::lerp(this->fade.from, this->fade.to, t);

    if (t >= 1.0f)
    {
        this->fade.active = false;
        Style.Alpha = this->fade.to;
    }
}

void CGui::ProcessFade()
{
    static bool lastActive = true;
    bool bActive = (this->bGameWindowActive || this->bSelfWindowActive);

    if (lastActive != bActive)
    {
        this->StartFade(!bActive);
        lastActive = bActive;
    }

    this->UpdateFade();
}

bool CGui::StartNewFrame()
{
    MSG msg;
    while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
    {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
        if (msg.message == WM_QUIT || msg.message == WM_CLOSE)
        {
            this->bWantExit = true;
            return false;
        }
    }

    this->bGameWindowActive = CConfig::Get().bCheckActiveWindow ? CGameForegroundTracker::Get().IsGameActive(this->bSelfWindowActive) : true;
    this->ProcessFade();
    CInput::Get().OnFrameStart(this->bGameWindowActive || this->bSelfWindowActive);

    // Handle window being minimized or screen locked
    if (g_SwapChainOccluded && g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        return false;
    }
    g_SwapChainOccluded = false;

    // Handle window resize (we don't resize directly in the WM_SIZE handler)
    if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
    {
        this->CleanupRenderTarget();
        g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
        g_ResizeWidth = g_ResizeHeight = 0;
        this->CreateRenderTarget();
    }

    this->now_time = std::chrono::steady_clock::now();
    this->UpdateTimers();

    // Start the Dear ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    return true;
}

void CGui::EndNewFrame()
{
    static const float clear_color_with_alpha[4] = { 0.f, 0.f, 0.f, 0.f };

    this->UpdateClickThroughState();
    CInput::Get().OnFrameEnd();

    ImGui::Render();
    g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
    g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    HRESULT hr = g_pSwapChain->Present(1, 0);   // Present with vsync
    //HRESULT hr = g_pSwapChain->Present(0, 0); // Present without vsync
    g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
    g_dcompDevice->Commit();
}

bool CGui::CreateDeviceD3D(int width, int height)
{
    DXGI_SWAP_CHAIN_DESC1 sc_desc = {};
    sc_desc.Width = width;
    sc_desc.Height = height;
    sc_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    sc_desc.Stereo = FALSE;
    sc_desc.SampleDesc.Count = 1;
    sc_desc.SampleDesc.Quality = 0;
    sc_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sc_desc.BufferCount = 2;
    sc_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    sc_desc.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
    sc_desc.Scaling = DXGI_SCALING_STRETCH;

    D3D_FEATURE_LEVEL featureLevel;
    if (FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext)))
        return false;

    IDXGIFactory2* dxgiFactory = nullptr;
    IDXGIDevice* dxgiDevice = nullptr;
    g_pd3dDevice->QueryInterface(IID_PPV_ARGS(&dxgiDevice));
    IDXGIAdapter* adapter = nullptr;
    dxgiDevice->GetAdapter(&adapter);
    adapter->GetParent(IID_PPV_ARGS(&dxgiFactory));

    dxgiFactory->CreateSwapChainForComposition(g_pd3dDevice, &sc_desc, nullptr, (IDXGISwapChain1**)&g_pSwapChain);

    bool ret = true;
    if (!CreateRenderTarget())
        ret = false;

    dxgiDevice->Release();
    adapter->Release();
    dxgiFactory->Release();
    return ret;
}

bool CGui::CreateRenderTarget()
{
    if (!this->g_pSwapChain || !this->g_pd3dDevice)
        return false;

    ID3D11Texture2D* pBackBuffer = nullptr;
    HRESULT hr = this->g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    if (FAILED(hr) || !pBackBuffer)
        return false;

    hr = this->g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &this->g_mainRenderTargetView);
    if (FAILED(hr))
    {
        this->g_mainRenderTargetView = nullptr;
        return false;
    }

    pBackBuffer->Release();
    return true;
}

void CGui::CleanupRenderTarget()
{
    if (this->g_mainRenderTargetView) { this->g_mainRenderTargetView->Release(); this->g_mainRenderTargetView = nullptr; }
}

void CGui::CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { this->g_pSwapChain->Release(); this->g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { this->g_pd3dDeviceContext->Release(); this->g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { this->g_pd3dDevice->Release(); this->g_pd3dDevice = nullptr; }
}

bool CGui::LoadTextureFromMemory(const void* data, size_t data_size, ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height)
{
    int image_width = 0;
    int image_height = 0;

    unsigned char* image_data = stbi_load_from_memory((const unsigned char*)data, (int)data_size, &image_width, &image_height, NULL, 4);
    if (image_data == NULL)
        return false;

    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = image_width;
    desc.Height = image_height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;

    ID3D11Texture2D* pTexture = NULL;
    D3D11_SUBRESOURCE_DATA subResource;
    subResource.pSysMem = image_data;
    subResource.SysMemPitch = desc.Width * 4;
    subResource.SysMemSlicePitch = 0;

    HRESULT hr = this->g_pd3dDevice->CreateTexture2D(&desc, &subResource, &pTexture);
    if (FAILED(hr))
    {
        stbi_image_free(image_data);
        return false;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = desc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = desc.MipLevels;

    hr = this->g_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, out_srv);
    pTexture->Release();

    if (FAILED(hr))
    {
        stbi_image_free(image_data);
        return false;
    }

    *out_width = image_width;
    *out_height = image_height;
    stbi_image_free(image_data);

    return true;
}

bool CGui::LoadTextureFromFile(const char* filename, ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height)
{
    FILE* f = nullptr;
    errno_t err = fopen_s(&f, filename, "rb");
    if (err != 0 || !f)
        return false;

    fseek(f, 0, SEEK_END);
    size_t file_size = (size_t)ftell(f);
    fseek(f, 0, SEEK_SET);
    void* file_data = malloc(file_size);
    if (file_data == nullptr)
    {
        fclose(f);
        return false;
    }
    fread(file_data, 1, file_size, f);
    fclose(f);

    bool ret = LoadTextureFromMemory(file_data, file_size, out_srv, out_width, out_height);

    free(file_data);
    return ret;
}

void CGui::UpdateTimers()
{
    if (this->bGameWindowActive)
    {
        const auto& SmudgeTimerKey = CInput::Get().GetKeyData(CConfig::Get().vkSmudgeTimerBind);
        const auto& SmudgeTimerSwitchKey = CInput::Get().GetKeyData(CConfig::Get().vkSwitchSmudgeTimerModeBind);
        const auto& HuntTimerKey = CInput::Get().GetKeyData(CConfig::Get().vkHuntTimerBind);
        const auto& TouchKey = CInput::Get().GetKeyData(CConfig::Get().vkTouchBind);
        const auto& UseKey = CInput::Get().GetKeyData(CConfig::Get().vkUseBind);
        const auto& FullResetKey = CInput::Get().GetKeyData(CConfig::Get().vkFullResetBind);
        const auto& ResetKey = CInput::Get().GetKeyData(CConfig::Get().vkResetBind);

        if (FullResetKey.bWasPressedThisFrame)
        {
            this->SmudgeTimerManual.Reset();
            this->SmudgeTimerAuto.Reset();
            this->HuntTimer.Reset();
            this->ObamboTimer.Reset();
        }
        else if (ResetKey.bIsDown)
        {
            if (SmudgeTimerKey.bWasPressedThisFrame || SmudgeTimerKey.bIsDown)
            {
                this->SmudgeTimerManual.Reset();
                this->SmudgeTimerAuto.Reset();
            }

            if (HuntTimerKey.bWasPressedThisFrame || HuntTimerKey.bIsDown)
                this->HuntTimer.Reset();
        }
        else
        {
            if (SmudgeTimerKey.bWasPressedThisFrame)
            {
                this->SmudgeTimerManual.Set(SmudgeTimerKey.tLastPressTime);
                this->SmudgeTimerAuto.Set(UseKey.tUseRegTime);
            }

            if (SmudgeTimerSwitchKey.bWasPressedThisFrame)
                this->bAutoSmudgeTimer = !this->bAutoSmudgeTimer;

            if (HuntTimerKey.bWasPressedThisFrame)
                this->HuntTimer.Set(HuntTimerKey.tLastPressTime);

            if (TouchKey.bWasPressedThisFrame)
                this->ObamboTimer.Set(TouchKey.tLastPressTime);
        }
    }

    this->SmudgeTimerManual.Update(this->now_time, CConfig::Get().iStartSmudgeTimerAt);
    this->SmudgeTimerAuto.Update(this->now_time, 0i64);
    this->HuntTimer.Update(this->now_time, CConfig::Get().iStartHuntTimerAt);
    this->ObamboTimer.Update(this->now_time);
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI CGui::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        CGui::Get().g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        CGui::Get().g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_CLOSE:
        CGui::Get().bWantExit = true;
        return 0;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}