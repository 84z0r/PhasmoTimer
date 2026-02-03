#pragma once
#include <imgui/imgui.h>
#include <string>
#include "config.h"

class CHUDBar
{
public:
    CHUDBar(const std::string& label, double value, double max_value, const float& rounding, const float& fill_rounding, const float& padding, const ImVec2& size, const ImVec4& bg, const ImVec4& bd, const ImVec4& fill_top, const ImVec4& fill_bottom, const ImVec4& fill_top_alt, const ImVec4& fill_bottom_alt, const ImVec4& flash, const bool& gloss) :
        strLabel(label), flValue(value), flMaxValue(max_value), Rounding(rounding), FillRounding(fill_rounding), Padding(padding), Size(size), BgColor(bg), BorderColor(bd), FillColorTop(fill_top), FillColorBottom(fill_bottom), FillColorTopAlt(fill_top_alt), FillColorBottomAlt(fill_bottom_alt), FlashColor(flash), bGloss(gloss) {}

    void Draw() const;

public:
    std::string strLabel;
    double flValue;
    double flMaxValue;

    const float& Rounding;
    const float& FillRounding;
    const float& Padding;

    const ImVec2& Size;

    const ImVec4& BgColor;
    const ImVec4& BorderColor;
    const ImVec4& FillColorTop;
    const ImVec4& FillColorBottom;
    const ImVec4& FillColorTopAlt;
    const ImVec4& FillColorBottomAlt;
    const ImVec4& FlashColor;

	const bool& bGloss;
    
    float FlashSpeed = 10.0f;

    bool bUseAltFill = false;
    bool bCriticalFlash = false;
    bool ShowText = false;

private:
    void ApplyVertexGradient(ImDrawList* draw_list, int vtx_start, const ImVec4& col_top, const ImVec4& col_bottom) const;
};