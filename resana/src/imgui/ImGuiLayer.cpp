#include "rspch.h"
#include "ImGuiLayer.h"

#include "core/Application.h"
#include "core/Renderer.h"

#include "imgui/ImGuiBuild.h"

#include <imgui.h>

#include "perfdata/PerfManager.h"

namespace RESANA {

    ImGuiLayer::ImGuiLayer()
            : Layer("ImGuiLayer") {}

    ImGuiLayer::~ImGuiLayer() = default;

    void ImGuiLayer::OnAttach() {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void) io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;        // Enable Keyboard controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;            // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;          // Enable Multi-viewport / Platform windows

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();

        // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to the style
        ImGuiStyle &style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        Application &app = Application::Get();
        auto *window = (GLFWwindow *) (app.GetWindow().GetNativeWindow());

        // Setup Platform/Renderer bindings
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 460 core");
    }

    void ImGuiLayer::OnDetach() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void ImGuiLayer::Begin() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void ImGuiLayer::End() {
        ImGuiIO &io = ImGui::GetIO();
        Application &app = Application::Get();
        io.DisplaySize = ImVec2((float) app.GetWindow().GetWidth(), (float) app.GetWindow().GetHeight());

        // Rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow *backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
    }

    void ImGuiLayer::ShowImGuiDockspace() {
        static bool opt_fullscreen = true;
        static bool opt_padding = false;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // because it would be confusing to have two docking targets within each others.
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        if (opt_fullscreen) {
            const ImGuiViewport *viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                            ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        } else {
            dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
        }

        // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
        // and handle the pass-thru hole, so we ask Begin() to not render a background.
        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) {
            window_flags |= ImGuiWindowFlags_NoBackground;
        }

        // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
        // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
        // all active windows docked into it will lose their parent and become undocked.
        // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
        // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
        if (!opt_padding) {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        }
        ImGui::Begin("DockSpace Demo", nullptr, window_flags);
        if (!opt_padding) {
            ImGui::PopStyleVar();
        }

        if (opt_fullscreen) {
            ImGui::PopStyleVar(2);
        }

        // Submit the DockSpace
        ImGuiIO &io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }

        ImGui::End();
    }

    void ImGuiLayer::OnImGuiRender() {
        ShowImGuiDockspace();

        static bool show = true;
        ImGui::ShowDemoWindow(&show);

        static bool show_sys_res_win = true;
        if (ImGui::Begin("System Resources", &show_sys_res_win)) {
            auto *memInfo = PerfManager::GetMemory();
            auto *cpuInfo = PerfManager::GetCPU();

            ImGui::TextUnformatted("Memory");
            if (ImGui::BeginTable("##Physical Memory", 2, ImGuiTableFlags_Borders)) {

                ImGui::TableSetupColumn("Physical");
                ImGui::TableSetupColumn("##values");
                ImGui::TableHeadersRow();

                ImGui::TableNextColumn();
                // TODO: Resize columns based on size
                ImGui::Text("Total");
                ImGui::Text("In use");
                ImGui::Text("Available");
                ImGui::Text("Used by process");
                ImGui::TableNextColumn();

                auto total_mem = memInfo->GetTotalPhys() / BYTES_PER_MB;
                auto used_mem = memInfo->GetUsedPhys() / BYTES_PER_MB;
                auto avail_mem = memInfo->GetAvailPhys() / BYTES_PER_MB;
                auto proc_mem = memInfo->GetCurrProcUsagePhys() / BYTES_PER_MB;

                ImGui::Text("%llu.%llu GB", total_mem / 1000, total_mem % 10);
                ImGui::Text("%llu.%llu GB", used_mem / 1000, used_mem % 10);
                ImGui::Text("%llu.%llu GB", avail_mem / 1000, avail_mem % 10);
                ImGui::Text("%lu MB", proc_mem);
                ImGui::EndTable();
            }

            if (ImGui::BeginTable("##Virtual Memory", 2, ImGuiTableFlags_Borders)) {

                ImGui::TableSetupColumn("Virtual");
                ImGui::TableSetupColumn("##values");
                ImGui::TableHeadersRow();

                ImGui::TableNextColumn();
                ImGui::Text("Total");
                ImGui::Text("In use");
                ImGui::Text("Available");
                ImGui::Text("Used by process");
                ImGui::TableNextColumn();

                auto total_mem = memInfo->GetTotalVirtual() / BYTES_PER_MB;
                auto used_mem = memInfo->GetUsedVirtual() / BYTES_PER_MB;
                auto avail_mem = memInfo->GetAvailVirtual() / BYTES_PER_MB;
                auto proc_mem = memInfo->GetCurrProcUsageVirtual() / BYTES_PER_MB;

                ImGui::Text("%llu.%llu GB", total_mem / 1000, total_mem % 10);
                ImGui::Text("%llu.%llu GB", used_mem / 1000, used_mem % 10);
                ImGui::Text("%llu.%llu GB", avail_mem / 1000, avail_mem % 10);
                ImGui::Text("%lu MB", proc_mem);
                ImGui::EndTable();
            }
            if (ImGui::BeginTable("##CPU", 2, ImGuiTableFlags_Borders)) {
                ImGui::TableSetupColumn("CPU");
                ImGui::TableSetupColumn("##values");
                ImGui::TableHeadersRow();

                ImGui::TableNextColumn();

                static auto processorLoads = cpuInfo->GetCurrentLoadAll();
                {
                    for (auto const &processor: processorLoads) {
                        ImGui::Text("cpu %d", processor.first);

                    }
                    ImGui::Text("%% Total");
                    // ImGui::Text("%% In use");
                    ImGui::Text("%% Used by process");
                    ImGui::TableNextColumn();
                }

                static double currLoad{}, procLoad{};
                static unsigned int counter = 0;
                if (counter == 50) {
                    counter = 0;
                    processorLoads = cpuInfo->GetCurrentLoadAll();
                    currLoad = cpuInfo->GetCurrentLoad();
                    procLoad = cpuInfo->GetCurrentLoadProc();
                }
                ++counter;

                // Display values for all logical processors
                {
                    for (auto const &processor: processorLoads) {
                        ImGui::Text("%.1f", processor.second);
                    }

                    // Display current CPU load and load in use by process
                    ImGui::Text("%.1f", currLoad);
                    ImGui::Text("%.1f", procLoad);
                }
                ImGui::EndTable();
            }
        }
        ImGui::End();

        // static bool show_bkg_col_win = false;
        // if (ImGui::Begin("Background Color", &show_bkg_col_win)) {
        //     static float color[4] = {0.2f, 0.2f, 0.2f, 1.0f};
        //     {
        //         ImGui::SliderFloat4("Color", color, 0, 1.0f);
        //         // ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(color[0], color[1], color[2], color[3]));
        //     }
        // }
        // ImGui::End();
    }

} // RESANA