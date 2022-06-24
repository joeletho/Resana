#pragma once

#include "core/Layer.h"

#include "panels/PerformancePanel.h"
#include "panels/ProcessPanel.h"

#include "helpers/Time.h"
#include "panels/ResourceAnalyzer.h"

namespace RESANA
{

	class ExampleLayer final : public Layer
	{
	public:
		ExampleLayer();
		~ExampleLayer() override;

		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate(Timestep ts) override;
		void OnImGuiRender() override;

	private:
		ResourceAnalyzer* mResanaPanel = nullptr;
		//ProcessPanel* mProcessPanel = nullptr;
		//PerformancePanel* mPerformancePanel = nullptr;
	};

}
