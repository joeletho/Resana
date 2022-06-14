#include "Sandbox.h"

#include "core/Application.h"
#include "core/EntryPoint.h" // For CreateApplication()

#include <imgui.h>

namespace RESANA {

	ExampleLayer::ExampleLayer()
		: Layer("ExampleLayer")
	{
	}

	ExampleLayer::~ExampleLayer() = default;

	void ExampleLayer::OnAttach()
	{
		mResourcePanel = new ResourcePanel();
		mProcessPanel = new ProcessPanel();
	}

	void ExampleLayer::OnDetach()
	{
	}

	void ExampleLayer::OnUpdate()
	{
	}

	void ExampleLayer::OnImGuiRender()
	{
		static bool showDemo = false;
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
				ImGui::MenuItem("Resource Manager", nullptr, &showResourcePanel);
				ImGui::MenuItem("Process", nullptr, &showProcessPanel);
				ImGui::MenuItem("ImGui Demo", nullptr, &showDemo);
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		if (showDemo) {
			ImGui::ShowDemoWindow(&showDemo);
		}

		mResourcePanel->ShowPanel(&showResourcePanel);
		mProcessPanel->ShowPanel(&showProcessPanel);
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

