#pragma once

#include "Panel.h"

#include "system/processes/ProcessManager.h"
#include "system/processes/ProcessContainer.h"

namespace RESANA {

	class ProcessPanel final : public Panel
	{
	public:
		ProcessPanel();
		~ProcessPanel() override;

		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate(Timestep ts) override;
		void OnImGuiRender() override;

		void ShowPanel(bool* pOpen) override;
		void SetTickRate(Timestep tickRate) override;

		bool IsPanelOpen() const { return mPanelOpen; };

	private:
		void ShowProcessTable();
	private:
		ProcessManager* mProcessManager = nullptr;
		ProcessContainer mDataCache{};
		Timestep mTickRate{};
		bool mPanelOpen = false;
	};

} // RESANA
