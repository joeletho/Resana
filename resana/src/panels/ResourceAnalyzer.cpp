#include "ResourceAnalyzer.h"

#include <imgui.h>

#include "core/Core.h"

namespace RESANA
{
	ResourceAnalyzer::ResourceAnalyzer() = default;

	ResourceAnalyzer::~ResourceAnalyzer()
	{
		for (Panel* panel : mPanelStack)
		{
			panel->OnDetach();
		}
		mTimeTick.Stop();

	}

	void ResourceAnalyzer::ShowPanel(bool* pOpen)
	{
		static bool showThisPanel = true;
		static bool showPerfPanel = false;
		static bool showProcPanel = false;

		// Create window and assign each panel to a tab
		if (ImGui::Begin("Resource Analyzer", &showThisPanel))
		{
			if (ImGui::Button("Details", { 50.0f, 20.0f }))
			{
				showProcPanel = true;
				showPerfPanel = false;
			}

			ImGui::SameLine();
			if (ImGui::Button("Performance", { 90.0f, 20.0f }))
			{
				showPerfPanel = true;
				showProcPanel = false;
			}

			ImGui::SameLine();
			static std::string label = "Normal";
			static int i = 1;
			if (ImGui::Button(label.c_str(), { 90.0f, 20.0f }))
			{
				if (i == 0)
				{
					mTickRate = Slow;
					label = "Slow";
				}
				else if (i == 1)
				{
					mTickRate = Normal;
					label = "Normal";
				}
				else if (i == 2)
				{
					mTickRate = Fast;
					label = "Fast";
				}
				else
				{
					RS_CORE_ASSERT((i < 0 || i > 2), "Unknown update speed! {0}");
				}
				i = (i + 1) % 3;
			}

			mProcPanel->ShowPanel(&showProcPanel);
			mPerfPanel->ShowPanel(&showPerfPanel);

			ImGui::End();
		}
	}

	void ResourceAnalyzer::UpdatePanels(RefreshRate rate)
	{
		if (const auto tick = mTimeTick.GetTickCount(); tick > mLastTick) {
			for (Panel* panel : mPanelStack)
			{
				panel->OnUpdate(rate);
			}
			mLastTick = tick;
		}
	}

	void ResourceAnalyzer::OnAttach()
	{
		mPerfPanel = new PerformancePanel();
		mPerfPanel->OnAttach();
		mPanelStack.PushLayer(mPerfPanel);

		mProcPanel = new ProcessPanel();
		mProcPanel->OnAttach();
		mPanelStack.PushLayer(mProcPanel);
	}

	void ResourceAnalyzer::OnDetach()
	{
		for (Panel* panel : mPanelStack)
		{
			panel->OnDetach();
		}
		mTimeTick.Stop();
	}

	void ResourceAnalyzer::OnUpdate(Timestep ts)
	{
		static int called_n = 0;
		// Update the panels
		UpdatePanels(mTickRate);
	}

	void ResourceAnalyzer::OnImGuiRender()
	{

	}

}
