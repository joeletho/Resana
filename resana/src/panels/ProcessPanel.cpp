#include "ProcessPanel.h"

#include "imgui/ImGuiHelpers.h"
#include <imgui.h>

#include <mutex>

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
			if (const auto& data = mProcessManager->GetData()) // Notify data is in use
			{
				// Lock the data and read the entries
				std::scoped_lock slock(data->GetMutex());

				for (const auto& entry : data->GetEntries())
				{
					ImGui::TableNextRow();
					ImGui::TableNextColumn();

					{
						// Lock the entry
						std::scoped_lock slock(entry->Mutex());

						static char uniqueId[64];
						sprintf_s(uniqueId, "##%lu", entry->GetProcessId());

						if (ImGui::Selectable(entry->GetName().c_str(), entry->IsSelected(),
							ImGuiSelectableFlags_SpanAllColumns, ImGui::GetColumnWidth(1), uniqueId))
						{
							data->SelectEntry(entry);
						}

						ImGui::TableNextColumn();
						ImGui::Text("%lu", entry->GetProcessId());
						ImGui::TableNextColumn();
						ImGui::Text("%lu", entry->GetThreadCount());
					}
				}

				mProcessManager->ReleaseData(); // Notify data is no longer needed
			}

			ImGui::EndTable();
		}
	}

} // RESANA