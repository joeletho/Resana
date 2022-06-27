#include "ResourceAnalyzer.h"

#include <imgui.h>

#include "core/Application.h"
#include "core/Core.h"

namespace RESANA
{
	ResourceAnalyzer::ResourceAnalyzer()
		: mUpdateInterval(TimeTick::Rate::Normal) {}

	ResourceAnalyzer::~ResourceAnalyzer()
	{
		CloseChildren();
	}

	void ResourceAnalyzer::ShowPanel(bool* pOpen)
	{
		// Create window and assign each panel to a tab
		if ((mPanelOpen = *pOpen))
		{
			if (ImGui::Begin("Resource Panel", pOpen))
			{
				RS_CORE_ASSERT(pOpen, "Resource Panel should be open!")

					if (ImGui::Button("Process Details", { 110.0f, 20.0f }))
					{
						mShowProcPanel = true;
						mShowPerfPanel = false;
					}

				ImGui::SameLine();
				if (ImGui::Button("Performance", { 90.0f, 20.0f }))
				{
					mShowPerfPanel = true;
					mShowProcPanel = false;
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

				mProcPanel->ShowPanel(&mShowProcPanel);
				mPerfPanel->ShowPanel(&mShowPerfPanel);
			}
			ImGui::End();
		}
		else {
			Close();
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

		mTimeTick.Start();
	}

	void ResourceAnalyzer::OnDetach()
	{
		mPanelOpen = false;
		mTimeTick.Stop();

		for (Panel* panel : mPanelStack) {
			panel->OnDetach();
		}
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

	void ResourceAnalyzer::Close()
	{
		const auto& app = Application::Get();
		auto& threadPool = app.GetThreadPool();
		threadPool.Queue([&, this] { delete this; });
	}

	void ResourceAnalyzer::CloseChildren()
	{
		ResourceAnalyzer::OnDetach();
	}
}
