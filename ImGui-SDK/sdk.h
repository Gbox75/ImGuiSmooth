#pragma once

#include "SmoothCalss.h"

namespace imguiAPI {

    namespace button {

        //滑动按钮
        inline bool ToggleButton(const char* label, const char* identifier, bool* state,const ImVec2& size = ImVec2(35, 20),ImU32 activeBg = IM_COL32(100, 200, 255, 255),ImU32 inactiveBg = IM_COL32(100, 100, 100, 255),ImU32 thumbActive = IM_COL32(255, 255, 255, 255),ImU32 thumbInactive = IM_COL32(180, 180, 180, 255),ImU32 textActive = IM_COL32(255, 255, 255, 255),ImU32 textInactive = IM_COL32(150, 150, 150, 255)) {

            SmoothToggleButton* _this = nullptr;

            return _this->Toggle(label, identifier, state, size, activeBg, inactiveBg, thumbActive, thumbInactive, textActive, textInactive);
        }
    }

    namespace Slider {

        //float滑动条
        inline float floatSlider(const char* label, const char* identifier, float* value, float min, float max,const ImVec2& size = ImVec2(300, 5),float thumbWidth = 10.0f,float thumbHeight = 10.0f,float thumbRounding = 20.0f,ImU32 bgCol = IM_COL32(50, 50, 80, 255),ImU32 fillCol = IM_COL32(100, 200, 255, 255),ImU32 thumbCol = IM_COL32(255, 255, 255, 255),ImU32 textCol = IM_COL32(255, 255, 255, 255),ImU32 titleCol = IM_COL32(220, 220, 220, 255)) {

            SmoothSliderfloat* _this = nullptr;

            return _this->Slider(label, identifier, value, min, max, size, thumbWidth, thumbHeight, thumbRounding, bgCol, fillCol, thumbCol, titleCol);
        }

        //int滑动条
        inline int intSlider(const char* label, const char* identifier, int* value, int min, int max,const ImVec2& size = ImVec2(300, 5),float thumbWidth = 10.0f, float thumbHeight = 10.0f, float thumbRounding = 20.0f,ImU32 bgCol = IM_COL32(50, 50, 80, 255),ImU32 fillCol = IM_COL32(100, 200, 255, 255), ImU32 thumbCol = IM_COL32(255, 255, 255, 255), ImU32 textCol = IM_COL32(255, 255, 255, 255),ImU32 titleCol = IM_COL32(220, 220, 220, 255)) {

            SmoothSliderInt* _this = nullptr;

            return _this->Slider(label, identifier, value, min, max, size, thumbWidth, thumbHeight, thumbRounding, bgCol, fillCol, thumbCol, titleCol);
        }
    }

    namespace Input {

        //输入框
        inline bool InputText(const char* label, char* buf, size_t buf_size,const ImVec2& size = ImVec2(200, 30),float rounding = 4.0f,float borderWidth = 1.5f, ImU32 bgCol = IM_COL32(40, 40, 40, 255), ImU32 borderCol = IM_COL32(100, 100, 100, 255),  ImU32 hoverCol = IM_COL32(255, 255, 255, 30),ImU32 focusCol = IM_COL32(100, 150, 255, 150)) {

            SmoothInput* _this = nullptr;

            return _this->InputText(label, buf, buf_size, size, rounding, borderWidth, bgCol, borderCol, hoverCol, focusCol);
        }
    }

    namespace box {
        
        //选择框
        inline bool CheckBox(const char* label, const char* identifier, bool* state,const ImVec2& size = ImVec2(20, 20),ImU32 activeCol = IM_COL32(100, 200, 255, 255),ImU32 inactiveCol = IM_COL32(100, 100, 100, 255), ImU32 borderCol = IM_COL32(200, 200, 200, 255), ImU32 textCol = IM_COL32(255, 255, 255, 255)) {

            SmoothCheckBox* _this = nullptr;

            return _this->Checkbox(label, identifier, state, size, activeCol, inactiveCol, borderCol, textCol);
        }

        //盒子滑动按钮
        inline bool ToggleBox(const char* label, const char* identifier, bool* state,const ImVec2& boxSize = ImVec2(200, 40),const ImVec2& toggleSize = ImVec2(50, 25),ImU32 boxBgCol = IM_COL32(50, 50, 50, 200),ImU32 activeBgCol = IM_COL32(0, 255, 255, 255),ImU32 inactiveBgCol = IM_COL32(100, 100, 100, 255), ImU32 thumbActiveCol = IM_COL32(255, 255, 255, 255),      ImU32 thumbInactiveCol = IM_COL32(180, 180, 180, 255),   ImU32 textCol = IM_COL32(200, 200, 200, 255), ImU32 focusTextCol = IM_COL32(255, 255, 255, 255)) {

            SmoothToggleBox* _this = nullptr;

            return _this->ToggleBox(label, identifier, state, boxSize, toggleSize, boxBgCol, activeBgCol, inactiveBgCol, thumbActiveCol, thumbInactiveCol, textCol, focusTextCol);
        }
    }

    namespace Multi {

        //下拉多选框
        inline bool MultiSelect(const char* identifier, const char* items[], bool selected[], int itemCount, const ImVec2& size = ImVec2(200, 30), int maxVisibleItems = 5, ImU32 bgCol = IM_COL32(50, 50, 50, 255), ImU32 borderCol = IM_COL32(80, 80, 80, 255), ImU32 textCol = IM_COL32(255, 255, 255, 255), ImU32 hoverCol = IM_COL32(70, 70, 70, 255), ImU32 activeCol = IM_COL32(100, 200, 255, 255), ImU32 checkmarkCol = IM_COL32(255, 255, 255, 255), float rounding = 4.0f) {

            SmoothMultiSelectDropdown* _this = nullptr;

            return _this->MultiSelect(identifier, items, selected, itemCount, size, maxVisibleItems, bgCol, borderCol, textCol, hoverCol, activeCol, checkmarkCol, rounding);
        }
    }
}