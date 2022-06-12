#pragma once

#include "Panel.h"

#include "system/ProcessManager.h"

namespace RESANA {

	class ProcessPanel : public Panel
	{
	public:
		ProcessPanel();
		~ProcessPanel() override = default;

		void ShowPanel(bool* pOpen) override;

	private:
		void ShowProcessTable();

	private:
		ProcessManager* mProcessManager = nullptr;
	};

} // RESANA
