#include "Sandbox.h"

#include "core/Application.h"
#include "core/EntryPoint.h" // For CreateApplication()

#include <imgui.h>

namespace RESANA {

ExampleLayer::ExampleLayer()
    : Layer("ExampleLayer")
{
}

ExampleLayer::~ExampleLayer()
{
    ExampleLayer::OnDetach();
};

void ExampleLayer::OnAttach()
{
    mSystemTasksPanel = SystemTasksPanel::Create();
    mSystemTasksPanel->OnAttach();
}

void ExampleLayer::OnDetach()
{
    if (SystemTasksPanel::IsValid()) {
        mShowSystemTasksPanel = false;
        mSystemTasksPanel->OnDetach();
    }
}

void ExampleLayer::OnUpdate(Timestep ts)
{
    if (!SystemTasksPanel::IsValid()) {
        return;
    }

    if (mShowSystemTasksPanel && mSystemTasksPanel->IsPanelOpen()) {
        mSystemTasksPanel->OnUpdate(ts);
    }
}

void ExampleLayer::OnImGuiRender()
{
    static bool showDemo = false;

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Quit")) {
                Application::Get().Terminate();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("System Tasks", nullptr, &mShowSystemTasksPanel);
            ImGui::MenuItem("ImGui Demo", nullptr, &showDemo);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    if (showDemo) {
        ImGui::ShowDemoWindow(&showDemo);
    }

    ShowSystemTasksPanel();
}

void ExampleLayer::ShowSystemTasksPanel()
{
    if (!SystemTasksPanel::IsValid() && mShowSystemTasksPanel) {
        ExampleLayer::OnAttach();
    } else if (SystemTasksPanel::IsValid()) {
        mSystemTasksPanel->ShowPanel(&mShowSystemTasksPanel);

        if (mSystemTasksPanel->ShouldClose()) {
            mShowSystemTasksPanel = false;
            SystemTasksPanel::Close();
            mSystemTasksPanel = nullptr;
        }
    }
}

class Sandbox final : public Application {
public:
    Sandbox()
    {
        PushLayer(new ExampleLayer());
    }

    ~Sandbox() override = default;
};

// ----------------------------------------------------
// [ENTRY POINT]
// ----------------------------------------------------
Application* CreateApplication()
{
    return new Sandbox();
}
}
