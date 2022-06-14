#pragma once

#include "core/Layer.h"

#include "panels/ProcessPanel.h"
#include "panels/ResourcePanel.h"

namespace RESANA
{

	class ExampleLayer final : public Layer
	{
	public:
		ExampleLayer();
		~ExampleLayer() override;

		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate() override;
		void OnImGuiRender() override;

	private:
		ProcessPanel* mProcessPanel = nullptr;
		ResourcePanel* mResourcePanel = nullptr;
	};

}
