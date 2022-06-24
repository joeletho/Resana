#pragma once

#include "helpers/Time.h"

#include <string>

namespace RESANA {

    class Layer {
    public:
        explicit Layer(std::string name = "Layer");
        virtual ~Layer() = default;

        virtual void OnAttach() {}

        virtual void OnDetach() {}

        virtual void OnUpdate(Timestep ts) {}

        virtual void OnImGuiRender() {}

        [[nodiscard]] const std::string &GetName() const { return mDebugName; }

    private:
        std::string mDebugName;
    };

} // RESANA

