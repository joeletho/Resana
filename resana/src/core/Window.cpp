#include "Window.h"

#include "Core.h"
#include "Log.h"

#include <glad/glad.h>

namespace RESANA {

    static bool sGLFWInitialized = false;

    static void GlfwErrorCallback(int error, const char *description) {
        fprintf(stderr, "Glfw Error %d: %s\n", error, description);
    }

    Window::Window(const WindowProps &props)
            : mWindow(nullptr) { Init(props); }

    Window::~Window() { Shutdown(); }

    Window *Window::Create(const WindowProps &props) { return new Window(props); }

    void Window::Init(const WindowProps &props) {
        mData.Title = props.Title;
        mData.Width = props.Width;
        mData.Height = props.Height;

        RS_CORE_INFO("Creating window: '{0}', {1}x{2}", props.Title.c_str(), props.Width, props.Height);

        if (!sGLFWInitialized) {
            int success = glfwInit();
            RS_CORE_ASSERT(success, "Could not initialize GLFW!");

            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            glfwSetErrorCallback(GlfwErrorCallback);

            sGLFWInitialized = true;
        }

        mWindow = glfwCreateWindow(static_cast<int>(mData.Width),
                                   static_cast<int>(mData.Height),
                                   mData.Title.c_str(), nullptr, nullptr);
        RS_CORE_ASSERT(mWindow, "Failed to create window!");

        glfwMakeContextCurrent(mWindow);
        int status = gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
        RS_CORE_ASSERT(status, "Failed to initialize glad!");
        RS_CORE_INFO("OpenGL info: ");
        RS_CORE_INFO("   Vendor: {0}", glGetString(GL_VENDOR));
        RS_CORE_INFO("   Renderer: {0}", glGetString(GL_RENDERER));
        RS_CORE_INFO("   Version: {0}", glGetString(GL_VERSION));

        SetVSync(true);
        glfwShowWindow(mWindow);
        //
        // // Initialize statics
        // MouseListener::Get();
        // KeyListener::Get();
        //
        // // Set callbacks
        // glfwSetCursorPosCallback(mWindow, MouseListener::MousePosCallback);
        // glfwSetMouseButtonCallback(mWindow, MouseListener::MouseButtonCallback);
        // glfwSetScrollCallback(mWindow, MouseListener::MouseScrollCallback);
        // glfwSetKeyCallback(mWindow, KeyListener::KeyCallback);
    }


    void Window::Shutdown() { glfwDestroyWindow(mWindow); }

    void Window::Update() {
        glfwPollEvents();
        glfwSwapBuffers(mWindow);
    }

    void Window::SetVSync(bool enabled) {
        if (enabled) {
            glfwSwapInterval(1);
        } else {
            glfwSwapInterval(0);
        }

        mData.VSync = enabled;
    }


    bool Window::IsVSync() const { return mData.VSync; }

} // RESANA