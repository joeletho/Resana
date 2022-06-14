#pragma once

#include "Window.h"
#include "Layer.h"
#include "imgui/ImGuiLayer.h"
#include "LayerStack.h"

#include <memory>

namespace RESANA {

	class Application {
	public:
		Application();
		virtual ~Application();

		void PushLayer(Layer* layer);
		void Run();

		void Terminate();
		bool IsMinimized() const;

		[[nodiscard]] Window& GetWindow() const { return *mWindow; }

		static Application& Get() { return *sInstance; }

	private:
		std::shared_ptr<Window> mWindow;
		ImGuiLayer* mImGuiLayer;
		LayerStack mLayerStack;
		bool mRunning = true;
		bool mMinimized = false;

		static Application* sInstance;
	};

	// To be defined by CLIENT
	Application* CreateApplication();

} // RESANA


