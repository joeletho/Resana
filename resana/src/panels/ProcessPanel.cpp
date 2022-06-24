#include "rspch.h"
#include "ProcessPanel.h"

#include "core/Application.h"

#include "imgui/ImGuiHelpers.h"
#include <imgui.h>

#include <mutex>

namespace RESANA {

	ProcessPanel::ProcessPanel()
	{
		mProcessManager = ProcessManager::Get();
	}

	ProcessPanel::~ProcessPanel()
	{
		ProcessManager::Stop();
	}

	void ProcessPanel::OnAttach()
	{
		ProcessManager::Run();
		mTickRate = 1000;
		mProcessManager->SetUpdateSpeed(mTickRate);
	}

	void ProcessPanel::OnDetach()
	{
		ProcessManager::Stop();
	}

	void ProcessPanel::OnUpdate(Timestep ts)
	{
		mTickRate = ts;
		mProcessManager->SetUpdateSpeed(mTickRate);

		auto& app = Application::Get();
		auto& threadPool = app.GetThreadPool();
		threadPool.Queue([&]
			{
				const auto& data = mProcessManager->GetData();

				if (data) {
					static std::mutex mutex;
					// Be sure to let the original data know what entry is selected (if any) before
					//	reset mDataCache. If the new data has that entry, it will be selected from
					//	within data and then copied to mDataCache.
					data->SelectEntry(mDataCache.GetSelectedEntry(), true);
					mDataCache.Copy(data.get());
				}
				mProcessManager->ReleaseData();
			});
	}

	void ProcessPanel::OnImGuiRender()
	{
	}

	void ProcessPanel::ShowPanel(bool* pOpen)
	{
		if (*pOpen)
		{
			if (ImGui::BeginChild("Details", ImGui::GetContentRegionAvail())) {
				ProcessManager::Run();

				ShowProcessTable();
			}
			ImGui::EndChild();

		}
		else {
			//ProcessManager::Stop();

		}
	}

	void ProcessPanel::SetTickRate(Timestep tickRate)
	{
		mTickRate = tickRate;
	}

	void ProcessPanel::ShowProcessTable()
	{
		static bool showProcessID = true;
		static bool showParentProcessID = false;
		static bool showModuleID = false;
		static bool showMemoryUsage = true;
		static bool showThreadCount = true;
		static bool showPriorityClass = false;


		ImGui::PushStyleColor(ImGuiCol_Text, { 0.0f,0.0f,0.0f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_TableHeaderBg, { 1.0f,1.0f,1.0f,1.0f });
		ImGui::PushStyleColor(ImGuiCol_TableBorderStrong, { 1.0f,1.0f,1.0f,1.0f });
		ImGui::PushStyleColor(ImGuiCol_TableBorderLight, { 1.0f,1.0f,1.0f,1.0f });

		if (ImGui::BeginMenu("View"))
		{
			ImGui::Checkbox("Process ID", &showProcessID);
			ImGui::Checkbox("Parent Process ID", &showParentProcessID);
			ImGui::Checkbox("Module ID", &showModuleID);
			ImGui::Checkbox("Memory Usage", &showMemoryUsage);
			ImGui::Checkbox("Thread Count", &showThreadCount);
			ImGui::Checkbox("Priority Class", &showPriorityClass);
			ImGui::EndMenu();
		}

		constexpr auto numOptions = 6;
		const bool viewOptions[] = {
			showProcessID,
			showParentProcessID,
			showModuleID,
			showMemoryUsage,
			showThreadCount,
			showPriorityClass
		};

		int numColumns = 1;
		for (const bool viewOption : viewOptions) {
			numColumns += viewOption ? 1 : 0;
		}

		static ImGuiTableFlags flags = ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Borders |
			ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_NoSavedSettings;

		const auto outerSize = ImVec2(-1.0f, ImGui::GetContentRegionAvail().y);

		if (ImGui::BeginTable("proc_table", numColumns, flags, outerSize))
		{
			static bool showColWidthRow;
			showColWidthRow = true;

			constexpr int freezeCols = 0, freezeRows = 1;

			ImGui::TableSetupScrollFreeze(freezeCols, freezeRows);
			ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoReorder, 160.0f);

			if (showProcessID) {
				ImGui::TableSetupColumn("PID", ImGuiTableColumnFlags_WidthFixed, 50.0f);
			}
			if (showParentProcessID) {
				ImGui::TableSetupColumn("PPID", ImGuiTableColumnFlags_WidthFixed, 50.0f);
			}
			if (showModuleID) {
				ImGui::TableSetupColumn("Module", ImGuiTableColumnFlags_WidthFixed, 50.0f);
			}
			if (showMemoryUsage) {
				ImGui::TableSetupColumn("Memory", ImGuiTableColumnFlags_WidthFixed);
			}
			if (showPriorityClass) {
				ImGui::TableSetupColumn("Priority", ImGuiTableColumnFlags_WidthFixed);
			}
			if (showThreadCount) {
				ImGui::TableSetupColumn("Threads");
			}

			ImGui::TableHeadersRow();

			// Lock the data and read the entries
			std::scoped_lock slock(mDataCache.GetMutex());

			for (const auto& entry : mDataCache.GetEntries())
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();

				/* [DEBUG]
				 * Show width of each column in the first row. */
				if (showColWidthRow) {
					ImGui::Text("%.2f", ImGui::GetContentRegionAvail().x);
					ImGui::TableNextColumn();
					ImGui::Text("%.2f", ImGui::GetContentRegionAvail().x);
					ImGui::TableNextColumn();
					ImGui::Text("%.2f", ImGui::GetContentRegionAvail().x);
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					showColWidthRow = false;	// Only called once (first row)
				}

				{
					// Lock the entry
					std::scoped_lock slock(entry->Mutex());

					static char uniqueId[64];
					sprintf_s(uniqueId, "##%lu", entry->GetProcessId());

					if (ImGui::Selectable(entry->GetName().c_str(), entry->IsSelected(),
						ImGuiSelectableFlags_SpanAllColumns, ImGui::GetColumnWidth(-1), uniqueId))
					{
						mDataCache.SelectEntry(entry);
					}

					if (showProcessID)
					{
						ImGui::TableNextColumn();
						ImGui::Text("%lu", entry->GetProcessId());

					}
					if (showParentProcessID)
					{
						ImGui::TableNextColumn();
						ImGui::Text("%lu", entry->GetParentProcessId());

					}
					if (showModuleID)
					{
						ImGui::TableNextColumn();
						ImGui::Text("%lu", entry->GetModuleId());

					}
					if (showMemoryUsage)
					{
						ImGui::TableNextColumn();
						ImGui::Text("%lu", entry->GetMemoryUsage());

					}
					if (showPriorityClass)
					{
						ImGui::TableNextColumn();
						ImGui::Text("%lu", entry->GetPriorityClass());

					}
					if (showThreadCount)
					{
						ImGui::TableNextColumn();
						ImGui::Text("%lu", entry->GetThreadCount());
					}
				}
			}
			ImGui::EndTable();
		}
		ImGui::PopStyleColor(4);
	}

} // RESANA