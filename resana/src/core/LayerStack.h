#pragma once

#include "Core.h"

#include "Layer.h"

namespace RESANA {

    class LayerStack {
    public:
        LayerStack();
        ~LayerStack();

        void PushLayer(Layer *layer);
        void PopLayer(Layer *layer);

        std::vector<Layer *>::iterator begin() { return mLayers.begin(); }

        std::vector<Layer *>::iterator end() { return mLayers.end(); }

    private:
        std::vector<Layer *> mLayers;
        unsigned int mLayerInsertIndex = 0;
    };

} // RESANA
