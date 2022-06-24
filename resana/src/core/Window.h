#pragma once

#include <string>
#include <type_traits>

#include <GLFW/glfw3.h>

namespace RESANA {

    struct WindowProps {
        std::string Title;
        unsigned int Width;
        unsigned int Height;

        explicit WindowProps(std::string title = "Resource Analyzer",
                             unsigned int width = 1280,
                             unsigned int height = 720)
                : Title(std::move(title)), Width(width), Height(height) {}
    };

    class Window {
    public:
        explicit Window(const WindowProps &props);
        ~Window();

        static Window *Create(const WindowProps &props = WindowProps());

        [[nodiscard]] inline void *GetNativeWindow() const { return mWindow; }

        [[nodiscard]] inline unsigned int GetWidth() const { return mData.Width; }

        [[nodiscard]] inline unsigned int GetHeight() const { return mData.Height; }

        void SetVSync(bool enabled);
        void Update();

        [[nodiscard]] bool IsVSync() const;

    private:
        void Init(const WindowProps &props);
        void Shutdown();

    private:
        GLFWwindow *mWindow;

        struct WindowData {
            WindowData() = default;
            std::string Title;
            unsigned int Width{}, Height{};
            bool VSync{};
        };

        WindowData mData;
    };

}