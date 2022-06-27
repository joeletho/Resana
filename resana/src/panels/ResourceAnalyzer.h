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

		void UpdatePanels(Timestep interval = TimeTick::Rate::Normal);
		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate(Timestep ts = 0) override;
		void OnImGuiRender() override;

		bool IsPanelOpen() const override { return mPanelOpen; };
		
	private:
		void Close();
		void CloseChildren();

		ProcessPanel* mProcPanel = nullptr;
		PerformancePanel* mPerfPanel = nullptr;
		LayerStack<Panel> mPanelStack{};

		bool mPanelOpen{};
		bool mShowProcPanel = false;
		bool mShowPerfPanel = false;

		uint32_t mUpdateInterval{};
		Timestep mLastTick{};
		TimeTick mTimeTick{};
	};

}
