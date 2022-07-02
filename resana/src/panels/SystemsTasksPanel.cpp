#include "SystemTasksPanel.h"

#include <imgui.h>

#include "core/Application.h"
#include "core/Core.h"

namespace RESANA {

SystemTasksPanel* SystemTasksPanel::sInstance = nullptr;
;
SystemTasksPanel::SystemTasksPanel()
    : mUpdateInterval(TimeTick::Rate::Normal)
{
}

SystemTasksPanel::~SystemTasksPanel() = default;

bool SystemTasksPanel::ShouldClose() const
{
    return !mPanelOpen;
}

SystemTasksPanel* SystemTasksPanel::Create()
{
    RS_CORE_ASSERT(!sInstance, "Instance is already created!")
    sInstance = new SystemTasksPanel();
    return sInstance;
}

void SystemTasksPanel::ShowPanel(bool* pOpen)
{
    IM_ASSERT(ImGui::GetCurrentContext() != NULL && "Missing dear imgui context!");

    if ((mPanelOpen = *pOpen)) {
        // Create window and assign each panel to a tab
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar;

        if (ImGui::Begin("System Tasks", pOpen, windowFlags)) {
            ShowMenuBar();

            RS_CORE_ASSERT(pOpen, "Resource Window should be open!")

            if (ImGui::Button("Process Details", { 110.0f, 20.0f })) {
                mShowProcPanel = true;
                mShowPerfPanel = false;
            }

            ImGui::SameLine();
            if (ImGui::Button("Performance", { 90.0f, 20.0f })) {
                mShowPerfPanel = true;
                mShowProcPanel = false;
            }

            ImGui::SameLine();
            static std::string label = "Normal";
            static int i = 1;
            if (ImGui::Button(label.c_str(), { 90.0f, 20.0f })) {
                if (i == 0) {
                    mUpdateInterval = TimeTick::Slow;
                    label = "Slow";
                } else if (i == 1) {
                    mUpdateInterval = TimeTick::Normal;
                    label = "Normal";
                } else if (i == 2) {
                    mUpdateInterval = TimeTick::Fast;
                    label = "Fast";
                }

                i = (i + 1) % 3;
            }

            mProcPanel->ShowPanel(&mShowProcPanel);
            mPerfPanel->ShowPanel(&mShowPerfPanel);
        }
        ImGui::End();
    }
}

void SystemTasksPanel::UpdatePanels(Timestep interval)
{
    if (mPanelOpen) {
        if (const auto tick = mTimeTick.GetTickCount(); tick > mLastTick) {
            for (Panel* panel : mPanelStack) {
                if (panel->IsPanelOpen()) {
                    panel->OnUpdate(interval);
                }
            }
            mLastTick = tick;
        }
    }
}

void SystemTasksPanel::OnAttach()
{
    mPerfPanel = new PerformancePanel();
    mPerfPanel->OnAttach();
    mPanelStack.PushLayer(mPerfPanel);

    mProcPanel = new ProcessPanel();
    mProcPanel->OnAttach();
    mPanelStack.PushLayer((Panel*)mProcPanel);

    mTimeTick.Start();
}

void SystemTasksPanel::OnDetach()
{
    Close();
}

void SystemTasksPanel::OnUpdate(Timestep ts)
{
    if (mPanelOpen) {
        // Update the panels
        UpdatePanels(mUpdateInterval);
    }
}

void SystemTasksPanel::OnImGuiRender()
{
}

void SystemTasksPanel::Close()
{
    if (sInstance) {
        sInstance->mPanelOpen = false;
        sInstance->mTimeTick.Stop();
        sInstance->CloseChildren();

        const auto& app = Application::Get();
        auto& threadPool = app.GetThreadPool();
        threadPool.Queue([&] { delete sInstance; });

        sInstance = nullptr;
    }
}

void SystemTasksPanel::CloseChildren()
{
    for (Panel* panel : mPanelStack) {
        panel->OnDetach();
    }
}

void SystemTasksPanel::ShowMenuBar()
{
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Close")) {
                mPanelOpen = false;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit")) {
            ImGui::Text("Test edit item");
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            if (mShowProcPanel) {
                mProcPanel->ShowPanelMenu();
            } else {
                ImGui::Text("Test view item");
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}
}
