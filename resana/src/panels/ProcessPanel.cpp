#include "ProcessPanel.h"

#include <imgui.h>

namespace RESANA {

	ProcessPanel::ProcessPanel()
	{
		mProcessManager = ProcessManager::Get();
	}

	void ProcessPanel::ShowPanel(bool* pOpen)
	{
		if (*pOpen)
		{
			if (ImGui::Begin("Process", pOpen))
			{
				ProcessManager::Run();
				ShowProcessTable();
			}
			ImGui::End();

		}
		else {
			ProcessManager::Stop();
		}
	}

	void ProcessPanel::ShowProcessTable()
	{
		if (ImGui::BeginTable("proc table", 3,
			ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings))
		{
			mProcessManager = ProcessManager::Get();
			if (const auto data = mProcessManager->GetData()) // Mutex is locked
			{
				for (const auto entry : data->Entries)
				{
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					ImGui::Selectable(entry->Process.szExeFile, &entry->flag, ImGuiSelectableFlags_SpanAllColumns);
					ImGui::TableNextColumn();
					ImGui::Text("%lu", entry->Process.th32ProcessID);
					ImGui::TableNextColumn();
					ImGui::Text("%lu", entry->Process.cntThreads);
				}

				mProcessManager->ReleaseData(); // Must unlock mutex!
			}

			ImGui::EndTable();
		}
	}

} // RESANA