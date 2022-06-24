#pragma once

#include "Panel.h"
#include "PerformancePanel.h"
#include "ProcessPanel.h"

#include "core/LayerStack.h"

#include "helpers/Time.h"

namespace RESANA
{

	class ResourceAnalyzer : public Panel
	{
	public:
		ResourceAnalyzer();
		~ResourceAnalyzer() override;

		void ShowPanel(bool* pOpen) override;

		void UpdatePanels(RefreshRate rate = Normal);
		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate(Timestep ts = 0) override;
		void OnImGuiRender() override;

	private:
		PerformancePanel* mPerfPanel = nullptr;
		ProcessPanel* mProcPanel = nullptr;
		LayerStack<Panel> mPanelStack{};

		RefreshRate mTickRate = Normal;
		Timestep mLastTick{};
		TimeTick mTimeTick{};
	};

}
