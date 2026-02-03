#include "hud_bar.h"
#include "tools.h"
#include <imgui/imgui_internal.h>

void CHUDBar::Draw() const
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext& g = *GImGui;
    const float global_alpha = g.Style.Alpha;
    const ImGuiID id = window->GetID(this->strLabel.c_str());

    ImVec2 pos = window->DC.CursorPos;
    ImRect bb(pos, ImVec2(pos.x + this->Size.x, pos.y + this->Size.y));
    ImGui::ItemSize(bb, g.Style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id))
        return;

    double fraction = CTools::Get().Saturate(this->flValue / this->flMaxValue);
    ImDrawList* draw_list = window->DrawList;

    ImVec4 bg = this->BgColor;
    bg.w *= global_alpha;
    draw_list->AddRectFilled(bb.Min, bb.Max, ImGui::ColorConvertFloat4ToU32(bg), this->Rounding);

    if (fraction > 0.0)
    {
        ImVec2 fill_min = ImVec2(bb.Min.x + this->Padding, bb.Min.y + this->Padding);
        ImVec2 fill_max_full = ImVec2(bb.Max.x - this->Padding, bb.Max.y - this->Padding);
        float current_width = static_cast<float>(static_cast<double>(fill_max_full.x - fill_min.x) * fraction);
        ImVec2 fill_max = ImVec2(fill_min.x + current_width, fill_max_full.y);

        ImDrawFlags rounding_flags = ImDrawFlags_RoundCornersLeft;
        if (fraction > 0.999)
            rounding_flags = ImDrawFlags_RoundCornersAll;

        int vtx_start = draw_list->VtxBuffer.Size;
        draw_list->AddRectFilled(fill_min, fill_max, IM_COL32_WHITE, this->FillRounding, rounding_flags);
        const ImVec4& top = this->bUseAltFill ? this->FillColorTopAlt : this->FillColorTop;
        const ImVec4& bottom = this->bUseAltFill ? this->FillColorBottomAlt : this->FillColorBottom;
        ApplyVertexGradient(draw_list, vtx_start, top, bottom);

        if (this->bGloss)
        {
            int gloss_vtx_start = draw_list->VtxBuffer.Size;
            ImVec2 gloss_max = ImVec2(fill_max.x, fill_min.y + (fill_max.y - fill_min.y) * 0.45f);
            draw_list->AddRectFilled(fill_min, gloss_max, IM_COL32_WHITE, this->FillRounding, rounding_flags & ImDrawFlags_RoundCornersTop);
            ApplyVertexGradient(draw_list, gloss_vtx_start, ImVec4(1, 1, 1, 0.25f), ImVec4(1, 1, 1, 0.0f));
        }
    }

    ImVec4 borderColor = this->BorderColor;
    float borderThickness = 1.0f;

    if (this->bCriticalFlash)
    {
        float pulse = (float)sin(ImGui::GetTime() * this->FlashSpeed) * 0.5f + 0.5f;
        borderColor = CTools::Get().Lerp(borderColor, this->FlashColor, pulse);
        borderThickness = 1.0f + pulse * 1.0f;
    }

    borderColor.w *= global_alpha;
    draw_list->AddRect(bb.Min, bb.Max, ImGui::ColorConvertFloat4ToU32(borderColor), this->Rounding, 0, borderThickness);

    if (this->ShowText)
    {
        char buf[64];
        ImFormatString(buf, IM_ARRAYSIZE(buf), "%.0f / %.0f", this->flValue, this->flMaxValue);
        ImVec2 text_size = ImGui::CalcTextSize(buf);
        ImVec2 text_pos = ImVec2(bb.Min.x + (bb.GetWidth() - text_size.x) * 0.5f, bb.Min.y + (bb.GetHeight() - text_size.y) * 0.5f);

        ImU32 shadow_col = IM_COL32(0, 0, 0, static_cast<int>(255.0f * global_alpha));
        ImU32 text_col = IM_COL32(255, 255, 255, static_cast<int>(255.0f * global_alpha));

        draw_list->AddText(ImVec2(text_pos.x + 1, text_pos.y + 1), shadow_col, buf);
        draw_list->AddText(text_pos, text_col, buf);
    }
}

void CHUDBar::ApplyVertexGradient(ImDrawList* draw_list, int vtx_start, const ImVec4& col_top, const ImVec4& col_bottom) const
{
    int vtx_end = draw_list->VtxBuffer.Size;
    if (vtx_start >= vtx_end) return;

    const float global_alpha = ImGui::GetStyle().Alpha;

    float min_y = FLT_MAX;
    float max_y = -FLT_MAX;
    for (int i = vtx_start; i < vtx_end; i++)
    {
        min_y = std::min(min_y, draw_list->VtxBuffer[i].pos.y);
        max_y = std::max(max_y, draw_list->VtxBuffer[i].pos.y);
    }

    float height = std::max(1.0f, max_y - min_y);

    for (int i = vtx_start; i < vtx_end; i++)
    {
        float t = ImSaturate((draw_list->VtxBuffer[i].pos.y - min_y) / height);
        ImVec4 grad_col = CTools::Get().Lerp(col_top, col_bottom, t);

        ImU32 original_col = draw_list->VtxBuffer[i].col;
        float original_alpha = ((original_col >> IM_COL32_A_SHIFT) & 0xFF) / 255.0f;

        grad_col.w *= original_alpha * global_alpha;
        draw_list->VtxBuffer[i].col = ImGui::ColorConvertFloat4ToU32(grad_col);
    }
}