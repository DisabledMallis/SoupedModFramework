#include "imgui.h"
#include "imgui_internal.h"

//TODO: This should really be a shader
void ImGui::Rainbow(float thickness, float speed) {
    static float staticHue = 0;
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 win_size = ImGui::GetWindowSize();
    float win_width = win_size.x;
    float x1 = window->Pos.x;
    float x2 = window->Pos.x + window->Size.x;
    float y1 = window->DC.CursorPos.y;
    float y2 = window->DC.CursorPos.y + thickness;
    window->DC.CursorPos.y += thickness;
    //const ImRect bb(ImVec2(x1, window->DC.CursorPos.y), ImVec2(x2, window->DC.CursorPos.y + thickness_draw));
    staticHue -= ImGui::GetIO().DeltaTime * speed;
    if (staticHue < -1.f) staticHue += 1.f;
    for (int i = 0; i < win_width; i++)
    {
        float hue = staticHue + (1.f / (float)win_width) * i;
        if (hue < 0.f) hue += 1.f;
        ImColor cRainbow = ImColor::HSV(hue, 1.f, 1.f);
        draw_list->AddRectFilled(ImVec2(x1 + i, y1), ImVec2(x1 + i + 1, y2), cRainbow);
    }
}

void ImGui::StyleColorsCustom() {
    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 0.0f;
    style.ChildRounding = 5.0f;
    style.GrabRounding = 0.0f;
    style.PopupRounding = 0.0f;
    style.FrameRounding = 0.0f;
    style.ScrollbarRounding = 0.0f;
    style.TabRounding = 0.0f;
    style.FrameBorderSize = 1.0f;
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Button] = ImColor(255, 0, 128, 120);
    colors[ImGuiCol_ButtonHovered] = ImColor(255, 0, 128, 150);
    colors[ImGuiCol_ButtonActive] = ImColor(255, 0, 128, 255);
    colors[ImGuiCol_CheckMark] = ImColor(255, 0, 128, 255);
    colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.24f, 0.24f, 0.40f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.50f, 0.50f, 0.50f, 0.67f);
    colors[ImGuiCol_Header] = ImColor(0, 0, 0, 255);
    colors[ImGuiCol_HeaderHovered] = ImColor(40, 40, 40, 255);
    colors[ImGuiCol_HeaderActive] = ImColor(60, 60, 60, 255);
}

float ImGui::EaseOutExpo(float x) {
    return x == 1 ? 1 : 1 - pow(2, -10 * x);
}