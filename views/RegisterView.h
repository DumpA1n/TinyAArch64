#pragma once

#include "View.h"
#include "Log.h"
#include "CPU.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

class RegisterView : public View {
public:
    RegisterView() {}
    void Show() {
        ImGui::SetNextWindowSize(ImVec2{getViewSize().w, getViewSize().h});
        ImGui::SetNextWindowPos(ImVec2{getViewPos().x, getViewPos().y});

        ImGui::Begin("Register View");
        ImGui::BeginChild("RegisterRegion", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);

        CPU& cpu = CPU::GetInstance();

        // PC / SP / IR / 状态寄存器
        ImGui::Text("PC:  0x%016llX", cpu.getPC());
        ImGui::Text("SP:  0x%016llX", cpu.getSP());
        ImGui::Text("IR:  0x%08X", cpu.getIR());
        ImGui::Text("Status: %s", cpu.getStatusReg().toString().c_str());

        // ImGui::Separator();

        if (ImGui::BeginTable("RegisterTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit)) {
            ImGui::TableSetupColumn("Register");
            ImGui::TableSetupColumn("Value");
            ImGui::TableHeadersRow();  // 可选：表头

            for (int i = 0; i < 31; ++i) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("X%02d", i);
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("0x%016llX", cpu.getReg(i));
            }

            ImGui::EndTable();
        }
        
        ImGui::EndChild();
        ImGui::End();
    }
};