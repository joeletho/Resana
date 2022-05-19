#pragma once

#include <imgui.h>

namespace RESANA {

    class Renderer {
    public:
        static void Init();
        static void SetClearColor(ImVec4 color);
        static void Clear();

        static void GetError();
    };

} // RESANA
