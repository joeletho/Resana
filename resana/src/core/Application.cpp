#include "Application.h"

#include "imgui.h"

namespace RA {

    Application *Application::sInstance = nullptr;

    Application::Application() {
        // RA_CORE_ASSERT(sInstance, "Application already exists!");
        sInstance = this;
    }

    Application::~Application() {

    }

    void Application::Run() {
        while (true) {
        }
    }

} // RA



// -----------------------------------------------------------------
// [[ENTRY POINT]]
// -----------------------------------------------------------------
int main(int argc, char **argv) {
    auto* app = new RA::Application;
    app->Run();
    delete app;
}