#pragma once

#include <vector>

namespace RESANA {

template <class T, typename Sp = std::shared_ptr<T>>
class LayerStack {
public:
    LayerStack();
    ~LayerStack();

    void PushLayer(Sp layer);
    void PopLayer(Sp layer);

    typename std::vector<Sp>::iterator begin();
    typename std::vector<Sp>::iterator end();

private:
    std::vector<Sp> mLayers;
    unsigned int mLayerInsertIndex = 0;
};

template <class T, typename Sp>
LayerStack<T, Sp>::LayerStack() = default;

template <class T, typename Sp>

LayerStack<T, Sp>::~LayerStack()
{
    mLayers.clear();
}

template <class T, typename Sp>

void LayerStack<T, Sp>::PushLayer(Sp layer)
{
    mLayers.emplace(mLayers.begin() + (int)mLayerInsertIndex, layer);
    ++mLayerInsertIndex;
}

template <class T, typename Sp>

void LayerStack<T, Sp>::PopLayer(Sp layer)
{
    auto it = std::find(mLayers.begin(), mLayers.end(), layer);
    if (it != mLayers.end()) {
        mLayers.erase(it);
        --mLayerInsertIndex;
    }
}

template <class T, typename Sp>

typename std::vector<Sp>::iterator LayerStack<T, Sp>::begin()
{
    return mLayers.begin();
}

template <class T, typename Sp>

typename std::vector<Sp>::iterator LayerStack<T, Sp>::end()
{
    return mLayers.end();
}

} // RESANA
