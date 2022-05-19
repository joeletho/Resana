#include "rspch.h"
#include "LayerStack.h"

namespace RESANA {

    LayerStack::LayerStack() = default;

    LayerStack::~LayerStack() {
        for (Layer *layer: mLayers) {
            delete layer;
        }
    }

    void LayerStack::PushLayer(Layer *layer) {
        mLayers.emplace(mLayers.begin() + (int) mLayerInsertIndex, layer);
        ++mLayerInsertIndex;
    }

    void LayerStack::PopLayer(Layer *layer) {
        auto it = std::find(mLayers.begin(), mLayers.end(), layer);
        if (it != mLayers.end()) {
            mLayers.erase(it);
            --mLayerInsertIndex;
        }
    }

} // RESANA