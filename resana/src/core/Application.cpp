#include "rspch.h"

#include "Application.h"

#include "Core.h"
#include "Log.h"
#include "Renderer.h"
#include <memory>

#include "system/ThreadPool.h"

namespace RESANA {

Application* Application::sInstance = nullptr;

Application::Application() {
  RS_CORE_ASSERT(!sInstance, "Application already exists!");
  sInstance = this;
  mRunning = true;

  mWindow = std::unique_ptr<Window>(Window::Create());

  // Start statics
  Renderer::Init();
  Time::Start();

  mThreadPool = std::make_shared<ThreadPool>();
  mThreadPool->Start();

  mImGuiLayer = std::make_shared<ImGuiLayer>();
  PushLayer(mImGuiLayer);
}

Application::~Application() {
  for (const auto &layer : mLayerStack) {
    layer->OnDetach();
  }

  Time::Stop();
  mThreadPool->Stop();

  RS_CORE_TRACE("Application destroyed");
}

void Application::PushLayer(const std::shared_ptr<Layer> &layer) {
  mLayerStack.PushLayer(layer);
  layer->OnAttach();
}

void Application::Run() {
  Timestep ts;

  while (mRunning) {
    // Check if window has been closed
    mRunning = !glfwWindowShouldClose((GLFWwindow *)mWindow->GetNativeWindow());

    if (!IsMinimized()) {
      for (auto &layer : mLayerStack) {
        layer->OnUpdate(ts);
      }

      ImGuiLayer::Begin();
      for (auto &layer : mLayerStack) {
        layer->OnImGuiRender();
      }
      ImGuiLayer::End();
    }

    mWindow->Update();
  }

  // Clean up
  glfwDestroyWindow((GLFWwindow *)mWindow->GetNativeWindow());
  glfwTerminate();
  glfwSetErrorCallback(nullptr);
}

void Application::Terminate() { mRunning = false; }

bool Application::IsMinimized() const {
  return mWindow->GetWidth() == 0 || mWindow->GetHeight() == 0;
}

} // namespace RESANA
