#pragma once

#include <vector>

namespace RESANA {

	template<class T>
	class LayerStack {
	public:
		LayerStack();
		~LayerStack();

		void PushLayer(T* layer);
		void PopLayer(T* layer);

		typename std::vector<T*>::iterator begin();

		typename std::vector<T*>::iterator end();

	private:
		std::vector<T*> mLayers;
		unsigned int mLayerInsertIndex = 0;
	};

	template <class T>
	LayerStack<T>::LayerStack()
	{
	}

	template <class T>
	LayerStack<T>::~LayerStack()
	{
		for (const T* layer : mLayers) {
			delete layer;
			layer = nullptr;
		}
	}

	template <class T>
	void LayerStack<T>::PushLayer(T* layer)
	{
		mLayers.emplace(mLayers.begin() + (int)mLayerInsertIndex, layer);
		++mLayerInsertIndex;
	}

	template <class T>
	void LayerStack<T>::PopLayer(T* layer)
	{
		auto it = std::find(mLayers.begin(), mLayers.end(), layer);
		if (it != mLayers.end()) {
			mLayers.erase(it);
			--mLayerInsertIndex;
		}
	}

	template <class T>
	typename std::vector<T*>::iterator LayerStack<T>::begin()
	{
		return mLayers.begin();
	}

	template <class T>
	typename std::vector<T*>::iterator LayerStack<T>::end()
	{
		return mLayers.end();
	}

} // RESANA
