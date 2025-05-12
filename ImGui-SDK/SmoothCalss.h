#pragma once

#include "../imconfig.h"
#include "../imgui.h"
#include "../imgui_impl_dx11.h"
#include "../imgui_impl_win32.h"
#include "../imgui_internal.h"
#include "../imstb_rectpack.h"
#include "../imstb_textedit.h"
#include "../imstb_truetype.h"

#include <d3d11.h>
#pragma comment(lib,"d3d11.lib")

#include <cmath>
#include <unordered_map>
#include <string>
#include <unordered_set>

class SmoothToggleButton {
private:
    // 开关状态
    float switchPos = 0.0f;
    bool currentState = false;
    bool isAnimating = false;

    // 悬停状态
    float hoverAlpha = 0.0f;

    // 动画参数
    float switchSpeed = 5.0f;
    float hoverSpeed = 10.0f;

    // 样式参数
    ImVec2 size = ImVec2(50, 25);
    float rounding = 12.0f;
    float thumbSizeRatio = 0.8f;
    float thumbPadding = 4.0f;
    float hoverBrightness = 0.2f;

    ImU32 activeBgColor = IM_COL32(0, 255, 255, 255);
    ImU32 inactiveBgColor = IM_COL32(100, 100, 100, 255);
    ImU32 thumbActiveColor = IM_COL32(255, 255, 255, 255);
    ImU32 thumbInactiveColor = IM_COL32(180, 180, 180, 255);
    ImU32 textActiveColor = IM_COL32(255, 255, 255, 255);
    ImU32 textInactiveColor = IM_COL32(150, 150, 150, 255);

    // 缓动效果
    float SmoothStep(float t) {
        return t * t * (3.0f - 2.0f * t);
    }

    // 绘制实现
    bool DrawImpl(const char* label, const char* identifier, bool state) {
        ImGui::PushID(identifier);

        if (state != currentState) {
            currentState = state;
            isAnimating = true;
        }

        // 计算布局
        bool hasLabel = label && label[0] != '\0';
        float textWidth = hasLabel ? ImGui::CalcTextSize(label).x + ImGui::GetStyle().ItemInnerSpacing.x : 0.0f;
        ImVec2 p = ImGui::GetCursorScreenPos();
        ImVec2 totalSize(size.x + textWidth, ImMax(size.y, ImGui::GetTextLineHeight()));

        // 交互区域
        bool clicked = ImGui::InvisibleButton(identifier, totalSize);
        bool isHovered = ImGui::IsItemHovered();

        // 更新动画
        float delta = ImGui::GetIO().DeltaTime;
        float switchTarget = currentState ? 1.0f : 0.0f;
        switchPos = ImLerp(switchPos, switchTarget, delta * switchSpeed);
        if (fabs(switchPos - switchTarget) < 0.001f) isAnimating = false;

        float hoverTarget = isHovered ? 1.0f : 0.0f;
        hoverAlpha = ImLerp(hoverAlpha, hoverTarget, delta * hoverSpeed);

        // 计算颜色
        auto ApplyHover = [this](ImU32 color, float alpha) {
            if (isAnimating || alpha <= 0.0f) return color;
            ImVec4 c = ImGui::ColorConvertU32ToFloat4(color);
            float factor = hoverBrightness * SmoothStep(alpha);
            c.x = ImMin(c.x + factor, 1.0f);
            c.y = ImMin(c.y + factor, 1.0f);
            c.z = ImMin(c.z + factor, 1.0f);
            return ImGui::ColorConvertFloat4ToU32(c);
            };

        ImU32 bgColor = currentState ? activeBgColor : inactiveBgColor;
        ImU32 thumbColor = currentState ? thumbActiveColor : thumbInactiveColor;
        ImU32 textColor = currentState ? textActiveColor : textInactiveColor;

        bgColor = ApplyHover(bgColor, hoverAlpha);
        if (!currentState) textColor = ApplyHover(textColor, hoverAlpha);

        // 绘制
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddRectFilled(p, ImVec2(p.x + size.x, p.y + size.y), bgColor, rounding);

        // 滑块
        float thumbSize = size.y * thumbSizeRatio;
        float maxOffset = size.x - thumbSize - thumbPadding * 2;
        float thumbX = p.x + thumbPadding + maxOffset * switchPos;
        drawList->AddCircleFilled(ImVec2(thumbX + thumbSize / 2, p.y + size.y / 2),
            thumbSize / 2, thumbColor, 32);

        // 文本
        if (hasLabel) {
            ImVec2 textPos(p.x + size.x + ImGui::GetStyle().ItemInnerSpacing.x,
                p.y + (size.y - ImGui::GetTextLineHeight()) / 2);
            drawList->AddText(textPos, textColor, label);
        }

        if (clicked) currentState = !currentState;
        ImGui::PopID();
        return clicked;
    }

public:
    // 静态接口 - 模仿ImGui风格，使用独立的label和identifier
    static bool Toggle(
        const char* label, //按钮的标签文本，如果传入空字符则不绘制
        const char* identifier,  //按钮的唯一标识符，用于区分不同的按钮
        bool* state,  //指向一个布尔值的指针，表示按钮的当前状态（true 表示开启，false 表示关闭）
        const ImVec2& size = ImVec2(35, 20),  //按钮的大小，以像素为单位
        ImU32 activeBg = IM_COL32(0, 255, 255, 255),  //按钮处于开启状态时的背景颜色
        ImU32 inactiveBg = IM_COL32(100, 100, 100, 255), //按钮处于关闭状态时的背景颜色
        ImU32 thumbActive = IM_COL32(255, 255, 255, 255),  //按钮处于开启状态时的滑块颜色
        ImU32 thumbInactive = IM_COL32(180, 180, 180, 255),  //按钮处于关闭状态时的滑块颜色
        ImU32 textActive = IM_COL32(255, 255, 255, 255),  //按钮处于开启状态时的文本颜色
        ImU32 textInactive = IM_COL32(150, 150, 150, 255))  //按钮处于关闭状态时的文本颜色
    {

        std::string key = identifier ? identifier : "";
        static std::unordered_map<std::string, SmoothToggleButton> instances;
        auto& instance = instances[key];

        // 首次调用时初始化
        static std::unordered_set<std::string> initialized;
        if (initialized.find(key) == initialized.end()) {
            instance.size = size;
            instance.activeBgColor = activeBg;
            instance.inactiveBgColor = inactiveBg;
            instance.thumbActiveColor = thumbActive;
            instance.thumbInactiveColor = thumbInactive;
            instance.textActiveColor = textActive;
            instance.textInactiveColor = textInactive;
            initialized.insert(key);
        }

        bool clicked = instance.DrawImpl(label, identifier, *state);
        if (clicked) *state = instance.currentState;
        return clicked;
    }
}; //滑动按钮

class SmoothSliderfloat {
private:
    // 滑动值
    float currentValue = 0.0f;
    float displayValue = 0.0f;
    bool isDragging = false;
    bool isHovered = false;
    bool hasFocus = false;
    float fadeAlpha = 1.0f;
    float focusLostTime = 0.0f;
    const float fadeDelay = 3.0f;

    // 动画参数
    float animationSpeed = 10.0f;
    float hoverIntensity = 0.0f;
    float hoverAlpha = 0.0f;
    bool isActive = false;

    // 样式参数
    ImVec2 size = ImVec2(200, 20);
    float thumbWidth = 12.0f;
    float thumbHeight = 20.0f;
    float rounding = 4.0f;
    float thumbRounding = 4.0f;
    float textOffsetX = 5.0f;
    float textOffsetY = 2.0f;

    // 颜色配置
    ImU32 bgColor = IM_COL32(70, 70, 70, 255);
    ImU32 fillColor = IM_COL32(255, 182, 193, 255);
    ImU32 thumbColor = IM_COL32(220, 220, 220, 255);
    ImU32 hoverColor = IM_COL32(255, 255, 255, 50);
    ImU32 textColor = IM_COL32(255, 255, 255, 255);
    ImU32 titleColor = IM_COL32(220, 220, 220, 255);

    // 缓动函数
    float EaseOut(float t) {
        return 1.0f - (1.0f - t) * (1.0f - t);
    }

    // 绘制实现
    float DrawImpl(const char* label, const char* identifier, float min, float max, float value) {
        ImGui::PushID(identifier);

        // 更新当前值
        currentValue = value;

        // 计算布局
        ImGui::BeginGroup();

        // 绘制标签（如果有） - 固定在滑动条上方
        bool hasLabel = label && label[0] != '\0';
        if (hasLabel) {
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(titleColor), "%s", label);
        }

        // 滑动条主体
        ImVec2 p = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton("##slider", size);

        // 交互处理
        isActive = ImGui::IsItemActive();
        isHovered = ImGui::IsItemHovered();
        bool wasHasFocus = hasFocus;
        hasFocus = isActive || isHovered;

        ImVec2 thumbMin = ImVec2(p.x + (size.x - thumbWidth) * ((value - min) / (max - min)), p.y - (thumbHeight - size.y) / 2);
        ImVec2 thumbMax = ImVec2(thumbMin.x + thumbWidth, thumbMin.y + thumbHeight);

        bool isThumbHovered = ImGui::IsMouseHoveringRect(thumbMin, thumbMax);

        if ((isActive && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) || (isThumbHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))) {
            float mouseX = ImGui::GetMousePos().x - p.x;
            currentValue = (mouseX / size.x) * (max - min) + min;
            currentValue = ImClamp(currentValue, min, max);
            isDragging = true;
        }
        else if (isDragging && !ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            isDragging = false;
        }

        // 更新动画
        float delta = ImGui::GetIO().DeltaTime;
        displayValue = ImLerp(displayValue, currentValue, delta * animationSpeed);

        // 更新悬停效果
        float hoverTarget = isHovered || isActive ? 1.0f : 0.0f;
        hoverAlpha = ImLerp(hoverAlpha, hoverTarget, delta * 15.0f);

        // 更新文本透明度
        float fadeTarget = 0.0f;

        if (hasFocus) {
            fadeTarget = 1.0f;
        }
        else {
            if (wasHasFocus && !hasFocus) {
                focusLostTime = ImGui::GetTime();
            }

            if (ImGui::GetTime() - focusLostTime < fadeDelay) {
                fadeTarget = 1.0f;
            }
        }

        fadeAlpha = ImLerp(fadeAlpha, fadeTarget, delta * 15.0f);

        // 计算绘制参数
        float fillRatio = (displayValue - min) / (max - min);
        thumbMin = ImVec2(p.x + fillRatio * (size.x - thumbWidth), p.y - (thumbHeight - size.y) / 2);
        thumbMax = ImVec2(thumbMin.x + thumbWidth, thumbMin.y + thumbHeight);

        // 绘制背景
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        // 背景
        drawList->AddRectFilled(
            p,
            ImVec2(p.x + size.x, p.y + size.y),
            bgColor,
            rounding
        );

        // 填充部分
        drawList->AddRectFilled(
            p,
            ImVec2(p.x + fillRatio * size.x, p.y + size.y),
            fillColor,
            rounding
        );

        // 悬停效果
        if (hoverAlpha > 0.01f) {
            ImVec4 hoverCol = ImGui::ColorConvertU32ToFloat4(hoverColor);
            hoverCol.w *= hoverAlpha;
            drawList->AddRectFilled(
                p,
                ImVec2(p.x + size.x, p.y + size.y),
                ImGui::GetColorU32(hoverCol),
                rounding
            );
        }

        // 滑块（拇指）
        drawList->AddRectFilled(
            thumbMin,
            thumbMax,
            thumbColor,
            thumbRounding,
            ImDrawFlags_RoundCornersAll
        );

        // 在滑块右下角绘制当前值
        char valueText[32];
        snprintf(valueText, sizeof(valueText), "%.2f", displayValue);
        ImVec2 textSize = ImGui::CalcTextSize(valueText);

        // 计算文本位置（跟随滑块移动）
        ImVec2 textPos = ImVec2(thumbMax.x + textOffsetX, thumbMax.y + textOffsetY);

        // 获取窗口边界信息
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 windowSize = ImGui::GetWindowSize();

        // 确保文本不会超出窗口右边界
        if (textPos.x + textSize.x > windowPos.x + windowSize.x) {
            textPos.x = thumbMin.x - textSize.x - textOffsetX;
        }

        // 确保文本不会超出窗口底部边界
        if (textPos.y + textSize.y > windowPos.y + windowSize.y) {
            textPos.y = thumbMin.y - textSize.y - textOffsetY;
        }

        // 绘制文本
        ImVec4 textCol = ImGui::ColorConvertU32ToFloat4(textColor);
        textCol.w *= fadeAlpha;
        drawList->AddText(
            textPos,
            ImGui::GetColorU32(textCol),
            valueText
        );

        // ===== 仅添加的间距控制代码 =====
        float textBottom = textPos.y + textSize.y;
        float currentBottom = ImGui::GetCursorScreenPos().y;
        if (textBottom > currentBottom) {
            ImGui::Dummy(ImVec2(0, textBottom - currentBottom));
        }
        // ==============================

        ImGui::EndGroup();
        ImGui::PopID();
        return currentValue;
    }

public:
    // 静态接口
    static float Slider(
        const char* label,                       // 滑动条的标签文本，显示在滑动条旁边或滑动条上
        const char* identifier,                  // 滑动条的唯一标识符，用于区分不同的滑动条
        float* value,                            // 指向滑动条当前值的指针
        float min,                                 // 滑动条的最小值
        float max,                                 // 滑动条的最大值
        const ImVec2& size = ImVec2(300, 5),       // 滑动条的大小，默认宽度为300像素，高度为5像素
        float thumbWidth = 10.0f,                  // 滑块的宽度，默认为10像素
        float thumbHeight = 10.0f,                 // 滑块的高度，默认为10像素
        float thumbRounding = 20.0f,               // 滑块的圆角半径，默认为20像素
        ImU32 bgCol = IM_COL32(50, 50, 80, 255),   // 滑动条背景颜色，默认为深蓝色
        ImU32 fillCol = IM_COL32(100, 200, 255, 255), // 滑动条填充颜色，默认为亮蓝色
        ImU32 thumbCol = IM_COL32(255, 255, 255, 255),// 滑块颜色，默认为白色
        ImU32 textCol = IM_COL32(255, 255, 255, 255), // 文本颜色，默认为白色
        ImU32 titleCol = IM_COL32(220, 220, 220, 255) // 标题颜色，默认为浅灰色
    ) {

        std::string key = identifier ? identifier : "";
        static std::unordered_map<std::string, SmoothSliderfloat> instances;
        auto& instance = instances[key];

        static std::unordered_set<std::string> initialized;
        if (initialized.find(key) == initialized.end()) {
            instance.size = size;
            instance.thumbWidth = thumbWidth;
            instance.thumbHeight = thumbHeight;
            instance.thumbRounding = thumbRounding;
            instance.bgColor = bgCol;
            instance.fillColor = fillCol;
            instance.thumbColor = thumbCol;
            instance.textColor = textCol;
            instance.titleColor = titleCol;
            initialized.insert(key);
        }

        float newValue = instance.DrawImpl(label, identifier, min, max, *value);
        if (newValue != *value) {
            *value = newValue;
        }

        return newValue;
    }
};  //float滑动条

class SmoothSliderInt {
private:
    // 滑动值
    int currentValue = 0;
    float displayValue = 0.0f; // 用于平滑显示的浮点值
    bool isDragging = false;
    bool isHovered = false;
    bool hasFocus = false;
    float fadeAlpha = 1.0f;
    float focusLostTime = 0.0f;
    const float fadeDelay = 3.0f;

    // 动画参数
    float animationSpeed = 10.0f;
    float hoverIntensity = 0.0f;
    float hoverAlpha = 0.0f;
    bool isActive = false;

    // 样式参数
    ImVec2 size = ImVec2(200, 20);
    float thumbWidth = 12.0f;
    float thumbHeight = 20.0f;
    float rounding = 4.0f;
    float thumbRounding = 4.0f;
    float textOffsetX = 5.0f;
    float textOffsetY = 2.0f;

    // 颜色配置
    ImU32 bgColor = IM_COL32(70, 70, 70, 255);
    ImU32 fillColor = IM_COL32(255, 182, 193, 255);
    ImU32 thumbColor = IM_COL32(220, 220, 220, 255);
    ImU32 hoverColor = IM_COL32(255, 255, 255, 50);
    ImU32 textColor = IM_COL32(255, 255, 255, 255);
    ImU32 titleColor = IM_COL32(220, 220, 220, 255);

    // 缓动函数
    float EaseOut(float t) {
        return 1.0f - (1.0f - t) * (1.0f - t);
    }

    // 绘制实现
    int DrawImpl(const char* label, const char* identifier, int min, int max, int value) {
        ImGui::PushID(identifier);

        // 更新当前值
        currentValue = value;

        // 计算布局
        ImGui::BeginGroup();

        // 绘制标签（如果有） - 固定在滑动条上方
        bool hasLabel = label && label[0] != '\0';
        if (hasLabel) {
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(titleColor), "%s", label);
        }

        // 滑动条主体
        ImVec2 p = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton("##slider", size);

        // 交互处理
        isActive = ImGui::IsItemActive();
        isHovered = ImGui::IsItemHovered();
        bool wasHasFocus = hasFocus;
        hasFocus = isActive || isHovered;

        ImVec2 thumbMin = ImVec2(p.x + (size.x - thumbWidth) * ((value - min) / (float)(max - min)), p.y - (thumbHeight - size.y) / 2);
        ImVec2 thumbMax = ImVec2(thumbMin.x + thumbWidth, thumbMin.y + thumbHeight);

        bool isThumbHovered = ImGui::IsMouseHoveringRect(thumbMin, thumbMax);

        if ((isActive && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) || (isThumbHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))) {
            float mouseX = ImGui::GetMousePos().x - p.x;
            currentValue = (int)((mouseX / size.x) * (max - min) + min);
            currentValue = ImClamp(currentValue, min, max);
            isDragging = true;
        }
        else if (isDragging && !ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            isDragging = false;
        }

        // 更新动画
        float delta = ImGui::GetIO().DeltaTime;
        displayValue = ImLerp(displayValue, (float)currentValue, delta * animationSpeed);

        // 更新悬停效果
        float hoverTarget = isHovered || isActive ? 1.0f : 0.0f;
        hoverAlpha = ImLerp(hoverAlpha, hoverTarget, delta * 15.0f);

        // 更新文本透明度
        float fadeTarget = 0.0f;

        if (hasFocus) {
            fadeTarget = 1.0f;
        }
        else {
            if (wasHasFocus && !hasFocus) {
                focusLostTime = ImGui::GetTime();
            }

            if (ImGui::GetTime() - focusLostTime < fadeDelay) {
                fadeTarget = 1.0f;
            }
        }

        fadeAlpha = ImLerp(fadeAlpha, fadeTarget, delta * 15.0f);

        // 计算绘制参数
        float fillRatio = (displayValue - min) / (max - min);
        thumbMin = ImVec2(p.x + fillRatio * (size.x - thumbWidth), p.y - (thumbHeight - size.y) / 2);
        thumbMax = ImVec2(thumbMin.x + thumbWidth, thumbMin.y + thumbHeight);

        // 绘制背景
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        // 背景
        drawList->AddRectFilled(
            p,
            ImVec2(p.x + size.x, p.y + size.y),
            bgColor,
            rounding
        );

        // 填充部分
        drawList->AddRectFilled(
            p,
            ImVec2(p.x + fillRatio * size.x, p.y + size.y),
            fillColor,
            rounding
        );

        // 悬停效果
        if (hoverAlpha > 0.01f) {
            ImVec4 hoverCol = ImGui::ColorConvertU32ToFloat4(hoverColor);
            hoverCol.w *= hoverAlpha;
            drawList->AddRectFilled(
                p,
                ImVec2(p.x + size.x, p.y + size.y),
                ImGui::GetColorU32(hoverCol),
                rounding
            );
        }

        // 滑块（拇指）
        drawList->AddRectFilled(
            thumbMin,
            thumbMax,
            thumbColor,
            thumbRounding,
            ImDrawFlags_RoundCornersAll
        );

        // 在滑块右下角绘制当前值
        char valueText[32];
        snprintf(valueText, sizeof(valueText), "%d", (int)displayValue);
        ImVec2 textSize = ImGui::CalcTextSize(valueText);

        // 计算文本位置（跟随滑块移动）
        ImVec2 textPos = ImVec2(thumbMax.x + textOffsetX, thumbMax.y + textOffsetY);

        // 获取窗口边界信息
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 windowSize = ImGui::GetWindowSize();

        // 确保文本不会超出窗口右边界
        if (textPos.x + textSize.x > windowPos.x + windowSize.x) {
            textPos.x = thumbMin.x - textSize.x - textOffsetX;
        }

        // 确保文本不会超出窗口底部边界
        if (textPos.y + textSize.y > windowPos.y + windowSize.y) {
            textPos.y = thumbMin.y - textSize.y - textOffsetY;
        }

        // 绘制文本
        ImVec4 textCol = ImGui::ColorConvertU32ToFloat4(textColor);
        textCol.w *= fadeAlpha;
        drawList->AddText(
            textPos,
            ImGui::GetColorU32(textCol),
            valueText
        );

        // ===== 仅添加的间距控制代码 =====
        float textBottom = textPos.y + textSize.y;
        float currentBottom = ImGui::GetCursorScreenPos().y;
        if (textBottom > currentBottom) {
            ImGui::Dummy(ImVec2(0, textBottom - currentBottom + 5.0f));
        }
        // ==============================

        ImGui::EndGroup();
        ImGui::PopID();
        return currentValue;
    }

public:
    // 静态接口
    static int Slider(
        const char* label,                     // 滑动条的标签文本，显示在滑动条旁边或滑动条上
        const char* identifier,                // 滑动条的唯一标识符，用于区分不同的滑动条
        int* value,                          // 指向滑动条当前值的指针
        int min,                             // 滑动条的最小值
        int max,                             // 滑动条的最大值
        const ImVec2& size = ImVec2(300, 5),   // 滑动条的大小，默认宽度为300像素，高度为5像素
        float thumbWidth = 10.0f,              // 滑块的宽度，默认为10像素
        float thumbHeight = 10.0f,             // 滑块的高度，默认为10像素
        float thumbRounding = 20.0f,           // 滑块的圆角半径，默认为20像素
        ImU32 bgCol = IM_COL32(50, 50, 80, 255),// 滑动条背景颜色，默认为深蓝色
        ImU32 fillCol = IM_COL32(100, 200, 255, 255),// 滑动条填充颜色，默认为亮蓝色
        ImU32 thumbCol = IM_COL32(255, 255, 255, 255),// 滑块颜色，默认为白色
        ImU32 textCol = IM_COL32(255, 255, 255, 255), // 文本颜色，默认为白色
        ImU32 titleCol = IM_COL32(220, 220, 220, 255) // 标题颜色，默认为浅灰色
    ) {

        std::string key = identifier ? identifier : "";
        static std::unordered_map<std::string, SmoothSliderInt> instances;
        auto& instance = instances[key];

        static std::unordered_set<std::string> initialized;
        if (initialized.find(key) == initialized.end()) {
            instance.size = size;
            instance.thumbWidth = thumbWidth;
            instance.thumbHeight = thumbHeight;
            instance.thumbRounding = thumbRounding;
            instance.bgColor = bgCol;
            instance.fillColor = fillCol;
            instance.thumbColor = thumbCol;
            instance.textColor = textCol;
            instance.titleColor = titleCol;
            initialized.insert(key);
        }

        int newValue = instance.DrawImpl(label, identifier, min, max, *value);
        if (newValue != *value) {
            *value = newValue;
        }

        return newValue;
    }
};  //int滑动条    

class SmoothInput {
private:
    // 状态变量
    float hoverAlpha = 0.0f;
    float focusAlpha = 0.0f;
    bool hasFocus = false;

    // 样式参数
    ImVec2 size = ImVec2(200, 30);
    float rounding = 4.0f;
    float borderWidth = 1.5f;

    // 颜色配置
    ImU32 bgColor = IM_COL32(40, 40, 40, 255);
    ImU32 borderColor = IM_COL32(100, 100, 100, 255);
    ImU32 hoverColor = IM_COL32(255, 255, 255, 30);
    ImU32 focusColor = IM_COL32(100, 150, 255, 150);

    // 核心绘制实现
    bool DrawImpl(const char* label, char* buf, size_t buf_size) {
        std::string thisId = label ? std::string(label) : "##input";
        ImGui::PushID(thisId.c_str());

        // 1. 绘制背景和交互区域
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton("##input_area", size);

        bool hovered = ImGui::IsItemHovered();
        bool clicked = ImGui::IsItemClicked();

        // 2. 焦点管理
        if (clicked) {
            hasFocus = true;
        }

        // 检查是否失去焦点
        if (hasFocus && !ImGui::IsItemActive() && ImGui::IsMouseClicked(0)) {
            hasFocus = false;
        }

        bool isActive = hasFocus;

        // 3. 更新动画状态
        float delta = ImGui::GetIO().DeltaTime;
        hoverAlpha = ImLerp(hoverAlpha, hovered ? 1.0f : 0.0f, delta * 10.0f);
        focusAlpha = ImLerp(focusAlpha, isActive ? 1.0f : 0.0f, delta * 15.0f);

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), bgColor, rounding);

        ImGui::SetCursorScreenPos(pos);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, (size.y - ImGui::GetTextLineHeight()) * 0.5f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, rounding);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));

        ImGui::PushItemWidth(size.x);
        bool changed = false;

        if (isActive) {
            ImGui::SetKeyboardFocusHere();
        }
        changed = ImGui::InputText(
            "##real_input",
            buf,
            buf_size,
            ImGuiInputTextFlags_None
        );

        ImGui::PopItemWidth();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar(2);

        if (hoverAlpha > 0.01f) {
            drawList->AddRectFilled(
                pos, ImVec2(pos.x + size.x, pos.y + size.y),
                IM_COL32(255, 255, 255, (int)(30 * hoverAlpha)),
                rounding
            );
        }

        if (focusAlpha > 0.01f) {
            float width = borderWidth * (1.0f + focusAlpha * 0.5f);
            drawList->AddRect(
                ImVec2(pos.x - width * 0.5f, pos.y - width * 0.5f),
                ImVec2(pos.x + size.x + width * 0.5f, pos.y + size.y + width * 0.5f),
                focusColor, rounding, 0, width
            );
        }

        drawList->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), borderColor, rounding, 0, borderWidth);

        ImGui::PopID();
        return changed;
    }

public:
    static bool InputText(
        const char* label,                     // 输入框的标签文本，显示在输入框旁边或输入框上
        char* buf,                             // 指向输入框内容的字符数组
        size_t buf_size,                       // 输入框内容的缓冲区大小
        const ImVec2& size = ImVec2(200, 30),  // 输入框的大小，默认宽度为200像素，高度为30像素
        float rounding = 4.0f,                 // 输入框的圆角半径，默认为4像素
        float borderWidth = 1.5f,              // 输入框边框的宽度，默认为1.5像素
        ImU32 bgCol = IM_COL32(40, 40, 40, 255),// 输入框背景颜色，默认为深灰色
        ImU32 borderCol = IM_COL32(100, 100, 100, 255),// 输入框边框颜色，默认为灰色
        ImU32 hoverCol = IM_COL32(255, 255, 255, 30),// 输入框悬停时的背景颜色，默认为浅灰色
        ImU32 focusCol = IM_COL32(100, 150, 255, 150) // 输入框获得焦点时的背景颜色，默认为亮蓝色
    ) {

        // 使用哈希值作为索引，为每个输入框创建独立的实例
        static std::unordered_map<std::string, SmoothInput> instances;
        std::string id = label ? std::string(label) : "##input";

        SmoothInput& instance = instances[id];
        instance.size = size;
        instance.rounding = rounding;
        instance.borderWidth = borderWidth;
        instance.bgColor = bgCol;
        instance.borderColor = borderCol;
        instance.hoverColor = hoverCol;
        instance.focusColor = focusCol;

        return instance.DrawImpl(label, buf, buf_size);
    }
};//输入框

class SmoothCheckBox {
private:
    // 动画状态
    float checkAnim = 0.0f;      // 选中动画进度(0.0到1.0)
    bool currentState = false;   // 当前复选框状态
    bool isAnimating = false;    // 是否正在播放动画

    // 悬停效果
    float hoverAlpha = 0.0f;     // 悬停强度(0.0到1.0)

    // 动画速度
    float animSpeed = 8.0f;      // 选中动画速度
    float hoverSpeed = 12.0f;    // 悬停动画速度

    // 样式参数
    ImVec2 size = ImVec2(20, 20); // 默认复选框大小
    float rounding = 4.0f;        // 边框圆角
    float borderThickness = 2.0f; // 边框厚度
    float hoverBrightness = 0.2f; // 悬停亮度增加值

    // 颜色设置
    ImU32 activeColor = IM_COL32(100, 200, 255, 255);   // 选中状态颜色
    ImU32 inactiveColor = IM_COL32(100, 100, 100, 255); // 未选中状态颜色
    ImU32 borderColor = IM_COL32(200, 200, 200, 255);   // 边框颜色
    ImU32 textColor = IM_COL32(255, 255, 255, 255);     // 文本颜色

    // 颜色插值函数
    ImU32 ImLerpColor(ImU32 col1, ImU32 col2, float t) {
        ImVec4 c1 = ImGui::ColorConvertU32ToFloat4(col1);
        ImVec4 c2 = ImGui::ColorConvertU32ToFloat4(col2);
        return ImGui::ColorConvertFloat4ToU32(ImVec4(
            c1.x + (c2.x - c1.x) * t,
            c1.y + (c2.y - c1.y) * t,
            c1.z + (c2.z - c1.z) * t,
            c1.w + (c2.w - c1.w) * t
        ));
    }

    // 缓动函数
    float SmoothStep(float t) {
        return t * t * (3.0f - 2.0f * t);
    }

    // 应用悬停效果
    ImU32 ApplyHover(ImU32 color, float alpha) {
        if (alpha <= 0.0f) return color;
        ImVec4 c = ImGui::ColorConvertU32ToFloat4(color);
        float factor = hoverBrightness * SmoothStep(alpha);
        c.x = ImMin(c.x + factor, 1.0f);
        c.y = ImMin(c.y + factor, 1.0f);
        c.z = ImMin(c.z + factor, 1.0f);
        return ImGui::ColorConvertFloat4ToU32(c);
    }

    // 绘制实现
    bool DrawImpl(const char* label, const char* identifier, bool state) {
        ImGui::PushID(identifier);

        // 状态变化时触发动画
        if (state != currentState) {
            currentState = state;
            isAnimating = true;
        }

        // 计算布局
        bool hasLabel = label && label[0] != '\0';
        float textWidth = hasLabel ? ImGui::CalcTextSize(label).x + ImGui::GetStyle().ItemInnerSpacing.x : 0.0f;
        ImVec2 p = ImGui::GetCursorScreenPos();
        ImVec2 totalSize(size.x + textWidth, ImMax(size.y, ImGui::GetTextLineHeight()));

        // 交互区域
        bool clicked = ImGui::InvisibleButton(identifier, totalSize);
        bool isHovered = ImGui::IsItemHovered();

        // 更新动画
        float delta = ImGui::GetIO().DeltaTime;
        float animTarget = currentState ? 1.0f : 0.0f;
        checkAnim = ImLerp(checkAnim, animTarget, delta * animSpeed);
        if (fabs(checkAnim - animTarget) < 0.001f) isAnimating = false;

        float hoverTarget = isHovered ? 1.0f : 0.0f;
        hoverAlpha = ImLerp(hoverAlpha, hoverTarget, delta * hoverSpeed);

        // 计算颜色
        ImU32 bgColor = ImLerpColor(inactiveColor, activeColor, checkAnim);
        bgColor = ApplyHover(bgColor, hoverAlpha);

        // 绘制
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        // 绘制背景(带动画颜色)
        drawList->AddRectFilled(p, ImVec2(p.x + size.x, p.y + size.y), bgColor, rounding);

        // 绘制边框
        drawList->AddRect(p, ImVec2(p.x + size.x, p.y + size.y), borderColor, rounding, 0, borderThickness);

        // 绘制文本
        if (hasLabel) {
            ImVec2 textPos(p.x + size.x + ImGui::GetStyle().ItemInnerSpacing.x,
                p.y + (size.y - ImGui::GetTextLineHeight()) * 0.5f);
            drawList->AddText(textPos, textColor, label);
        }

        if (clicked) currentState = !currentState;
        ImGui::PopID();
        return clicked;
    }

public:
    // 静态接口 - 模仿ImGui风格，将标题和ID分开
    static bool Checkbox(
        const char* label,                     // 复选框的标签文本，显示在复选框旁边
        const char* identifier,                // 复选框的唯一标识符，用于区分不同的复选框
        bool* state,                           // 指向复选框当前状态的指针（true 表示选中，false 表示未选中）
        const ImVec2& size = ImVec2(20, 20),   // 复选框的大小，默认为 20x20 像素
        ImU32 activeCol = IM_COL32(100, 200, 255, 255),// 复选框选中时的颜色，默认为亮蓝色
        ImU32 inactiveCol = IM_COL32(100, 100, 100, 255),// 复选框未选中时的颜色，默认为灰色
        ImU32 borderCol = IM_COL32(200, 200, 200, 255),// 复选框边框的颜色，默认为浅灰色
        ImU32 textCol = IM_COL32(255, 255, 255, 255)   // 标签文本的颜色，默认为白色
    ) {

        std::string key = identifier ? identifier : "";
        static std::unordered_map<std::string, SmoothCheckBox> instances;
        auto& instance = instances[key];

        // 首次调用时初始化
        static std::unordered_set<std::string> initialized;
        if (initialized.find(key) == initialized.end()) {
            instance.size = size;
            instance.activeColor = activeCol;
            instance.inactiveColor = inactiveCol;
            instance.borderColor = borderCol;
            instance.textColor = textCol;
            initialized.insert(key);
        }

        bool clicked = instance.DrawImpl(label, identifier, *state);
        if (clicked) *state = instance.currentState;
        return clicked;
    }
};//选择框

class SmoothToggleBox {
private:
    // 滑动按钮状态
    float switchPos = 0.0f;
    bool currentState = false;
    bool isAnimating = false;

    // 悬停和焦点状态
    float hoverAlpha = 0.0f;
    float focusAnim = 0.0f;
    bool isHovered = false;
    bool isFocused = false;
    bool wasFocused = false;
    bool wasHovered = false;

    // 动画参数
    float switchSpeed = 5.0f;
    float hoverSpeed = 10.0f;
    float focusSpeed = 8.0f;

    // 样式参数
    ImVec2 boxSize = ImVec2(200, 40);
    ImVec2 toggleSize = ImVec2(50, 25);
    float boxRounding = 4.0f;
    float toggleRounding = 12.0f;
    float thumbSizeRatio = 0.8f;
    float thumbPadding = 4.0f;
    float hoverBrightness = 0.2f;
    float highlightIntensity = 0.5f;

    // 颜色设置
    ImU32 boxBgColor = IM_COL32(50, 50, 50, 200);
    ImU32 activeBgColor = IM_COL32(0, 255, 255, 255);
    ImU32 inactiveBgColor = IM_COL32(100, 100, 100, 255);
    ImU32 thumbActiveColor = IM_COL32(255, 255, 255, 255);
    ImU32 thumbInactiveColor = IM_COL32(180, 180, 180, 255);
    ImU32 textColor = IM_COL32(200, 200, 200, 255);  // 默认文本颜色
    ImU32 focusTextColor = IM_COL32(255, 255, 255, 255);  // 焦点文本颜色(纯白色)

    // 缓动函数
    float SmoothStep(float t) {
        return t * t * (3.0f - 2.0f * t);
    }

    // 线性插值
    float ImLerp(float a, float b, float t) {
        return a + (b - a) * t;
    }

    // 颜色插值
    ImU32 LerpColor(ImU32 col1, ImU32 col2, float t) {
        ImVec4 c1 = ImGui::ColorConvertU32ToFloat4(col1);
        ImVec4 c2 = ImGui::ColorConvertU32ToFloat4(col2);
        return ImGui::ColorConvertFloat4ToU32(ImVec4(
            c1.x + (c2.x - c1.x) * t,
            c1.y + (c2.y - c1.y) * t,
            c1.z + (c2.z - c1.z) * t,
            c1.w + (c2.w - c1.w) * t
        ));
    }

    // 应用悬停效果
    ImU32 ApplyHover(ImU32 color, float alpha) {
        if (alpha <= 0.0f) return color;
        ImVec4 c = ImGui::ColorConvertU32ToFloat4(color);
        float factor = hoverBrightness * SmoothStep(alpha);
        c.x = ImMin(c.x + factor, 1.0f);
        c.y = ImMin(c.y + factor, 1.0f);
        c.z = ImMin(c.z + factor, 1.0f);
        return ImGui::ColorConvertFloat4ToU32(c);
    }

    // 绘制实现
    bool DrawImpl(const char* label, const char* identifier, bool state) {
        ImGui::PushID(identifier);

        // 状态变化时触发动画
        if (state != currentState) {
            currentState = state;
            isAnimating = true;
        }

        ImVec2 p = ImGui::GetCursorScreenPos();
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        // 绘制Box背景
        drawList->AddRectFilled(p, ImVec2(p.x + boxSize.x, p.y + boxSize.y), boxBgColor, boxRounding);

        // 计算布局
        bool hasLabel = label && label[0] != '\0';
        float textWidth = hasLabel ? ImGui::CalcTextSize(label).x + ImGui::GetStyle().ItemInnerSpacing.x : 0.0f;

        // 滑动按钮位置(居右)
        ImVec2 togglePos = ImVec2(
            p.x + boxSize.x - toggleSize.x - ImGui::GetStyle().ItemInnerSpacing.x,
            p.y + (boxSize.y - toggleSize.y) * 0.5f
        );

        // 创建交互区域
        ImGui::InvisibleButton(identifier, boxSize);

        // 更新交互状态
        wasHovered = isHovered;
        wasFocused = isFocused;
        isHovered = ImGui::IsItemHovered();
        isFocused = isHovered; // 鼠标悬停时视为有焦点

        // 检测悬停状态变化
        if (isHovered != wasHovered) {
            focusAnim = isFocused ? 0.0f : 1.0f; // 重置焦点动画
        }

        // 更新动画
        float delta = ImGui::GetIO().DeltaTime;

        // 滑动按钮动画
        float switchTarget = currentState ? 1.0f : 0.0f;
        switchPos = ImLerp(switchPos, switchTarget, delta * switchSpeed);
        if (fabs(switchPos - switchTarget) < 0.001f) isAnimating = false;

        // 悬停动画
        float hoverTarget = isHovered ? 1.0f : 0.0f;
        hoverAlpha = ImLerp(hoverAlpha, hoverTarget, delta * hoverSpeed);

        // 焦点动画
        float focusTarget = isFocused ? 1.0f : 0.0f;
        focusAnim = ImLerp(focusAnim, focusTarget, delta * focusSpeed);

        // 计算颜色
        // 使用纯白色作为焦点状态下的文本颜色
        ImU32 currentTextColor = LerpColor(textColor, IM_COL32(255, 255, 255, 255), SmoothStep(focusAnim));

        ImU32 bgColor = currentState ? activeBgColor : inactiveBgColor;
        bgColor = ApplyHover(bgColor, hoverAlpha);
        ImU32 thumbColor = currentState ? thumbActiveColor : thumbInactiveColor;

        // 绘制滑动按钮
        drawList->AddRectFilled(togglePos, ImVec2(togglePos.x + toggleSize.x, togglePos.y + toggleSize.y), bgColor, toggleRounding);

        // 滑块
        float thumbSize = toggleSize.y * thumbSizeRatio;
        float maxOffset = toggleSize.x - thumbSize - thumbPadding * 2;
        float thumbX = togglePos.x + thumbPadding + maxOffset * switchPos;
        drawList->AddCircleFilled(
            ImVec2(thumbX + thumbSize / 2, togglePos.y + toggleSize.y / 2),
            thumbSize / 2, thumbColor, 32
        );

        // 绘制文本(居左)
        if (hasLabel) {
            ImVec2 textPos(
                p.x + ImGui::GetStyle().ItemInnerSpacing.x,
                p.y + (boxSize.y - ImGui::GetTextLineHeight()) * 0.5f
            );
            drawList->AddText(textPos, currentTextColor, label);
        }

        // 处理点击
        bool clicked = ImGui::IsItemClicked();
        if (clicked) currentState = !currentState;

        // 确保正确的布局继续
        ImGui::SetCursorScreenPos(ImVec2(p.x, p.y + boxSize.y + ImGui::GetStyle().ItemSpacing.y));

        ImGui::PopID();
        return clicked;
    }

public:
    static bool ToggleBox(
        const char* label,                             // 切换框的标签文本，显示在切换框旁边
        const char* identifier,                        // 切换框的唯一标识符，用于区分不同的切换框
        bool* state,                                   // 指向切换框当前状态的指针（true 表示开启，false 表示关闭）
        const ImVec2& boxSize = ImVec2(200, 40),       // 切换框的大小，默认为 200x40 像素
        const ImVec2& toggleSize = ImVec2(50, 25),     // 切换按钮的大小，默认为 50x25 像素
        ImU32 boxBgCol = IM_COL32(50, 50, 50, 200),    // 切换框背景颜色，默认为半透明深灰色
        ImU32 activeBgCol = IM_COL32(0, 255, 255, 255),// 切换框开启时的背景颜色，默认为亮蓝色
        ImU32 inactiveBgCol = IM_COL32(100, 100, 100, 255),// 切换框关闭时的背景颜色，默认为灰色
        ImU32 thumbActiveCol = IM_COL32(255, 255, 255, 255),// 切换按钮开启时的颜色，默认为白色
        ImU32 thumbInactiveCol = IM_COL32(180, 180, 180, 255),// 切换按钮关闭时的颜色，默认为浅灰色
        ImU32 textCol = IM_COL32(200, 200, 200, 255),  // 标签文本的颜色，默认为浅灰色
        ImU32 focusTextCol = IM_COL32(255, 255, 255, 255)// 切换框获得焦点时文本的颜色，默认为白色
    ) {

        std::string key = identifier ? identifier : "";
        static std::unordered_map<std::string, SmoothToggleBox> instances;
        auto& instance = instances[key];

        static std::unordered_set<std::string> initialized;
        if (initialized.find(key) == initialized.end()) {
            instance.boxSize = boxSize;
            instance.toggleSize = toggleSize;
            instance.boxBgColor = boxBgCol;
            instance.activeBgColor = activeBgCol;
            instance.inactiveBgColor = inactiveBgCol;
            instance.thumbActiveColor = thumbActiveCol;
            instance.thumbInactiveColor = thumbInactiveCol;
            instance.textColor = textCol;
            instance.focusTextColor = focusTextCol;
            initialized.insert(key);
        }

        bool clicked = instance.DrawImpl(label, identifier, *state);
        if (clicked) *state = instance.currentState;
        return clicked;
    }
};

class SmoothMultiSelectDropdown {
private:
    // 动画状态
    float openAnim = 0.0f;          // 下拉框打开动画进度(0.0到1.0)
    float scrollAnim = 0.0f;        // 滚动条动画进度
    float hoverAlpha = 0.0f;        // 悬停强度(0.0到1.0)
    bool isOpen = false;            // 是否打开下拉框
    bool isAnimating = false;       // 是否正在播放动画
    float currentScroll = 0.0f;     // 当前滚动位置
    float targetScroll = 0.0f;      // 目标滚动位置

    // 动画速度
    float openSpeed = 10.0f;        // 打开/关闭动画速度
    float scrollSpeed = 15.0f;      // 滚动动画速度
    float hoverSpeed = 12.0f;       // 悬停动画速度

    // 样式参数
    ImVec2 size = ImVec2(200, 30);  // 主控件大小
    float dropdownRounding = 4.0f;   // 下拉框圆角
    float itemRounding = 2.0f;      // 选项圆角
    float arrowSize = 8.0f;         // 箭头大小
    float checkmarkSize = 12.0f;    // 对勾大小
    float hoverBrightness = 0.2f;   // 悬停亮度增加值
    float scrollbarWidth = 6.0f;    // 滚动条宽度

    // 颜色设置
    ImU32 bgColor = IM_COL32(50, 50, 50, 255);      // 背景颜色
    ImU32 borderColor = IM_COL32(80, 80, 80, 255);  // 边框颜色
    ImU32 textColor = IM_COL32(255, 255, 255, 255); // 文本颜色
    ImU32 hoverColor = IM_COL32(70, 70, 70, 255);   // 悬停颜色
    ImU32 activeColor = IM_COL32(100, 200, 255, 255); // 激活颜色
    ImU32 checkmarkColor = IM_COL32(255, 255, 255, 255); // 对勾颜色
    ImU32 scrollbarColor = IM_COL32(100, 100, 100, 150); // 滚动条颜色
    ImU32 scrollbarHoverColor = IM_COL32(120, 120, 120, 200); // 滚动条悬停颜色

    // 缓动函数
    float SmoothStep(float t) {
        return t * t * (3.0f - 2.0f * t);
    }

    // 颜色插值
    ImU32 LerpColor(ImU32 col1, ImU32 col2, float t) {
        ImVec4 c1 = ImGui::ColorConvertU32ToFloat4(col1);
        ImVec4 c2 = ImGui::ColorConvertU32ToFloat4(col2);
        return ImGui::ColorConvertFloat4ToU32(ImVec4(
            c1.x + (c2.x - c1.x) * t,
            c1.y + (c2.y - c1.y) * t,
            c1.z + (c2.z - c1.z) * t,
            c1.w + (c2.w - c1.w) * t
        ));
    }

    // 应用悬停效果
    ImU32 ApplyHover(ImU32 color, float alpha) {
        if (alpha <= 0.0f) return color;
        ImVec4 c = ImGui::ColorConvertU32ToFloat4(color);
        float factor = hoverBrightness * SmoothStep(alpha);
        c.x = ImMin(c.x + factor, 1.0f);
        c.y = ImMin(c.y + factor, 1.0f);
        c.z = ImMin(c.z + factor, 1.0f);
        return ImGui::ColorConvertFloat4ToU32(c);
    }

    // 绘制对勾
    void DrawCheckmark(ImDrawList* drawList, ImVec2 pos, ImU32 color, float size, float alpha = 1.0f) {
        if (alpha <= 0.0f) return;

        float thickness = ImMax(1.0f, size * 0.15f);
        float offset = size * 0.25f;

        ImVec2 points[3] = {
            ImVec2(pos.x + offset, pos.y + size * 0.5f),
            ImVec2(pos.x + size * 0.5f, pos.y + size - offset),
            ImVec2(pos.x + size - offset, pos.y + offset)
        };

        drawList->AddPolyline(points, 3, color, false, thickness);
    }

    // 绘制下拉箭头
    void DrawArrow(ImDrawList* drawList, ImVec2 pos, ImU32 color, float size, float alpha = 1.0f, bool pointingDown = true) {
        if (alpha <= 0.0f) return;

        float thickness = ImMax(1.0f, size * 0.15f);
        float center = size * 0.5f;

        if (pointingDown) {
            ImVec2 points[3] = {
                ImVec2(pos.x, pos.y + center * 0.5f),
                ImVec2(pos.x + center, pos.y + center * 1.5f),
                ImVec2(pos.x + size, pos.y + center * 0.5f)
            };
            drawList->AddPolyline(points, 3, color, false, thickness);
        }
        else {
            ImVec2 points[3] = {
                ImVec2(pos.x, pos.y + center * 1.5f),
                ImVec2(pos.x + center, pos.y + center * 0.5f),
                ImVec2(pos.x + size, pos.y + center * 1.5f)
            };
            drawList->AddPolyline(points, 3, color, false, thickness);
        }
    }

    // 绘制省略号
    void DrawEllipsis(ImDrawList* drawList, ImVec2 pos, ImU32 color, float size) {
        float radius = size * 0.1f;
        float spacing = size * 0.3f;

        drawList->AddCircleFilled(ImVec2(pos.x + radius, pos.y + size * 0.5f), radius, color);
        drawList->AddCircleFilled(ImVec2(pos.x + radius + spacing, pos.y + size * 0.5f), radius, color);
        drawList->AddCircleFilled(ImVec2(pos.x + radius + spacing * 2, pos.y + size * 0.5f), radius, color);
    }

    // 绘制实现
    bool DrawImpl(const char* identifier, const char* items[], bool selected[], int itemCount, int maxVisibleItems = 5) {
        ImGui::PushID(identifier);
        bool valueChanged = false;

        ImVec2 p = ImGui::GetCursorScreenPos();
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImGuiIO& io = ImGui::GetIO();

        // 主控件交互区域
        ImGui::InvisibleButton(identifier, size);
        bool isHovered = ImGui::IsItemHovered();
        bool isClicked = ImGui::IsItemClicked();

        // 更新动画状态
        if (isClicked) {
            isOpen = !isOpen;
            isAnimating = true;
        }

        // 更新打开/关闭动画
        float delta = io.DeltaTime;
        float openTarget = isOpen ? 1.0f : 0.0f;
        openAnim = ImLerp(openAnim, openTarget, delta * openSpeed);
        if (fabs(openAnim - openTarget) < 0.001f) isAnimating = false;

        // 更新悬停动画
        float hoverTarget = isHovered ? 1.0f : 0.0f;
        hoverAlpha = ImLerp(hoverAlpha, hoverTarget, delta * hoverSpeed);

        // 计算主控件颜色
        ImU32 currentBgColor = ApplyHover(bgColor, hoverAlpha);
        ImU32 currentTextColor = textColor;

        // 绘制主控件背景
        drawList->AddRectFilled(p, ImVec2(p.x + size.x, p.y + size.y), currentBgColor, dropdownRounding);
        drawList->AddRect(p, ImVec2(p.x + size.x, p.y + size.y), borderColor, dropdownRounding, 0, 1.0f);

        // 计算并绘制选中项的文本
        int selectedCount = 0;
        std::string selectedText;
        for (int i = 0; i < itemCount; ++i) {
            if (selected[i]) {
                if (!selectedText.empty()) selectedText += ", ";
                selectedText += items[i];
                selectedCount++;
            }
        }

        if (selectedCount == 0) {
            selectedText = "None";
        }
        else if (selectedCount == itemCount) {
            selectedText = "All";
        }

        // 测量文本宽度
        float textWidth = ImGui::CalcTextSize(selectedText.c_str()).x;
        float availableWidth = size.x - arrowSize - ImGui::GetStyle().ItemInnerSpacing.x * 2;

        // 绘制文本(带省略号处理)
        if (textWidth <= availableWidth) {
            ImVec2 textPos(p.x + ImGui::GetStyle().ItemInnerSpacing.x, p.y + (size.y - ImGui::GetTextLineHeight()) * 0.5f);
            drawList->AddText(textPos, currentTextColor, selectedText.c_str());
        }
        else {
            // 查找可以显示的文本部分
            float ellipsisWidth = ImGui::CalcTextSize("...").x;
            float remainingWidth = availableWidth - ellipsisWidth;

            std::string displayText;
            float currentWidth = 0.0f;

            for (size_t i = 0; i < selectedText.size(); ++i) {
                float charWidth = ImGui::CalcTextSize(&selectedText[i], &selectedText[i] + 1).x;
                if (currentWidth + charWidth > remainingWidth) break;

                displayText += selectedText[i];
                currentWidth += charWidth;
            }

            displayText += "...";
            ImVec2 textPos(p.x + ImGui::GetStyle().ItemInnerSpacing.x, p.y + (size.y - ImGui::GetTextLineHeight()) * 0.5f);
            drawList->AddText(textPos, currentTextColor, displayText.c_str());
        }

        // 绘制下拉箭头
        ImVec2 arrowPos(p.x + size.x - arrowSize - ImGui::GetStyle().ItemInnerSpacing.x,
            p.y + (size.y - arrowSize) * 0.5f);
        DrawArrow(drawList, arrowPos, currentTextColor, arrowSize, 1.0f, !isOpen);

        // 下拉框逻辑
        if (openAnim > 0.0f) {
            float dropdownHeight = ImMin(static_cast<float>(maxVisibleItems) * size.y,
                static_cast<float>(itemCount) * size.y);
            float animHeight = dropdownHeight * SmoothStep(openAnim);

            // 计算下拉框位置
            ImVec2 dropdownPos(p.x, p.y + size.y + 2.0f);
            ImVec2 dropdownSize(size.x, animHeight);

            // 创建下拉框裁剪区域
            ImGui::PushClipRect(dropdownPos, ImVec2(dropdownPos.x + dropdownSize.x, dropdownPos.y + dropdownSize.y), true);

            // 绘制下拉框背景 - 确保不透明
            ImU32 opaqueBgColor = (bgColor & IM_COL32_A_MASK) ? bgColor : IM_COL32(50, 50, 50, 255);
            drawList->AddRectFilled(dropdownPos, ImVec2(dropdownPos.x + dropdownSize.x, dropdownPos.y + dropdownSize.y),
                opaqueBgColor, dropdownRounding);
            drawList->AddRect(dropdownPos, ImVec2(dropdownPos.x + dropdownSize.x, dropdownPos.y + dropdownSize.y),
                borderColor, dropdownRounding, 0, 1.0f);

            // 处理滚动
            if (ImGui::IsWindowHovered() && io.MouseWheel != 0.0f) {
                targetScroll = ImClamp(targetScroll - io.MouseWheel * 20.0f, 0.0f,
                    ImMax(0.0f, static_cast<float>(itemCount) * size.y - dropdownHeight));
            }

            // 更新滚动动画
            currentScroll = ImLerp(currentScroll, targetScroll, delta * scrollSpeed);

            // 计算可见项范围
            int firstVisible = static_cast<int>(currentScroll / size.y);
            int lastVisible = ImMin(firstVisible + maxVisibleItems + 1, itemCount);

            // 绘制可见项
            for (int i = firstVisible; i < lastVisible; ++i) {
                ImVec2 itemPos(dropdownPos.x, dropdownPos.y + i * size.y - currentScroll);

                // 跳过不可见项
                if (itemPos.y + size.y < dropdownPos.y || itemPos.y > dropdownPos.y + dropdownSize.y) continue;

                bool itemHovered = ImGui::IsMouseHoveringRect(itemPos, ImVec2(itemPos.x + size.x, itemPos.y + size.y));
                ImU32 itemBgColor = itemHovered ? hoverColor : bgColor;

                // 绘制项背景
                drawList->AddRectFilled(itemPos, ImVec2(itemPos.x + size.x, itemPos.y + size.y),
                    itemBgColor, itemRounding);

                // 绘制选中状态的对勾
                if (selected[i]) {
                    ImVec2 checkPos(itemPos.x + 5.0f, itemPos.y + (size.y - checkmarkSize) * 0.5f);
                    DrawCheckmark(drawList, checkPos, checkmarkColor, checkmarkSize);
                }

                // 绘制文本
                ImVec2 textPos(itemPos.x + checkmarkSize + 10.0f, itemPos.y + (size.y - ImGui::GetTextLineHeight()) * 0.5f);
                drawList->AddText(textPos, textColor, items[i]);

                // 处理项点击
                if (itemHovered && ImGui::IsMouseClicked(0)) {
                    selected[i] = !selected[i];
                    valueChanged = true;
                }
            }

            // 绘制滚动条(如果需要)
            if (itemCount > maxVisibleItems) {
                float scrollRatio = dropdownHeight / (static_cast<float>(itemCount) * size.y);
                float scrollbarHeight = dropdownHeight * scrollRatio;
                float scrollbarPos = (currentScroll / (static_cast<float>(itemCount) * size.y - dropdownHeight)) * (dropdownHeight - scrollbarHeight);

                ImVec2 scrollbarStart(dropdownPos.x + size.x - scrollbarWidth - 2.0f, dropdownPos.y + scrollbarPos);
                ImVec2 scrollbarEnd(scrollbarStart.x + scrollbarWidth, scrollbarStart.y + scrollbarHeight);

                bool scrollbarHovered = ImGui::IsMouseHoveringRect(scrollbarStart, scrollbarEnd);
                ImU32 scrollbarCol = scrollbarHovered ? scrollbarHoverColor : scrollbarColor;

                drawList->AddRectFilled(scrollbarStart, scrollbarEnd, scrollbarCol, scrollbarWidth * 0.5f);

                // 处理滚动条拖动
                if (scrollbarHovered && ImGui::IsMouseDown(0)) {
                    float mousePosY = io.MousePos.y - dropdownPos.y;
                    float normalizedPos = mousePosY / dropdownHeight;
                    targetScroll = normalizedPos * (static_cast<float>(itemCount) * size.y - dropdownHeight);
                    targetScroll = ImClamp(targetScroll, 0.0f, static_cast<float>(itemCount) * size.y - dropdownHeight);
                }
            }

            ImGui::PopClipRect();

            // 点击外部关闭下拉框
            if (ImGui::IsMouseClicked(0) && !ImGui::IsItemHovered() &&
                !ImGui::IsMouseHoveringRect(dropdownPos, ImVec2(dropdownPos.x + dropdownSize.x, dropdownPos.y + dropdownSize.y))) {
                isOpen = false;
                isAnimating = true;
            }

            // 阻止与下层组件的交互
            ImGui::SetItemAllowOverlap();
            ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPos().x, ImGui::GetCursorPos().y + animHeight + 2.0f));
        }

        ImGui::PopID();
        return valueChanged;
    }

public:
    // 静态接口
    static bool MultiSelect(
        const char* identifier,                     // 多选框的唯一标识符，用于区分不同的多选框
        const char* items[],                        // 选项数组，包含所有可选的项目
        bool selected[],                            // 选中状态数组，表示每个项目的选中状态
        int itemCount,                              // 选项数组的大小，即选项的数量
        const ImVec2& size = ImVec2(200, 30),       // 多选框的大小，默认宽度为200像素，高度为30像素
        int maxVisibleItems = 5,                    // 最多显示的选项数量，默认为5
        ImU32 bgCol = IM_COL32(50, 50, 50, 255),    // 多选框背景颜色，默认为深灰色
        ImU32 borderCol = IM_COL32(80, 80, 80, 255),// 多选框边框颜色，默认为灰色
        ImU32 textCol = IM_COL32(255, 255, 255, 255),// 文本颜色，默认为白色
        ImU32 hoverCol = IM_COL32(70, 70, 70, 255), // 悬停时的背景颜色，默认为浅灰色
        ImU32 activeCol = IM_COL32(100, 200, 255, 255),// 选中项的背景颜色，默认为亮蓝色
        ImU32 checkmarkCol = IM_COL32(255, 255, 255, 255),// 对勾颜色，默认为白色
        float rounding = 4.0f                       // 多选框的圆角半径，默认为4像素
    ) {

        static std::unordered_map<std::string, SmoothMultiSelectDropdown> instances;
        auto& instance = instances[identifier];

        // 首次调用时初始化
        static std::unordered_set<std::string> initialized;
        if (initialized.find(identifier) == initialized.end()) {
            instance.size = size;
            instance.bgColor = bgCol;
            instance.borderColor = borderCol;
            instance.textColor = textCol;
            instance.hoverColor = hoverCol;
            instance.activeColor = activeCol;
            instance.checkmarkColor = checkmarkCol;
            instance.dropdownRounding = rounding;
            initialized.insert(identifier);
        }

        return instance.DrawImpl(identifier, items, selected, itemCount, maxVisibleItems);
    }
};