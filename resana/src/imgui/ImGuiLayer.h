#pragma once

#include "core/Layer.h"

#include "panels/ResourcePanel.h"
#include "panels/ProcessPanel.h"

namespace RESANA
{
	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer() override;

		static void Begin();
		static void End();

		void OnAttach() override;
		void OnDetach() override;
		void OnImGuiRender() override;

	private:
		static void ShowImGuiDockspace();

	private:
		float mTime = 0.0f;

		ResourcePanel* mResourcePanel = nullptr;
		ProcessPanel* mProcessPanel = nullptr;
	};

} // RESANA

