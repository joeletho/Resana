#pragma once

namespace RA {

    class Application {
    public:
        Application();
        ~Application();

        void Run();

    private:
        static Application* sInstance;
    };

} // RA

