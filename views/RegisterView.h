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
        static uint64_t prevRegs[31] = {0};        // 上一帧寄存器值
        static int highlightTimer[31] = {0};       // 每个寄存器的高亮计时器（帧数）
        constexpr int kHighlightFrames = 70;       // 高亮持续的帧数

        ImGui::SetNextWindowSize(ImVec2{getViewSize().w, getViewSize().h});
        ImGui::SetNextWindowPos(ImVec2{getViewPos().x, getViewPos().y});

        ImGui::Begin("Register View");
        ImGui::BeginChild("RegisterRegion", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);

        CPU& cpu = CPU::GetInstance();

        ImGui::Text("PC:  0x%016llX", cpu.getPC());
        ImGui::Text("SP:  0x%016llX", cpu.getSP());
        ImGui::Text("IR:  0x%08X", cpu.getIR());
        ImGui::Text("Status: %s", cpu.getStatusReg().toString().c_str());

        if (ImGui::BeginTable("RegisterTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit)) {
            ImGui::TableSetupColumn("Register");
            ImGui::TableSetupColumn("Value");
            ImGui::TableHeadersRow();

            for (int i = 0; i < 31; ++i) {
                uint64_t value = cpu.getReg(i);

                // 寄存器值发生变化，重置计时器
                if (value != prevRegs[i]) {
                    highlightTimer[i] = kHighlightFrames;
                    prevRegs[i] = value;
                }

                ImGui::TableNextRow();

                // 在高亮时间内，设置背景色
                if (highlightTimer[i] > 0) {
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, IM_COL32(255, 0, 0, 170));
                    highlightTimer[i]--;
                }

                ImGui::TableSetColumnIndex(0);
                ImGui::Text("X%02d", i);
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("0x%016llX", value);
            }

            ImGui::EndTable();
        }

        ImGui::EndChild();
        ImGui::End();
    }
};