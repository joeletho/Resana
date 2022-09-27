#include "Panel.h"
#include "SystemTasksPanel.h"

#include <imgui.h>

#include <memory>

#include "core/Application.h"
#include "core/Core.h"

namespace RESANA {

std::shared_ptr<SystemTasksPanel> SystemTasksPanel::sInstance = nullptr;
;
SystemTasksPanel::SystemTasksPanel()
    : mUpdateInterval(TimeTick::Rate::Normal) {}

SystemTasksPanel::~SystemTasksPanel() {
  mTimeTick.Stop();
  mProcPanel.reset();
  mPerfPanel.reset();
  RS_CORE_TRACE("SystemTaskPanel destroyed");
}

bool SystemTasksPanel::ShouldClose() const { return !mPanelOpen; }

std::shared_ptr<SystemTasksPanel> SystemTasksPanel::Create() {
  RS_CORE_ASSERT(!sInstance, "Instance is already created!")
  sInstance.reset(new SystemTasksPanel);
  return sInstance;
}

void SystemTasksPanel::ShowPanel(bool *pOpen) {
  IM_ASSERT(ImGui::GetCurrentContext() != nullptr &&
            "Missing dear imgui context!");
  mPanelOpen = *pOpen;

  // Create window and assign each panel to a tab
  ImGuiWindowFlags windowFlags =
      ImGuiWindowFlags_None | ImGuiWindowFlags_NoCollapse |
      ImGuiWindowFlags_NoMove | ImGuiWindowFlags_MenuBar;

  if (ImGui::Begin("System Tasks", nullptr, windowFlags)) {
    ShowMenuBar();
    if (ImGui::Button("Process Details", {110.0f, 20.0f})) {
      mShowProcPanel = true;
      mShowPerfPanel = false;
    }

    ImGui::SameLine();
    if (ImGui::Button("Performance", {90.0f, 20.0f})) {
      mShowPerfPanel = true;
      mShowProcPanel = false;
    }

    ImGui::SameLine();
    static std::string label = "Normal";
    static int i = 1;
    if (ImGui::Button(label.c_str(), {90.0f, 20.0f})) {
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

void SystemTasksPanel::UpdatePanels(Timestep interval) {
  if (mPanelOpen) {
    if (const auto tick = mTimeTick.GetTickCount(); tick > mLastTick) {
      for (const auto &panel : mPanelStack) {
        if (panel->IsPanelOpen()) {
          panel->OnUpdate(interval);
        }
      }
      mLastTick = tick;
    }
  }
}

void SystemTasksPanel::OnAttach() {
  mPerfPanel = std::make_shared<PerformancePanel>();
  mPerfPanel->OnAttach();
  mPanelStack.PushLayer(mPerfPanel);

  mProcPanel = std::make_shared<ProcessPanel>();
  mProcPanel->OnAttach();
  mPanelStack.PushLayer(mProcPanel);

  mTimeTick.Start();
}

void SystemTasksPanel::OnDetach() { Close(); }

void SystemTasksPanel::OnUpdate(Timestep ts) {
  if (mPanelOpen) {
    UpdatePanels(mUpdateInterval);
  }
}

void SystemTasksPanel::OnImGuiRender() {}

void SystemTasksPanel::Close() {
  if (sInstance) {
    sInstance->mPanelOpen = false;
    sInstance->mTimeTick.Stop();
    sInstance->CloseChildren();
    sInstance.reset();
    sInstance = nullptr;
  }
}

void SystemTasksPanel::CloseChildren() {
  for (const auto &panel : mPanelStack) {
    panel->OnDetach();
  }
}

void SystemTasksPanel::ShowMenuBar() const {
  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("Quit")) {
        Application::Get().Terminate();
      }
      ImGui::EndMenu();
    }
    if (mShowProcPanel) {
      if (ImGui::BeginMenu("View")) {
        mProcPanel->ShowViewMenu();
        ImGui::EndMenu();
      }
    }
    ImGui::EndMenuBar();
  }
}
} // namespace RESANA
