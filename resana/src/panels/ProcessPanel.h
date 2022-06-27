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
		void SetUpdateInterval(Timestep interval) override;

		bool IsPanelOpen() const override { return mPanelOpen; };

	private:
		void ShowProcessTable();
	private:
		ProcessManager* mProcessManager = nullptr;
		ProcessContainer mDataCache{};
		bool mPanelOpen = false;
		uint32_t mUpdateInterval{};
	};

} // RESANA
