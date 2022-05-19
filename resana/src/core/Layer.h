#pragma once

#include "Core.h"

namespace RESANA {

    class Layer {
    public:
        explicit Layer(std::string name = "Layer");
        virtual ~Layer() = default;

        virtual void OnAttach() {}

        virtual void OnDetach() {}

        virtual void OnUpdate() {}

        virtual void OnImGuiRender() {}

        [[nodiscard]] inline const std::string &GetName() const { return mDebugName; }

    private:
        std::string mDebugName;
    };

} // RESANA

