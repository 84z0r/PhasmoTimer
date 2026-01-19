#include <cmath>
#include <algorithm>
#include <numbers>
#include "gui.h"
#include "config.h"
#include "imgui_wrappers.h"
#include "updater.h"
#include "foregroundtracker.h"
#include "version.h"
#include "tools.h"
#include "render.h"
#include "fonts.h"

constexpr const ImGuiWindowFlags timer_window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse;
constexpr const float top_ratio = 0.675f;
constexpr const float bottom_ratio = 1.f - top_ratio;

void CGui::Init()
{
	this->fakeDrawList = new ImDrawList(ImGui::GetDrawListSharedData());
}

void CGui::Cleanup()
{
    delete this->fakeDrawList;
}

void CGui::OnFrameStart()
{
    this->fakeDrawList->_ResetForNewFrame();
    this->UpdateTimers();
    this->ProcessFade();
}

//void PerformanceWindow()
//{
//    ImGui::Begin("Performance", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse);
//    auto& io = ImGui::GetIO();
//    ImGui::Text("FPS: %.0f", io.Framerate);
//    ImGui::Text("Frame Time: %.2f ms", 1000.0f / io.Framerate);
//    ImGui::End();
//}

void CGui::OnRender()
{
    //PerformanceWindow();
    this->UpdaterWindow();
    this->SettingsWindow();
    this->TimerWindows();
    this->AboutWindow();
}

void CGui::TimerWindows()
{
    if (!CConfig::Get().bEnableSplitMode)
    {
        this->MainTimerWindow();
    }
    else
    {
        if (this->bAutoSmudgeTimer)
            this->DrawTimerWindow("SmudgeTimerWindow", this->SmudgeTimerAuto.GetString(), this->SmudgeTimerAuto.GetColors(), CConfig::Get().flSmudgeTimerSize, CConfig::Get().imvSmudgeTimerWindowPos, this->bAutoSmudgeTimer);
        else
            this->DrawTimerWindow("SmudgeTimerWindow", this->SmudgeTimerManual.GetString(), this->SmudgeTimerManual.GetColors(), CConfig::Get().flSmudgeTimerSize, CConfig::Get().imvSmudgeTimerWindowPos, this->bAutoSmudgeTimer);

        if (CConfig::Get().bEnableSplitObambo)
            this->DrawTimerWindow("ObamboTimerWindow", this->ObamboTimer.GetString(), this->ObamboTimer.GetColors(), CConfig::Get().flObamboTimerSize, CConfig::Get().imvObamboTimerWindowPos, false);

        if (CConfig::Get().bEnableSplitHunt)
            this->DrawTimerWindow("HuntTimerWindow", this->HuntTimer.GetString(), this->HuntTimer.GetColors(), CConfig::Get().flHuntTimerSize, CConfig::Get().imvHuntTimerWindowPos, false);

        if (CConfig::Get().bEnableSplitCandle)
            this->DrawTimerWindow("CandleTimerWindow", this->CandleTimer.GetString(), this->CandleTimer.GetColors(), CConfig::Get().flCandleTimerSize, CConfig::Get().imvCandleTimerWindowPos, false);
    }
}

void CGui::MainTimerWindow()
{
    const float flWidthScale = CFonts::Get().GetWindowWidthFactor() * 0.7f;
    ImVec2 windowSize((CConfig::Get().flSize * flWidthScale), CConfig::Get().flSize);

    ImGui::SetNextWindowSize(windowSize);
    ImGui::SetNextWindowPos(CConfig::Get().imvTimerWindowPos, ImGuiCond_Once);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, CConfig::Get().flRounding);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, CConfig::Get().imvBackgroundColor);
    ImGui::PushStyleColor(ImGuiCol_Border, CConfig::Get().imvBordersColor);
    ImGui::PushStyleColor(ImGuiCol_Separator, CConfig::Get().imvBordersColor);

    bool bOpened = ImGui::BeginSnap("MainTimerWindow", nullptr, timer_window_flags);
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);

    if (bOpened)
    {
        if (CConfig::Get().IsConfigUpdated())
            ImGui::SetWindowPos(CConfig::Get().imvTimerWindowPos);
        else
            CConfig::Get().imvTimerWindowPos = ImGui::GetWindowPos();

        if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
            ImGui::OpenPopup("TimerContextMenu");

        if (ImGui::BeginPopup("TimerContextMenu"))
        {
            if (ImGui::MenuItem("Settings")) this->bShowSettings = true;
            if (ImGui::MenuItem("About")) this->bShowAbout = true;
            if (ImGui::MenuItem("Exit")) CRender::Get().bWantExit = true;
            ImGui::EndPopup();
        }

        ImGui::SetCursorPos(ImVec2(0.0f, 0.0f));
        if (this->bAutoSmudgeTimer)
            this->DrawTimerSection("Main", this->SmudgeTimerAuto.GetString(), top_ratio, windowSize, this->SmudgeTimerAuto.GetColors(), true);
        else
            this->DrawTimerSection("Main", this->SmudgeTimerManual.GetString(), top_ratio, windowSize, this->SmudgeTimerManual.GetColors(), true);

        const float bottom_y_start = windowSize.y * top_ratio;
        ImVec2 sizeAvailableHalf(windowSize.x * 0.5f, windowSize.y);

        ImGui::SetCursorPos(ImVec2(0.0f, bottom_y_start));
        this->DrawTimerSection("Obambo", this->ObamboTimer.GetString(), bottom_ratio, sizeAvailableHalf, this->ObamboTimer.GetColors(), false);

        ImGui::SetCursorPos(ImVec2(windowSize.x * 0.5f, bottom_y_start));
        this->DrawTimerSection("Hunt", this->HuntTimer.GetString(), bottom_ratio, sizeAvailableHalf, this->HuntTimer.GetColors(), false);

        float separator_y = std::floorf(CConfig::Get().imvTimerWindowPos.y + windowSize.y * top_ratio);
        ImGui::GetWindowDrawList()->AddLine(ImVec2(CConfig::Get().imvTimerWindowPos.x, separator_y), ImVec2(CConfig::Get().imvTimerWindowPos.x + windowSize.x, separator_y), ImGui::GetColorU32(ImGuiCol_Separator), 1.0f);

        float separator_x = std::floorf(CConfig::Get().imvTimerWindowPos.x + windowSize.x * 0.5f);
        ImGui::GetWindowDrawList()->AddLine(ImVec2(separator_x, separator_y), ImVec2(separator_x, CConfig::Get().imvTimerWindowPos.y + windowSize.y), ImGui::GetColorU32(ImGuiCol_Separator), 1.0f);
    }

    ImGui::End();
}

void CGui::SettingsWindow()
{
    if (!this->bShowSettings)
        return;

    const auto& Style = ImGui::GetStyle();
    const ImVec2 window_size(640 * Style.FontScaleDpi, 360 * Style.FontScaleDpi);
    ImGui::SetNextWindowSize(window_size, ImGuiCond_Always);
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 center((io.DisplaySize.x - window_size.x) * 0.5f, (io.DisplaySize.y - window_size.y) * 0.5f);
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing);

    if (ImGui::Begin("Settings", &this->bShowSettings, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
    {
        ImVec2 btn_size(90 * Style.FontScaleDpi, 25 * Style.FontScaleDpi);
        ImVec2 content_max = ImGui::GetWindowContentRegionMax();
        ImVec2 content_min = ImGui::GetWindowContentRegionMin();
        ImVec2 window_pos = ImGui::GetWindowPos();

        if (ImGui::BeginTabBar("SettingsTabs", ImGuiTabBarFlags_None))
        {
            if (ImGui::BeginTabItem("Main"))
            {
                if (ImGui::BeginTable("MainTable", 2, ImGuiTableFlags_None))
                {
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);

                    if (!CConfig::Get().bEnableSplitMode)
                    {
                        ImGui::SeparatorText("Timer Window");
                        ImGui::PushItemWidth(215.f * Style.FontScaleDpi);
                        ImGui::SliderFloat("Size", &CConfig::Get().flSize, 30.f, 500.f);
                        ImGui::SliderFloat("Rounding", &CConfig::Get().flRounding, 0.f, 20.f);
                        ImGui::PopItemWidth();
                    }

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

                        ImGui::SliderFloat("Inactive Alpha", &CConfig::Get().flInactiveAlpha, 0.f, 1.f, "%.3f", ImGuiSliderFlags_ClampOnInput);
                        ImGui::PopItemWidth();
                    }

                    ImGui::TableSetColumnIndex(1);

                    ImGui::SeparatorText("Keybinds");
                    this->KeyBindButton("Smudge Timer", &CConfig::Get().vkSmudgeTimerBind);
                    this->KeyBindButton("Smudge Timer Mode", &CConfig::Get().vkSwitchSmudgeTimerModeBind);
                    this->KeyBindButton("Hunt Timer", &CConfig::Get().vkHuntTimerBind);
                    this->KeyBindButton("Candle Timer", &CConfig::Get().vkCandleTimerBind);
                    ImGui::SetItemTooltip("Split Mode only!");
                    this->KeyBindButton("Full Reset", &CConfig::Get().vkFullResetBind);
                    this->KeyBindButton("Reset", &CConfig::Get().vkResetBind);
                    ImGui::SetItemTooltip("Hold \"Reset\" bind and any timer\nbind to reset the timer");
                    this->KeyBindButton("Touch", &CConfig::Get().vkTouchBind);
                    this->KeyBindButton("Use", &CConfig::Get().vkUseBind);
                    ImGui::SetItemTooltip("WARNING: The \"Use\" bind in game\nshould be the save as the \"Use Hold\"!");

                    ImGui::SeparatorText("Update");
                    ImGui::Checkbox("Check For Updates", &CConfig::Get().bCheckUpdates);

                    ImGui::EndTable();
                }

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Split Mode"))
            {
                if (ImGui::BeginTable("SplitModeTable", 2, ImGuiTableFlags_None))
                {
                    ImGui::TableSetupColumn("Settings", ImGuiTableColumnFlags_WidthFixed, 150.0f * Style.FontScaleDpi);
                    ImGui::TableSetupColumn("Sliders", ImGuiTableColumnFlags_WidthStretch);

                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    ImGui::SeparatorText("Split Mode");
                    ImGui::Checkbox("Enable Split Mode", &CConfig::Get().bEnableSplitMode);
                    if (CConfig::Get().bEnableSplitMode)
                    {
                        ImGui::Checkbox("Enable Obambo Timer", &CConfig::Get().bEnableSplitObambo);
                        ImGui::Checkbox("Enable Hunt Timer", &CConfig::Get().bEnableSplitHunt);
                        ImGui::Checkbox("Enable Candle Timer", &CConfig::Get().bEnableSplitCandle);
                    }

                    ImGui::TableSetColumnIndex(1);
                    if (CConfig::Get().bEnableSplitMode)
                    {
                        ImGui::SeparatorText("Timer Window Size");

                        ImGui::PushItemWidth(-FLT_MIN);

                        ImGui::SliderFloat("##SmudgeTimerSize", &CConfig::Get().flSmudgeTimerSize, 20.f, 300.f);
                        ImGui::SetItemTooltip("Ctrl+click to enter the exact value");

                        ImGui::SliderFloat("##ObamboTimerSize", &CConfig::Get().flObamboTimerSize, 20.f, 300.f);
                        ImGui::SetItemTooltip("Ctrl+click to enter the exact value");

                        ImGui::SliderFloat("##HuntTimerSize", &CConfig::Get().flHuntTimerSize, 20.f, 300.f);
                        ImGui::SetItemTooltip("Ctrl+click to enter the exact value");

                        ImGui::SliderFloat("##CandleTimerSize", &CConfig::Get().flCandleTimerSize, 20.f, 300.f);
                        ImGui::SetItemTooltip("Ctrl+click to enter the exact value");

                        ImGui::PopItemWidth();
                    }

                    ImGui::EndTable();
                }

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Font"))
            {
                if (ImGui::BeginTable("FontTable", 2, ImGuiTableFlags_None))
                {
                    ImGui::TableSetupColumn("SelectFonts", ImGuiTableColumnFlags_WidthFixed, 435.0f * Style.FontScaleDpi);
                    ImGui::TableSetupColumn("FontFoldersFiltes", ImGuiTableColumnFlags_WidthStretch);

                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    ImGui::SeparatorText("Timer Font##timer_font");

                    const auto& availableFonts = CFonts::Get().GetAvailableFonts();
                    size_t iVecSize = availableFonts.size();
                    size_t iFontIdx = CFonts::Get().GetCustomFontIndex();
                    ImVec2 avail_region = ImGui::GetContentRegionAvail();
                    
                    if (ImGui::BeginChild("##timer_font_list", ImVec2(0.0f, avail_region.y - btn_size.y - Style.WindowPadding.y - Style.ItemSpacing.y), ImGuiChildFlags_Borders, ImGuiWindowFlags_AlwaysVerticalScrollbar))
                    {
                        for (size_t i = 0ULL; i < iVecSize; ++i)
                        {
                            bool selected = (i == iFontIdx);
							std::u8string fontNameU8 = availableFonts[i].filename().u8string();

                            if (ImGui::Selectable(reinterpret_cast<const char*>(fontNameU8.c_str()), selected) && !selected)
                                CFonts::Get().SetFontToLoad(i);

                            if (selected)
                                ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndChild();

                    ImVec2 button_pos(window_pos.x + Style.WindowPadding.x, window_pos.y + content_max.y - btn_size.y - Style.ItemSpacing.y);
                    ImGui::SetCursorScreenPos(button_pos);
                    if (ImGui::Button("Refresh", btn_size)) CFonts::Get().ScanFonts();
                    ImGui::SetItemTooltip("Re-Scan fonts folder");

                    ImGui::TableSetColumnIndex(1);
					ImGui::SeparatorText("Filters##font_folder_filters");
                    if (ImGui::Checkbox("System fonts", &CConfig::Get().bScanSystemFonts))
						CFonts::Get().ScanFonts();

					if (ImGui::Checkbox("User fonts", &CConfig::Get().bScanUserFonts))
                        CFonts::Get().ScanFonts();

                    if (ImGui::Checkbox("Application fonts", &CConfig::Get().bScanAppFonts))
                        CFonts::Get().ScanFonts();

                    ImGui::EndTable();
                }

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Colors"))
            {
                if (ImGui::BeginTable("ColorsTable", 2, ImGuiTableFlags_None))
                {
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
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

                    ImGui::TableSetColumnIndex(1);

                    ImGui::SeparatorText("Hunt Timer##hunt_timer_colors");

                    ImGui::ColorEdit4("Hunt Timer Top", reinterpret_cast<float*>(&CConfig::Get().imvHuntTimerColor1), ImGuiColorEditFlags_NoInputs);
                    ImGui::ColorEdit4("Hunt Timer Bottom", reinterpret_cast<float*>(&CConfig::Get().imvHuntTimerColor2), ImGuiColorEditFlags_NoInputs);

                    ImGui::SeparatorText("Obambo Timer##obambo_timer_colors");

                    ImGui::ColorEdit4("Obambo Calm Top", reinterpret_cast<float*>(&CConfig::Get().imvObamboCalmColor1), ImGuiColorEditFlags_NoInputs);
                    ImGui::ColorEdit4("Obambo Calm Bottom", reinterpret_cast<float*>(&CConfig::Get().imvObamboCalmColor2), ImGuiColorEditFlags_NoInputs);
                    ImGui::ColorEdit4("Obambo Aggressive Top", reinterpret_cast<float*>(&CConfig::Get().imvObamboAggressiveColor1), ImGuiColorEditFlags_NoInputs);
                    ImGui::ColorEdit4("Obambo Aggressive Bottom", reinterpret_cast<float*>(&CConfig::Get().imvObamboAggressiveColor2), ImGuiColorEditFlags_NoInputs);

                    ImGui::SeparatorText("Candle Timer##candle_timer_colors");

                    ImGui::ColorEdit4("Candle Timer Top", reinterpret_cast<float*>(&CConfig::Get().imvCandleTimerColor1), ImGuiColorEditFlags_NoInputs);
                    ImGui::ColorEdit4("Candle Timer Bottom", reinterpret_cast<float*>(&CConfig::Get().imvCandleTimerColor2), ImGuiColorEditFlags_NoInputs);

                    ImGui::EndTable();
                }

                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImVec2 button_pos(window_pos.x + content_max.x - (btn_size.x * 2.f + Style.ItemSpacing.x), window_pos.y + content_max.y - btn_size.y - Style.ItemSpacing.y);
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
        ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);

        if (this->bShowAbout != bPrevShow)
            ImGui::OpenPopup("About##modal_about");

        if (ImGui::BeginPopupModal("About##modal_about", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
        {
            if (ImGui::BeginTable("AboutTable", 2, ImGuiTableFlags_None))
            {
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::PushStyleColor(ImGuiCol_Text, name_color);
                ImGui::Text("PhasmoTimer\n");
                ImGui::PopStyleColor();
                ImGui::Text("Version: %s\n\n", Version::APP_VERSION);
                ImGui::Text("Made by");
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Text, name_color);
                ImGui::Text("atlas2884");
                ImGui::PopStyleColor();

                ImGui::TableSetColumnIndex(1);
                float column_width = ImGui::GetColumnWidth();
                float image_width = 80.0f;
                float offset = (column_width - image_width) * 0.5f;
                if (offset > 0.0f) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
                ImGui::Image(reinterpret_cast<ImTextureID>(CRender::Get().logo_texture), ImVec2(image_width, 80.f));

                ImGui::EndTable();
            }

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
        ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);

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
        else sprintf_s(button_text, "[ None ]##%s", label);
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

void CGui::DrawTimerWindow(const char* window_name, const char* time_value, const ImVec4* colors, float flSize, ImVec2& imvPos, bool bGlow)
{
    ImVec2 windowSize((flSize * CFonts::Get().GetWindowWidthFactor()), flSize);

    ImGui::SetNextWindowSize(windowSize);
    ImGui::SetNextWindowPos(imvPos, ImGuiCond_Once);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, CConfig::Get().imvBackgroundColor);

    bool bOpened = ImGui::BeginSnap(window_name, nullptr, timer_window_flags);
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(3);

    if (bOpened)
    {
        if (CConfig::Get().IsConfigUpdated())
            ImGui::SetWindowPos(imvPos);
        else
            imvPos = ImGui::GetWindowPos();

        if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
            ImGui::OpenPopup("TimerContextMenu");

        if (ImGui::BeginPopup("TimerContextMenu"))
        {
            if (ImGui::MenuItem("Settings")) this->bShowSettings = true;
            if (ImGui::MenuItem("About")) this->bShowAbout = true;
            if (ImGui::MenuItem("Exit")) CRender::Get().bWantExit = true;
            ImGui::EndPopup();
        }

        float size = windowSize.y * CFonts::Get().GetFontScaleFactor();
        ImDrawList* draw = ImGui::GetWindowDrawList();
		const ImGuiIO& io = ImGui::GetIO();
		ImFont* font = io.Fonts->Fonts[1];
        const ImGuiStyle& Style = ImGui::GetStyle();
        const ImVec2 delta = this->CalculateTextCenterDelta(font, time_value, size, imvPos, windowSize);

        if (bGlow)
        {
            const ImVec4 glow_color1(CConfig::Get().imvGlowColor1.x, CConfig::Get().imvGlowColor1.y, CConfig::Get().imvGlowColor1.z, CConfig::Get().imvGlowColor1.w * Style.Alpha);
            const ImVec4 glow_color2(CConfig::Get().imvGlowColor2.x, CConfig::Get().imvGlowColor2.y, CConfig::Get().imvGlowColor2.z, CConfig::Get().imvGlowColor2.w * Style.Alpha);
            this->DrawSoftGlow(draw, font, size, delta, imvPos, time_value, glow_color1, 0.2f, (CConfig::Get().flSize / 12.f));
            this->DrawSoftGlow(draw, font, size, delta, imvPos, time_value, glow_color2, 0.1f, (CConfig::Get().flSize / 7.f));
        }

        if (Style.Alpha == 1.f && colors[0].w == 1.f && colors[1].w == 1.f)
            this->DrawTextOutline(draw, font, size, delta, imvPos, IM_COL32(0, 0, 0, 180), time_value);
        
        int vtx_start = draw->VtxBuffer.Size;
		draw->AddText(font, size, imvPos, IM_COL32_WHITE, time_value);
        int vtx_end = draw->VtxBuffer.Size;

        this->ApplyTextCenterDelta(vtx_start, vtx_end, delta, draw);
        this->TextColorGradient(vtx_start, vtx_end, colors, draw);       
    }

    ImGui::End();
}

void CGui::DrawTimerSection(const char* window_name, const char* time_value, float height_ratio, const ImVec2& available_size, const ImVec4* colors, bool bMain)
{
    float section_height = available_size.y * height_ratio;
    ImVec2 windowSize(available_size.x, section_height);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    bool bOpened = ImGui::BeginChild(window_name, windowSize, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground);
    ImGui::PopStyleVar();

    if (bOpened)
    {
        float size = windowSize.y * CFonts::Get().GetFontScaleFactor();
        ImDrawList* draw = ImGui::GetWindowDrawList();
        const ImGuiIO& io = ImGui::GetIO();
        ImFont* font = io.Fonts->Fonts[1];
        const ImGuiStyle& Style = ImGui::GetStyle();
        const ImVec2 imvPos = ImGui::GetWindowPos();
        const ImVec2 delta = this->CalculateTextCenterDelta(font, time_value, size, imvPos, windowSize);

        if (bMain && this->bAutoSmudgeTimer)
        {
            const ImVec4 glow_color1(CConfig::Get().imvGlowColor1.x, CConfig::Get().imvGlowColor1.y, CConfig::Get().imvGlowColor1.z, CConfig::Get().imvGlowColor1.w * Style.Alpha);
            const ImVec4 glow_color2(CConfig::Get().imvGlowColor2.x, CConfig::Get().imvGlowColor2.y, CConfig::Get().imvGlowColor2.z, CConfig::Get().imvGlowColor2.w * Style.Alpha);
            this->DrawSoftGlow(draw, font, size, delta, imvPos, time_value, glow_color1, 0.2f, (CConfig::Get().flSize / 12.f));
            this->DrawSoftGlow(draw, font, size, delta, imvPos, time_value, glow_color2, 0.1f, (CConfig::Get().flSize / 7.f));
        }

        if (Style.Alpha == 1.f && colors[0].w == 1.f && colors[1].w == 1.f)
            this->DrawTextOutline(draw, font, size, delta, imvPos, IM_COL32(0, 0, 0, 180), time_value);

        int vtx_start = draw->VtxBuffer.Size;
        draw->AddText(font, size, imvPos, IM_COL32_WHITE, time_value);
        int vtx_end = draw->VtxBuffer.Size;

        this->ApplyTextCenterDelta(vtx_start, vtx_end, delta, draw);
        this->TextColorGradient(vtx_start, vtx_end, colors, draw);
    }

    ImGui::EndChild(); 
}

void CGui::DrawTextOutline(ImDrawList* draw, ImFont* font, float size, const ImVec2& delta, const ImVec2& pos, ImU32 color, const char* text, float thickness)
{
    static const ImVec2 offsets[] =
    {
        { -1.f,  0.f }, { 1.f,  0.f },
        {  0.f, -1.f }, { 0.f,  1.f }
    };

    for (const ImVec2& o : offsets)
    {
        int vtx_start = draw->VtxBuffer.Size;
        draw->AddText(font, size, pos, color, text);
        int vtx_end = draw->VtxBuffer.Size;
        this->ApplyTextCenterDelta(vtx_start, vtx_end, ImVec2(delta.x + o.x * thickness, delta.y + o.y * thickness), draw);
    }
}

void CGui::DrawSoftGlow(ImDrawList* draw, ImFont* font, float size, const ImVec2& delta, const ImVec2& pos, const char* text, ImVec4 color, float flAlpha, float radius)
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
            float angle = (2.0f * std::numbers::pi_v<float> / steps) * i;
            ImVec2 offset(cosf(angle) * radius * t, sinf(angle) * radius * t);

            int vtx_start = draw->VtxBuffer.Size;
            draw->AddText(font, size, pos, ImGui::GetColorU32(color), text);
            int vtx_end = draw->VtxBuffer.Size;
			this->ApplyTextCenterDelta(vtx_start, vtx_end, ImVec2(delta.x + offset.x, delta.y + offset.y), draw);
        }
    }
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
    bool bActive = (CRender::Get().IsGameWindowActive() || CRender::Get().IsSelfWindowActive());

    if (lastActive != bActive)
    {
        this->StartFade(!bActive);
        lastActive = bActive;
    }

    this->UpdateFade();
}

ImVec2 CGui::CalculateTextCenterDelta(ImFont* font, const char* text, float size, const ImVec2& windowPos, const ImVec2& windowSize)
{
    std::string placeholder_text = CTools::Get().GeneratePlaceholderString(text, CFonts::Get().GetPlaceholderChar());

    this->fakeDrawList->PushClipRect(windowPos, ImVec2(windowPos.x + windowSize.x, windowPos.y + windowSize.y), false);
    int vtx_start = fakeDrawList->VtxBuffer.Size;
    this->fakeDrawList->AddText(font, size, windowPos, IM_COL32_WHITE, placeholder_text.c_str());
    int vtx_end = fakeDrawList->VtxBuffer.Size;
    this->fakeDrawList->PopClipRect();

    if (vtx_start >= vtx_end)
        return windowPos;

    ImVec2 bb_min(FLT_MAX, FLT_MAX);
    ImVec2 bb_max(-FLT_MAX, -FLT_MAX);

    for (int i = vtx_start; i < vtx_end; i++)
    {
        bb_min.x = std::min(bb_min.x, fakeDrawList->VtxBuffer[i].pos.x);
        bb_min.y = std::min(bb_min.y, fakeDrawList->VtxBuffer[i].pos.y);
        bb_max.x = std::max(bb_max.x, fakeDrawList->VtxBuffer[i].pos.x);
        bb_max.y = std::max(bb_max.y, fakeDrawList->VtxBuffer[i].pos.y);
    }

    ImVec2 text_center((bb_min.x + bb_max.x) * 0.5f, (bb_min.y + bb_max.y) * 0.5f);
    ImVec2 window_center(windowPos.x + windowSize.x * 0.5f, windowPos.y + windowSize.y * 0.5f);

    return ImVec2(window_center.x - text_center.x, window_center.y - text_center.y);
}

void CGui::ApplyTextCenterDelta(int vtx_start, int vtx_end, const ImVec2& delta, ImDrawList* draw)
{
	ImVec2 rounded_delta(std::roundf(delta.x), std::roundf(delta.y));

    for (int i = vtx_start; i < vtx_end; i++)
    {
        draw->VtxBuffer[i].pos.x += rounded_delta.x;
        draw->VtxBuffer[i].pos.y += rounded_delta.y;
    }
}

void CGui::TextColorGradient(int vtx_start, int vtx_end, const ImVec4* colors, ImDrawList* draw)
{
    float min_y = FLT_MAX;
    float max_y = -FLT_MAX;

    for (int i = vtx_start; i < vtx_end; i++)
    {
        min_y = std::min(min_y, draw->VtxBuffer[i].pos.y);
        max_y = std::max(max_y, draw->VtxBuffer[i].pos.y);
    }

    float height = std::max(1.0f, max_y - min_y);

    for (int i = vtx_start; i < vtx_end; i++)
    {
        float t = (draw->VtxBuffer[i].pos.y - min_y) / height;
        ImVec4 col = CTools::Get().Lerp(colors[0], colors[1], t);
        draw->VtxBuffer[i].col = ImGui::GetColorU32(col);
    }
}

void CGui::UpdateTimers()
{
    if (CRender::Get().IsGameWindowActive())
    {
        const auto& SmudgeTimerKey = CInput::Get().GetKeyData(CConfig::Get().vkSmudgeTimerBind);
        const auto& SmudgeTimerSwitchKey = CInput::Get().GetKeyData(CConfig::Get().vkSwitchSmudgeTimerModeBind);
        const auto& HuntTimerKey = CInput::Get().GetKeyData(CConfig::Get().vkHuntTimerBind);
        const auto& CandleTimerKey = CInput::Get().GetKeyData(CConfig::Get().vkCandleTimerBind);
        const auto& TouchKey = CInput::Get().GetKeyData(CConfig::Get().vkTouchBind);
        const auto& UseKey = CInput::Get().GetKeyData(CConfig::Get().vkUseBind);
        const auto& FullResetKey = CInput::Get().GetKeyData(CConfig::Get().vkFullResetBind);
        const auto& ResetKey = CInput::Get().GetKeyData(CConfig::Get().vkResetBind);

        if (FullResetKey.bWasPressedThisFrame)
        {
            this->SmudgeTimerManual.Reset();
            this->SmudgeTimerAuto.Reset();
            this->ObamboTimer.Reset();
            this->HuntTimer.Reset();
            this->CandleTimer.Reset();
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

            if (CandleTimerKey.bWasPressedThisFrame || CandleTimerKey.bIsDown)
                this->CandleTimer.Reset();
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

            if (CandleTimerKey.bWasPressedThisFrame)
                this->CandleTimer.Set(UseKey.tUseRegTime);

            if (TouchKey.bWasPressedThisFrame)
                this->ObamboTimer.Set(TouchKey.tLastPressTime);
        }
    }

    const auto& now_time = CRender::Get().GetNowTime();

    this->SmudgeTimerManual.Update(now_time, CConfig::Get().iStartSmudgeTimerAt);
    this->SmudgeTimerAuto.Update(now_time, 0LL);
    this->ObamboTimer.Update(now_time);
    this->HuntTimer.Update(now_time, CConfig::Get().iStartHuntTimerAt);
    this->CandleTimer.Update(now_time);
}