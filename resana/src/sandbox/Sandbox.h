#pragma once

#include "core/Layer.h"

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

		void ShowResanaPanel();

	private:
		ResourceAnalyzer* mResanaPanel = nullptr;
		bool mShowResanaPanel = false;
	};

}
