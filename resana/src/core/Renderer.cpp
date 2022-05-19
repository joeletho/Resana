#include "Renderer.h"

#include <glad/glad.h>

namespace RESANA {

    void Renderer::Init() {
        // Enable blending
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void Renderer::SetClearColor(ImVec4 color) {
        glClearColor(color.x, color.y, color.z, color.w);
    }

    void Renderer::Clear() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void Renderer::GetError() {
        GLenum e = glGetError();
        if (e > 0) {
            RS_CORE_ERROR("OpenGL error: {0}", e);
        }
    }

} // RESANA