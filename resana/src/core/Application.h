#pragma once

#include "Window.h"
#include "Layer.h"
#include "imgui/ImGuiLayer.h"
#include "LayerStack.h"

namespace RESANA {

    class Application {
    public:
        Application();
        ~Application();

        void PushLayer(Layer *layer);
        void Run();
        void Terminate();
        bool IsMinimized() const;

        inline Window &GetWindow() { return *mWindow; }

        inline static Application &Get() { return *sInstance; }

    private:
        std::shared_ptr<Window> mWindow;
        ImGuiLayer *mImGuiLayer;
        LayerStack mLayerStack;
        bool mRunning = true;
        bool mMinimized = false;

        float mLastFrameTime = 0.0f;

        static Application *sInstance;
    };

} // RESANA

