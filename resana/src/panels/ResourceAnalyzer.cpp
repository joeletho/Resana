#include "ResourceAnalyzer.h"

#include <imgui.h>

#include "core/Core.h"

namespace RESANA
{
	ResourceAnalyzer::ResourceAnalyzer()
		: mUpdateInterval(TimeTick::Rate::Normal) {}

	ResourceAnalyzer::~ResourceAnalyzer()
	{
	}

	void ResourceAnalyzer::ShowPanel(bool* pOpen)
	{
		static bool showPerfPanel = false;
		static bool showProcPanel = false;

		// Create window and assign each panel to a tab
		if ((mPanelOpen = *pOpen))
		{
			if (ImGui::Begin("Resource Panel", pOpen))
			{
				RS_CORE_ASSERT(pOpen, "Resource Panel should be open!")

					if (ImGui::Button("Process Details", { 110.0f, 20.0f }))
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
						mUpdateInterval = TimeTick::Slow;
						label = "Slow";
					}
					else if (i == 1)
					{
						mUpdateInterval = TimeTick::Normal;
						label = "Normal";
					}
					else if (i == 2)
					{
						mUpdateInterval = TimeTick::Fast;
						label = "Fast";
					}

					i = (i + 1) % 3;
				}

				mProcPanel->ShowPanel(&showProcPanel);
				mPerfPanel->ShowPanel(&showPerfPanel);
			}
			ImGui::End();
		}
		else
		{
			showPerfPanel = false;
			showProcPanel = false;
			this->OnDetach();
		}
	}
	void ResourceAnalyzer::UpdatePanels(Timestep interval)
	{
		if (mPanelOpen)
		{
			if (const auto tick = mTimeTick.GetTickCount(); tick > mLastTick) {
				for (Panel* panel : mPanelStack)
				{
					if (panel->IsPanelOpen()) {
						panel->OnUpdate(interval);
					}
				}
				mLastTick = tick;
			}
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
		if (mPanelOpen)
		{
			// Update the panels
			UpdatePanels(mUpdateInterval);
		}
	}

	void ResourceAnalyzer::OnImGuiRender()
	{

	}

	void ResourceAnalyzer::CloseChildren()
	{
		OnDetach();
	}
}
