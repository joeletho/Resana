#pragma once

#include "Layer.h"
#include "LayerStack.h"
#include "Window.h"
#include "imgui/ImGuiLayer.h"

#include <memory>

#include "system/ThreadPool.h"

namespace RESANA {

class Application {
public:
  Application();
  virtual ~Application();

  void PushLayer(const std::shared_ptr<Layer> &layer);
  void Run();

  void Terminate();
  [[nodiscard]] bool IsMinimized() const;

  [[nodiscard]] Window &GetWindow() const { return *mWindow; }
  [[nodiscard]] ThreadPool &GetThreadPool() const { return *mThreadPool; }

  static Application &Get() { return *sInstance; }

private:
  std::shared_ptr<Window> mWindow;
  std::shared_ptr<ImGuiLayer> mImGuiLayer;
  LayerStack<Layer> mLayerStack;
  std::shared_ptr<ThreadPool> mThreadPool;
  bool mRunning{true};
  bool mMinimized{false};

  static Application* sInstance;
};

// To be defined by CLIENT
Application *CreateApplication();

} // namespace RESANA
