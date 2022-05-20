#include "rspch.h"

#include "Application.h"
#include "Renderer.h"
#include "Core.h"
#include "Log.h"

#include "perfdata/PerfManager.h"

namespace RESANA {

    Application *Application::sInstance = nullptr;

    Application::Application() {
        RS_CORE_ASSERT(!sInstance, "Application already exists!");
        sInstance = this;
        mRunning = true;

        mWindow = std::unique_ptr<Window>(Window::Create());

        // Start statics
        PerfManager::Init();
        Renderer::Init();

        mImGuiLayer = new ImGuiLayer();
        PushLayer(mImGuiLayer);

    }

    Application::~Application() = default;


    void Application::PushLayer(Layer *layer) {
        mLayerStack.PushLayer(layer);
        layer->OnAttach();
    }

    void Application::Run() {

        while (mRunning) {
            // Check if window has been closed
            mRunning = !glfwWindowShouldClose((GLFWwindow *) mWindow->GetNativeWindow());

            mMinimized = (mWindow->GetWidth() == 0 || mWindow->GetHeight() == 0);
            if (!mMinimized) {
                for (Layer *layer: mLayerStack) {
                    layer->OnUpdate();
                }
            }

            ImGuiLayer::Begin();
            for (Layer *layer: mLayerStack) {
                layer->OnImGuiRender();
            }
            ImGuiLayer::End();

            mWindow->Update();
        }

        // Clean up
        glfwDestroyWindow((GLFWwindow *) mWindow->GetNativeWindow());
        glfwTerminate();
        glfwSetErrorCallback(nullptr);
    }

    void Application::Terminate() {
        mRunning = false;
    }


} // RESANA



// -----------------------------------------------------------------
// [[ENTRY POINT]]
// -----------------------------------------------------------------
int main(int argc, char **argv) {
    RESANA::Log::Init();

    RS_CORE_WARN("Application initialized!");

    auto *app = new RESANA::Application;
    app->Run();
    delete app;
}