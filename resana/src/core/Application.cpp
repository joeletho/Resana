#include "rspch.h"

#include "Application.h"
#include "Renderer.h"
#include "Core.h"
#include "Log.h"

#include "perfdata/PerfManager.h"
#include "system/ThreadPool.h"

namespace RESANA {

	Application* Application::sInstance = nullptr;

	Application::Application()
	{
		RS_CORE_ASSERT(!sInstance, "Application already exists!");
		sInstance = this;
		mRunning = true;

		mWindow = std::unique_ptr<Window>(Window::Create());

		// Start statics
		PerfManager::Init();
		Renderer::Init();

		mThreadPool.reset(new ThreadPool);
		mThreadPool->Start();

		mImGuiLayer = new ImGuiLayer();
		PushLayer(mImGuiLayer);
	}

	Application::~Application()
	{
		for (Layer* layer : mLayerStack)
		{
			layer->OnDetach();
		}

		mThreadPool->Stop();
	}


	void Application::PushLayer(Layer* layer)
	{
		mLayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::Run()
	{
		Timestep ts;

		while (mRunning)
		{
			// Check if window has been closed
			mRunning = !glfwWindowShouldClose((GLFWwindow*)mWindow->GetNativeWindow());

			if (!IsMinimized())
			{
				for (Layer* layer : mLayerStack) {
					layer->OnUpdate(ts);
				}

				ImGuiLayer::Begin();
				for (Layer* layer : mLayerStack) {
					layer->OnImGuiRender();
				}
				ImGuiLayer::End();
			}

			mWindow->Update();
		}

		// Clean up
		glfwDestroyWindow((GLFWwindow*)mWindow->GetNativeWindow());
		glfwTerminate();
		glfwSetErrorCallback(nullptr);
	}

	void Application::Terminate()
	{
		mRunning = false;
	}

	bool Application::IsMinimized() const
	{
		return mWindow->GetWidth() == 0 || mWindow->GetHeight() == 0;
	}

} // RESANA
