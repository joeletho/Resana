#include "Sandbox.h"

#include "core/Application.h"
#include "core/EntryPoint.h" // For CreateApplication()

#include <imgui.h>

namespace RESANA {

ExampleLayer::ExampleLayer() : Layer("ExampleLayer") {}

ExampleLayer::~ExampleLayer() {
  ExampleLayer::OnDetach();
  RS_CORE_TRACE("ExampleLayer destroyed");
};

void ExampleLayer::OnAttach() {
  mSystemTasksPanel = SystemTasksPanel::Create();
  mSystemTasksPanel->OnAttach();
}

void ExampleLayer::OnDetach() {
  if (SystemTasksPanel::IsValid()) {
    mSystemTasksPanel->OnDetach();
  }
  mSystemTasksPanel.reset();
}

void ExampleLayer::OnUpdate(Timestep ts) {
  if (!SystemTasksPanel::IsValid()) {
    return;
  }

  if (mSystemTasksPanel->IsPanelOpen()) {
    mSystemTasksPanel->OnUpdate(ts);
  }
}

void ExampleLayer::OnImGuiRender() { ShowSystemTasksPanel(); }

void ExampleLayer::ShowSystemTasksPanel() {
  if (!SystemTasksPanel::IsValid()) {
    ExampleLayer::OnAttach();
  } else {
    static bool showPanel = true;
    mSystemTasksPanel->ShowPanel(&showPanel);
  }
}

class Sandbox final : public Application {
public:
  Sandbox() { PushLayer(std::make_shared<ExampleLayer>()); }

  ~Sandbox() override = default;
};

// ----------------------------------------------------
// [ENTRY POINT]
// ----------------------------------------------------
Application *CreateApplication() { return new Sandbox(); }
} // namespace RESANA
