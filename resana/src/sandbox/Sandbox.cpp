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
		delete mResanaPanel;
	};

	void ExampleLayer::OnAttach()
	{
		mResanaPanel = new ResourceAnalyzer();
		mResanaPanel->OnAttach();
		//mPerformancePanel = new PerformancePanel();
		//mProcessPanel = new ProcessPanel();
	}

	void ExampleLayer::OnDetach()
	{
		mResanaPanel->OnDetach();
	}

	void ExampleLayer::OnUpdate(Timestep ts)
	{
		mResanaPanel->OnUpdate(ts);
	}

	void ExampleLayer::OnImGuiRender()
	{
		static bool showDemo = false;
		static bool showResanaPanel = true;
		static bool showResourcePanel = false;
		static bool showProcessPanel = false;

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
				//ImGui::MenuItem("Resource Manager", nullptr, &showResourcePanel);
				//ImGui::MenuItem("Process", nullptr, &showProcessPanel);
				ImGui::MenuItem("Resource Analyzer", nullptr, &showResanaPanel);
				ImGui::MenuItem("ImGui Demo", nullptr, &showDemo);
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		if (showDemo) {
			ImGui::ShowDemoWindow(&showDemo);
		}

		mResanaPanel->ShowPanel(&showResanaPanel);
		//mPerformancePanel->ShowPanel(&showResourcePanel);
		//mProcessPanel->ShowPanel(&showProcessPanel);

		/* DEBUG ONLY */
		


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

