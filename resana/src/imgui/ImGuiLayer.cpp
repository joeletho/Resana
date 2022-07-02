#include "ImGuiLayer.h"
#include "rspch.h"

#include "core/Application.h"

#include "imgui/ImGuiBuild.h"

#include <imgui.h>

namespace RESANA {

ImGuiLayer::ImGuiLayer()
    : Layer("ImGuiLayer")
{
}

ImGuiLayer::~ImGuiLayer() = default;

void ImGuiLayer::OnAttach()
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.IniFilename = R"(..\..\resana\config\imgui.ini)";
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-viewport / Platform windows

    // Setup Dear ImGui style
    // ImGui::StyleColorsLight();
    // ImGui::StyleColorsDark();
    SetTaskManagerTheme();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to the style
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    Application& app = Application::Get();
    auto* window = (GLFWwindow*)(app.GetWindow().GetNativeWindow());

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460 core");
}

void ImGuiLayer::OnDetach()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiLayer::Begin()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiLayer::End()
{
    ImGuiIO& io = ImGui::GetIO();
    Application& app = Application::Get();
    io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());

    // Rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}

void ImGuiLayer::ShowImGuiDockspace()
{
    static bool optFullscreen = true;
    static bool optPadding = false;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if (optFullscreen) {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    } else {
        dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
    }

    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
    // and handle the pass-thru hole, so we ask Begin() to not render a background.
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) {
        window_flags |= ImGuiWindowFlags_NoBackground;
    }

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
    if (!optPadding) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    }
    ImGui::Begin("DockSpace Demo", nullptr, window_flags);
    if (!optPadding) {
        ImGui::PopStyleVar();
    }

    if (optFullscreen) {
        ImGui::PopStyleVar(2);
    }

    // Submit the DockSpace
    const ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        const ImGuiID dockspaceId = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), dockspace_flags);
    }

    ImGui::End();
}

void ImGuiLayer::OnImGuiRender()
{
    ShowImGuiDockspace();
}

void ImGuiLayer::SetTaskManagerTheme()
{
    ImGui::StyleColorsLight();

    const float sizePixels = 14.0f;

    ImFontConfig config;
    config.OversampleH = 2;
    config.OversampleV = 1;
    config.GlyphExtraSpacing.x = 0.0f;

    auto& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("assets\\fonts\\REFSAN.TTF", sizePixels, &config);
    io.Fonts->AddFontDefault();

    // Set theme colors
    auto& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // colors[ImGuiCol_Text] = {0.0f, 0.0f, 0.0f, 1.0f};
    // colors[ImGuiCol_WindowBg] = { 1.0f,1.0f,1.0f,1.0f };
    // colors[ImGuiCol_ChildBg];
    // colors[ImGuiCol_PopupBg];
    // colors[ImGuiCol_FrameBg];
    // colors[ImGuiCol_FrameBgHovered];
    // colors[ImGuiCol_FrameBgActive];
    // colors[ImGuiCol_TitleBg]= { 204/255.0f,204/255.0f,204/255.0f,1.0f };
    // colors[ImGuiCol_TitleBgActive]={ 1.0f,1.0f,1.0f,1.0f };
    // colors[ImGuiCol_MenuBarBg];
    // colors[ImGuiCol_ScrollbarBg];
    // colors[ImGuiCol_ScrollbarGrab];
    // colors[ImGuiCol_ScrollbarGrabHovered];
    // colors[ImGuiCol_ScrollbarGrabActive];
    // colors[ImGuiCol_CheckMark];
    // colors[ImGuiCol_SliderGrab];
    // colors[ImGuiCol_SliderGrabActive];
    // colors[ImGuiCol_Button];
    // colors[ImGuiCol_ButtonHovered];
    // colors[ImGuiCol_ButtonActive];
    // colors[ImGuiCol_Header];
    // colors[ImGuiCol_HeaderHovered];
    // colors[ImGuiCol_HeaderActive];
    // colors[ImGuiCol_Separator] = { 36/255.0f,36/255.0f,36/255.0f, 1.0f};
    // colors[ImGuiCol_SeparatorHovered];
    // colors[ImGuiCol_SeparatorActive];
    // colors[ImGuiCol_Tab]= { 204/255.0f,204/255.0f,204/255.0f,1.0f };
    // colors[ImGuiCol_TabHovered]={ 66/255.0f,150/255.0f,250/255.0f, 240/255.0f };
    // colors[ImGuiCol_TabActive]={ 1.0f,1.0f,1.0f,1.0f };
    // colors[ImGuiCol_TabUnfocused]= { 204/255.0f,204/255.0f,204/255.0f,1.0f };
    // colors[ImGuiCol_TabUnfocusedActive]={ 1.0f,1.0f,1.0f,1.0f };
    // colors[ImGuiCol_DockingPreview];
    // colors[ImGuiCol_DockingEmptyBg];
    // colors[ImGuiCol_TableHeaderBg]= { 1.0f,1.0f,1.0f,1.0f };;
    // colors[ImGuiCol_TableBorderStrong]= { 36/255.0f,36/255.0f,36/255.0f, 1.0f};
    // colors[ImGuiCol_TableBorderLight]= { 110/255.0f,110/255.0f,128/255.0f,128/255.0f };;
    // colors[ImGuiCol_TableRowBg];
    // colors[ImGuiCol_TableRowBgAlt];
    // colors[ImGuiCol_TextSelectedBg];

    style.TabBorderSize = 1.0f;
    style.CellPadding = { 12.0f, 6.0f };
    style.WindowRounding = 1.0f;
    style.ChildRounding = 1.0f;
    style.FrameRounding = 1.0f;
    style.GrabRounding = 1.0f;
    style.PopupRounding = 1.0f;
    style.ScrollbarRounding = 1.0f;
    style.TabRounding = 0.0f;
}
} // RESANA