#pragma once

#include "View.h"
#include "Log.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

class DebugInfoView : public View {
public:
    DebugInfoView() {}
    void Show() {
        ImGui::SetNextWindowSize(ImVec2{getViewSize().w, getViewSize().h});
        ImGui::SetNextWindowPos(ImVec2{getViewPos().x, getViewPos().y});

        ImGui::Begin("DebugInfoView");

        if (ImGui::Button("Clear")) {
            LOG_INSTANCE("CPU")->clear();
        }
        ImGui::SameLine();
        static bool autoScroll = true;
        ImGui::Checkbox("Auto Scroll", &autoScroll);

        ImGui::BeginChild("LogRegion", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
        for (const auto& log : LOG_INSTANCE("CPU")->get()) {
            ImGui::Text(log.c_str());
        }

        if (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
            ImGui::SetScrollHereY(1.0f);
        }
        ImGui::EndChild();

        ImGui::End();
    }
};