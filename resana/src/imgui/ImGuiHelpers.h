#pragma once

#include <imgui.h>

namespace ImGui {

// This function will wrap text w/in a selectable
static bool SelectableWrapped(const char* label, bool selected, ImGuiSelectableFlags flags, const float window_width)
{
    ImFont* font = GetFont();
    ImVec2 size = font->CalcTextSizeA(font->FontSize, GetContentRegionMax().x, window_width, label);

    if (Selectable("##hidden", selected, flags, ImVec2(0.0, size.y))) {
        return true;
    }

    const ImVec2 p0 = GetItemRectMin();
    const ImVec2 p1 = GetItemRectMax();
    const ImVec2 text_pos = ImVec2(p0.x + 5.0f, p0.y);
    ImDrawList* draw_list = GetWindowDrawList();

    ImVec4 clip_rect(p0.x, p0.y, p1.x + 5.0f, p1.y); // AddText() takes a ImVec4* here so let's convert.
    draw_list->AddText(font, font->FontSize, text_pos, IM_COL32_WHITE, label, NULL,
        window_width, &clip_rect);

    return false;
}

static bool SelectableWrapped(const char* label, bool selected, ImGuiSelectableFlags flags, const float window_width, const char* uniqueId)
{
    ImFont* font = GetFont();
    ImVec2 size = font->CalcTextSizeA(font->FontSize, GetContentRegionMax().x, window_width, label);
    auto& style = ImGui::GetStyle();

    if (Selectable(uniqueId, selected, flags, ImVec2(0.0, size.y))) {
        return true;
    }

    const ImVec2 p0 = GetItemRectMin();
    const ImVec2 p1 = GetItemRectMax();
    const ImVec2 text_pos = ImVec2(p0.x + 5.0f, p0.y);
    ImDrawList* draw_list = GetWindowDrawList();

    ImVec4 clip_rect(p0.x, p0.y, p1.x + 5.0f, p1.y); // AddText() takes a ImVec4* here so let's convert.
    draw_list->AddText(font, font->FontSize, text_pos, ImColor(style.Colors[ImGuiCol_Text]), label, NULL,
        window_width, &clip_rect);

    return false;
}

static bool Selectable(const char* label, bool selected, ImGuiSelectableFlags flags, const float window_width, const char* uniqueId)
{

    ImFont* font = GetFont();
    auto& style = ImGui::GetStyle();

    if (Selectable(uniqueId, selected, flags)) {
        return true;
    }

    const ImVec2 p0 = GetItemRectMin();
    const ImVec2 p1 = GetItemRectMax();
    const ImVec2 text_pos = ImVec2(p0.x + 5.0f, p0.y);
    ImDrawList* draw_list = GetWindowDrawList();

    ImVec4 clip_rect(p0.x, p0.y, p1.x + 5.0f, p1.y); // AddText() takes a ImVec4* here so let's convert.
    draw_list->AddText(font, font->FontSize, text_pos, ImColor(style.Colors[ImGuiCol_Text]), label, NULL,
        window_width, &clip_rect);

    return false;
}

static void SetRightJustify(const char* string)
{
    if (const auto xPos = (GetCursorPosX() + GetColumnWidth() - CalcTextSize(string).x
            - GetScrollX() - 2 * GetStyle().ItemSpacing.x);
        xPos > GetCursorPosX()) {
        SetCursorPosX(xPos);
    }
}

static void SetCenterJustify(const char* string)
{
    if (const auto xPos = (GetCursorPosX() + GetColumnWidth() - CalcTextSize(string).x - GetScrollX()) / 2;
        xPos > GetCursorPosX()) {
        SetCursorPosX(xPos);
    }
}

}