#include "imgui_wrappers.h"
#include "tools.h"
#include <windows.h>
#include <shellapi.h>
#include <vector>
#include <cmath>

#ifndef IM_PI
#define IM_PI 3.14159265358979323846f
#endif

namespace ImGuiSnapWindow
{
    std::vector<ImGui::SnapWindow> g_SnapWindowsPrev;
    std::vector<ImGui::SnapWindow> g_SnapWindowsCurr;
}

static ImVec2 ApplySnapping(ImGuiID self_id, ImVec2 pos, ImVec2 size, float snap_dist = 8.0f)
{
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImVec2 vp_min = vp->WorkPos;
    ImVec2 vp_max(vp->WorkPos.x + vp->WorkSize.x, vp->WorkPos.y + vp->WorkSize.y);

    if (std::fabs(pos.x - vp_min.x) < snap_dist) pos.x = vp_min.x;
    if (std::fabs(pos.y - vp_min.y) < snap_dist) pos.y = vp_min.y;
    if (std::fabs((pos.x + size.x) - vp_max.x) < snap_dist)
        pos.x = vp_max.x - size.x;
    if (std::fabs((pos.y + size.y) - vp_max.y) < snap_dist)
        pos.y = vp_max.y - size.y;

    for (const auto& w : ImGuiSnapWindow::g_SnapWindowsPrev)
    {
        if (w.id == self_id)
            continue;

        bool local_snap_x = false;
        bool local_snap_y = false;

        if (std::fabs(pos.y - (w.pos.y + w.size.y)) < snap_dist)
        {
            pos.y = w.pos.y + w.size.y;
            local_snap_y = true;
        }
        else if (std::fabs((pos.y + size.y) - w.pos.y) < snap_dist)
        {
            pos.y = w.pos.y - size.y;
            local_snap_y = true;
        }

        if (std::fabs(pos.x - (w.pos.x + w.size.x)) < snap_dist)
        {
            pos.x = w.pos.x + w.size.x;
            local_snap_x = true;
        }
        else if (std::fabs((pos.x + size.x) - w.pos.x) < snap_dist)
        {
            pos.x = w.pos.x - size.x;
            local_snap_x = true;
        }

        if (local_snap_y)
        {
            if (std::fabs(pos.x - w.pos.x) < snap_dist)
                pos.x = w.pos.x;
            else if (std::fabs((pos.x + size.x) - (w.pos.x + w.size.x)) < snap_dist)
                pos.x = w.pos.x + w.size.x - size.x;
            else
            {
                float self_center = pos.x + size.x * 0.5f;
                float other_center = w.pos.x + w.size.x * 0.5f;

                if (std::fabs(self_center - other_center) < snap_dist)
                    pos.x = other_center - size.x * 0.5f;
            }

            break;
        }

        if (local_snap_x)
        {
            if (std::fabs(pos.y - w.pos.y) < snap_dist)
                pos.y = w.pos.y;
            else if (std::fabs((pos.y + size.y) - (w.pos.y + w.size.y)) < snap_dist)
                pos.y = w.pos.y + w.size.y - size.y;
            else
            {
                float self_center = pos.y + size.y * 0.5f;
                float other_center = w.pos.y + w.size.y * 0.5f;

                if (std::fabs(self_center - other_center) < snap_dist)
                    pos.y = other_center - size.y * 0.5f;
            }

            break;
        }
    }

    return pos;
}

static int InputTextCallback(ImGuiInputTextCallbackData* data)
{
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
    {
        auto* str = static_cast<std::string*>(data->UserData);
        str->resize(data->BufTextLen);
        data->Buf = str->data();
    }
    return 0;
}

void ImGui::BeginSnapFrame()
{
    ImGuiSnapWindow::g_SnapWindowsPrev = std::move(ImGuiSnapWindow::g_SnapWindowsCurr);
    ImGuiSnapWindow::g_SnapWindowsCurr.clear();
}

bool ImGui::BeginSnap(const char* name, bool* p_open, ImGuiWindowFlags flags)
{
    if (!ImGui::Begin(name, p_open, flags))
        return false;

    ImGuiID id = ImGui::GetID(name);
    ImVec2 pos = ImGui::GetWindowPos();
    ImVec2 size = ImGui::GetWindowSize();

    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
    {
        ImVec2 snapped = ApplySnapping(id, pos, size);
        if (snapped.x != pos.x || snapped.y != pos.y)
            ImGui::SetWindowPos(snapped, ImGuiCond_Always);
    }

    ImGuiSnapWindow::g_SnapWindowsCurr.emplace_back(id, pos, size);
    return true;
}

bool ImGui::InputText(const char* label, std::string& str, ImGuiInputTextFlags flags)
{
    flags |= ImGuiInputTextFlags_CallbackResize;
    return ImGui::InputText(label, str.data(), str.size() + 1, flags, InputTextCallback, &str);
}

bool ImGui::InputText(const char* label, std::wstring& wstr, ImGuiInputTextFlags flags)
{
    std::string str = CTools::Get().WStringToUtf8(wstr);
    bool ret = ImGui::InputText(label, str, flags);
    if (ret) wstr = CTools::Get().Utf8ToWString(str.c_str());
    return ret;
}

bool ImGui::Link(const char* text, const wchar_t* url)
{
    static const ImVec4 normalColor(0.2f, 0.6f, 1.0f, 1.0f);
    static const ImVec4 hoverColor(0.3f, 0.8f, 1.0f, 1.0f);
    bool ret = false;

    ImGui::PushStyleColor(ImGuiCol_Text, normalColor);
    ImGui::Text("%s", text);
    ImGui::PopStyleColor();

    if (ImGui::IsItemHovered())
    {
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 p = ImGui::GetItemRectMin();
        ImVec2 q = ImGui::GetItemRectMax();
        draw_list->AddRectFilled(p, q, IM_COL32(0, 0, 0, 0), 0.0f);
        draw_list->AddText(p, ImColor(hoverColor), text);
        draw_list->AddLine(ImVec2(p.x, q.y), ImVec2(q.x, q.y), ImColor(hoverColor), 1.0f);

        if (ImGui::IsItemClicked())
        {
            ret = true;
            ShellExecute(NULL, L"open", url, NULL, NULL, SW_SHOWNORMAL);
        }
    }

    return ret;
}

void ImGui::LoadDefaultStyle(ImGuiStyle& style)
{
    style.Alpha = 1.0f;             // Global alpha applies to everything in Dear ImGui.
    style.DisabledAlpha = 0.60f;            // Additional alpha multiplier applied by BeginDisabled(). Multiply over current value of Alpha.
    style.WindowPadding = ImVec2(8, 8);      // Padding within a window
    style.WindowRounding = 0.0f;             // Radius of window corners rounding. Set to 0.0f to have rectangular windows. Large values tend to lead to variety of artifacts and are not recommended.
    style.WindowBorderSize = 1.0f;             // Thickness of border around windows. Generally set to 0.0f or 1.0f. Other values not well tested.
    style.WindowBorderHoverPadding = 4.0f;             // Hit-testing extent outside/inside resizing border. Also extend determination of hovered window. Generally meaningfully larger than WindowBorderSize to make it easy to reach borders.
    style.WindowMinSize = ImVec2(32, 32);    // Minimum window size
    style.WindowTitleAlign = ImVec2(0.0f, 0.5f);// Alignment for title bar text
    style.WindowMenuButtonPosition = ImGuiDir_Left;    // Position of the collapsing/docking button in the title bar (left/right). Defaults to ImGuiDir_Left.
    style.ChildRounding = 0.0f;             // Radius of child window corners rounding. Set to 0.0f to have rectangular child windows
    style.ChildBorderSize = 1.0f;             // Thickness of border around child windows. Generally set to 0.0f or 1.0f. Other values not well tested.
    style.PopupRounding = 0.0f;             // Radius of popup window corners rounding. Set to 0.0f to have rectangular child windows
    style.PopupBorderSize = 1.0f;             // Thickness of border around popup or tooltip windows. Generally set to 0.0f or 1.0f. Other values not well tested.
    style.FramePadding = ImVec2(4, 3);      // Padding within a framed rectangle (used by most widgets)
    style.FrameRounding = 0.0f;             // Radius of frame corners rounding. Set to 0.0f to have rectangular frames (used by most widgets).
    style.FrameBorderSize = 0.0f;             // Thickness of border around frames. Generally set to 0.0f or 1.0f. Other values not well tested.
    style.ItemSpacing = ImVec2(8, 4);      // Horizontal and vertical spacing between widgets/lines
    style.ItemInnerSpacing = ImVec2(4, 4);      // Horizontal and vertical spacing between within elements of a composed widget (e.g. a slider and its label)
    style.CellPadding = ImVec2(4, 2);      // Padding within a table cell. Cellpadding.x is locked for entire table. CellPadding.y may be altered between different rows.
    style.TouchExtraPadding = ImVec2(0, 0);      // Expand reactive bounding box for touch-based system where touch position is not accurate enough. Unfortunately we don't sort widgets so priority on overlap will always be given to the first widget. So don't grow this too much!
    style.IndentSpacing = 21.0f;            // Horizontal spacing when e.g. entering a tree node. Generally == (FontSize + FramePadding.x*2).
    style.ColumnsMinSpacing = 6.0f;             // Minimum horizontal spacing between two columns. Preferably > (FramePadding.x + 1).
    style.ScrollbarSize = 14.0f;            // Width of the vertical scrollbar, Height of the horizontal scrollbar
    style.ScrollbarRounding = 9.0f;             // Radius of grab corners rounding for scrollbar
    style.ScrollbarPadding = 2.0f;             // Padding of scrollbar grab within its frame (same for both axises)
    style.GrabMinSize = 12.0f;            // Minimum width/height of a grab box for slider/scrollbar
    style.GrabRounding = 0.0f;             // Radius of grabs corners rounding. Set to 0.0f to have rectangular slider grabs.
    style.LogSliderDeadzone = 4.0f;             // The size in pixels of the dead-zone around zero on logarithmic sliders that cross zero.
    style.ImageBorderSize = 0.0f;             // Thickness of border around tabs.
    style.TabRounding = 5.0f;             // Radius of upper corners of a tab. Set to 0.0f to have rectangular tabs.
    style.TabBorderSize = 0.0f;             // Thickness of border around tabs.
    style.TabMinWidthBase = 1.0f;             // Minimum tab width, to make tabs larger than their contents. TabBar buttons are not affected.
    style.TabMinWidthShrink = 80.0f;            // Minimum tab width after shrinking, when using ImGuiTabBarFlags_FittingPolicyMixed policy.
    style.TabCloseButtonMinWidthSelected = -1.0f;       // -1: always visible. 0.0f: visible when hovered. >0.0f: visible when hovered if minimum width.
    style.TabCloseButtonMinWidthUnselected = 0.0f;        // -1: always visible. 0.0f: visible when hovered. >0.0f: visible when hovered if minimum width. FLT_MAX: never show close button when unselected.
    style.TabBarBorderSize = 1.0f;             // Thickness of tab-bar separator, which takes on the tab active color to denote focus.
    style.TabBarOverlineSize = 1.0f;             // Thickness of tab-bar overline, which highlights the selected tab-bar.
    style.TableAngledHeadersAngle = 35.0f * (IM_PI / 180.0f); // Angle of angled headers (supported values range from -50 degrees to +50 degrees).
    style.TableAngledHeadersTextAlign = ImVec2(0.5f, 0.0f);// Alignment of angled headers within the cell
    style.TreeLinesFlags = ImGuiTreeNodeFlags_DrawLinesNone;
    style.TreeLinesSize = 1.0f;             // Thickness of outlines when using ImGuiTreeNodeFlags_DrawLines.
    style.TreeLinesRounding = 0.0f;             // Radius of lines connecting child nodes to the vertical line.
    style.DragDropTargetRounding = 0.0f;             // Radius of the drag and drop target frame.
    style.DragDropTargetBorderSize = 2.0f;             // Thickness of the drag and drop target border.
    style.DragDropTargetPadding = 3.0f;             // Size to expand the drag and drop target from actual target item size.
    style.ColorButtonPosition = ImGuiDir_Right;   // Side of the color button in the ColorEdit4 widget (left/right). Defaults to ImGuiDir_Right.
    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);// Alignment of button text when button is larger than text.
    style.SelectableTextAlign = ImVec2(0.0f, 0.0f);// Alignment of selectable text. Defaults to (0.0f, 0.0f) (top-left aligned). It's generally important to keep this left-aligned if you want to lay multiple items on a same line.
    style.SeparatorTextBorderSize = 3.0f;             // Thickness of border in SeparatorText()
    style.SeparatorTextAlign = ImVec2(0.0f, 0.5f);// Alignment of text within the separator. Defaults to (0.0f, 0.5f) (left aligned, center).
    style.SeparatorTextPadding = ImVec2(20.0f, 3.f);// Horizontal offset of text from each edge of the separator + spacing on other axis. Generally small values. .y is recommended to be == FramePadding.y.
    style.DisplayWindowPadding = ImVec2(19, 19);    // Window position are clamped to be visible within the display area or monitors by at least this amount. Only applies to regular windows.
    style.DisplaySafeAreaPadding = ImVec2(3, 3);      // If you cannot see the edge of your screen (e.g. on a TV) increase the safe area padding. Covers popups/tooltips as well regular windows.
    style.MouseCursorScale = 1.0f;             // Scale software rendered mouse cursor (when io.MouseDrawCursor is enabled). May be removed later.
    style.AntiAliasedLines = true;             // Enable anti-aliased lines/borders. Disable if you are really tight on CPU/GPU.
    style.AntiAliasedLinesUseTex = true;             // Enable anti-aliased lines/borders using textures where possible. Require backend to render with bilinear filtering (NOT point/nearest filtering).
    style.AntiAliasedFill = true;             // Enable anti-aliased filled shapes (rounded rectangles, circles, etc.).
    style.CurveTessellationTol = 1.25f;            // Tessellation tolerance when using PathBezierCurveTo() without a specific number of segments. Decrease for highly tessellated curves (higher quality, more polygons), increase to reduce quality.
    style.CircleTessellationMaxError = 0.30f;            // Maximum error (in pixels) allowed when using AddCircle()/AddCircleFilled() or drawing rounded corner rectangles with no explicit segment count specified. Decrease for higher quality but more geometry.

    // Behaviors
    style.HoverStationaryDelay = 0.15f;            // Delay for IsItemHovered(ImGuiHoveredFlags_Stationary). Time required to consider mouse stationary.
    style.HoverDelayShort = 0.15f;            // Delay for IsItemHovered(ImGuiHoveredFlags_DelayShort). Usually used along with HoverStationaryDelay.
    style.HoverDelayNormal = 0.40f;            // Delay for IsItemHovered(ImGuiHoveredFlags_DelayNormal). "
    style.HoverFlagsForTooltipMouse = ImGuiHoveredFlags_Stationary | ImGuiHoveredFlags_DelayShort | ImGuiHoveredFlags_AllowWhenDisabled;    // Default flags when using IsItemHovered(ImGuiHoveredFlags_ForTooltip) or BeginItemTooltip()/SetItemTooltip() while using mouse.
    style.HoverFlagsForTooltipNav = ImGuiHoveredFlags_NoSharedDelay | ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled;  // Default flags when using IsItemHovered(ImGuiHoveredFlags_ForTooltip) or BeginItemTooltip()/SetItemTooltip() while using keyboard/gamepad.

    // [Internal]
    style._MainScale = 1.0f;
}

void ImGui::LoadTimerStyle(ImGuiStyle& style)
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
    style.WindowMinSize = ImVec2(1.f, 1.f);
    style.FramePadding = ImVec2(6.f, 2.f);
    style.CellPadding = ImVec2(4.f, 0.f);
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