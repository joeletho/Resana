#include "Sandbox.h"

#include "core/Application.h"
#include "core/EntryPoint.h" // For CreateApplication()

#include <imgui.h>


namespace RESANA {

	ExampleLayer::ExampleLayer()
		: Layer("ExampleLayer")
	{
	}

	ExampleLayer::~ExampleLayer()
	{
		ExampleLayer::OnDetach();
	};

	void ExampleLayer::OnAttach()
	{
		mResanaPanel = new ResourceAnalyzer();
		mResanaPanel->OnAttach();
	}

	void ExampleLayer::OnDetach()
	{
		if (mResanaPanel) {
			mShowResanaPanel = false;
			mResanaPanel->OnDetach();
		}
	}

	void ExampleLayer::OnUpdate(Timestep ts)
	{
		if (mShowResanaPanel) {
			mResanaPanel->OnUpdate(ts);
		}
	}

	void ExampleLayer::OnImGuiRender()
	{
		static bool showDemo = false;

		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Quit")) {
					Application::Get().Terminate();
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View"))
			{
				ImGui::MenuItem("Resource Analyzer", nullptr, &mShowResanaPanel);
				ImGui::MenuItem("ImGui Demo", nullptr, &showDemo);
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		if (showDemo) {
			ImGui::ShowDemoWindow(&showDemo);
		}

		ShowResanaPanel();

	}

	void ExampleLayer::ShowResanaPanel()
	{
		if (!mResanaPanel && mShowResanaPanel) {
			ExampleLayer::OnAttach();
		}
		else if (mResanaPanel) {
			mResanaPanel->ShowPanel(&mShowResanaPanel);
			if (!mShowResanaPanel) {
				mResanaPanel = nullptr; // Panel was just closed and destroyed
			}
		}
	}

	class Sandbox final : public Application {
	public:
		Sandbox() {
			PushLayer(new ExampleLayer());
		}

		~Sandbox() override = default;
	};

	// ----------------------------------------------------
	// [ENTRY POINT]
	// ----------------------------------------------------
	Application* CreateApplication() {
		return new Sandbox();
	}

}

