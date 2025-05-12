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
    // ����״̬
    float switchPos = 0.0f;
    bool currentState = false;
    bool isAnimating = false;

    // ��ͣ״̬
    float hoverAlpha = 0.0f;

    // ��������
    float switchSpeed = 5.0f;
    float hoverSpeed = 10.0f;

    // ��ʽ����
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

    // ����Ч��
    float SmoothStep(float t) {
        return t * t * (3.0f - 2.0f * t);
    }

    // ����ʵ��
    bool DrawImpl(const char* label, const char* identifier, bool state) {
        ImGui::PushID(identifier);

        if (state != currentState) {
            currentState = state;
            isAnimating = true;
        }

        // ���㲼��
        bool hasLabel = label && label[0] != '\0';
        float textWidth = hasLabel ? ImGui::CalcTextSize(label).x + ImGui::GetStyle().ItemInnerSpacing.x : 0.0f;
        ImVec2 p = ImGui::GetCursorScreenPos();
        ImVec2 totalSize(size.x + textWidth, ImMax(size.y, ImGui::GetTextLineHeight()));

        // ��������
        bool clicked = ImGui::InvisibleButton(identifier, totalSize);
        bool isHovered = ImGui::IsItemHovered();

        // ���¶���
        float delta = ImGui::GetIO().DeltaTime;
        float switchTarget = currentState ? 1.0f : 0.0f;
        switchPos = ImLerp(switchPos, switchTarget, delta * switchSpeed);
        if (fabs(switchPos - switchTarget) < 0.001f) isAnimating = false;

        float hoverTarget = isHovered ? 1.0f : 0.0f;
        hoverAlpha = ImLerp(hoverAlpha, hoverTarget, delta * hoverSpeed);

        // ������ɫ
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

        // ����
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddRectFilled(p, ImVec2(p.x + size.x, p.y + size.y), bgColor, rounding);

        // ����
        float thumbSize = size.y * thumbSizeRatio;
        float maxOffset = size.x - thumbSize - thumbPadding * 2;
        float thumbX = p.x + thumbPadding + maxOffset * switchPos;
        drawList->AddCircleFilled(ImVec2(thumbX + thumbSize / 2, p.y + size.y / 2),
            thumbSize / 2, thumbColor, 32);

        // �ı�
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
    // ��̬�ӿ� - ģ��ImGui���ʹ�ö�����label��identifier
    static bool Toggle(
        const char* label, //��ť�ı�ǩ�ı������������ַ��򲻻���
        const char* identifier,  //��ť��Ψһ��ʶ�����������ֲ�ͬ�İ�ť
        bool* state,  //ָ��һ������ֵ��ָ�룬��ʾ��ť�ĵ�ǰ״̬��true ��ʾ������false ��ʾ�رգ�
        const ImVec2& size = ImVec2(35, 20),  //��ť�Ĵ�С��������Ϊ��λ
        ImU32 activeBg = IM_COL32(0, 255, 255, 255),  //��ť���ڿ���״̬ʱ�ı�����ɫ
        ImU32 inactiveBg = IM_COL32(100, 100, 100, 255), //��ť���ڹر�״̬ʱ�ı�����ɫ
        ImU32 thumbActive = IM_COL32(255, 255, 255, 255),  //��ť���ڿ���״̬ʱ�Ļ�����ɫ
        ImU32 thumbInactive = IM_COL32(180, 180, 180, 255),  //��ť���ڹر�״̬ʱ�Ļ�����ɫ
        ImU32 textActive = IM_COL32(255, 255, 255, 255),  //��ť���ڿ���״̬ʱ���ı���ɫ
        ImU32 textInactive = IM_COL32(150, 150, 150, 255))  //��ť���ڹر�״̬ʱ���ı���ɫ
    {

        std::string key = identifier ? identifier : "";
        static std::unordered_map<std::string, SmoothToggleButton> instances;
        auto& instance = instances[key];

        // �״ε���ʱ��ʼ��
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
}; //������ť

class SmoothSliderfloat {
private:
    // ����ֵ
    float currentValue = 0.0f;
    float displayValue = 0.0f;
    bool isDragging = false;
    bool isHovered = false;
    bool hasFocus = false;
    float fadeAlpha = 1.0f;
    float focusLostTime = 0.0f;
    const float fadeDelay = 3.0f;

    // ��������
    float animationSpeed = 10.0f;
    float hoverIntensity = 0.0f;
    float hoverAlpha = 0.0f;
    bool isActive = false;

    // ��ʽ����
    ImVec2 size = ImVec2(200, 20);
    float thumbWidth = 12.0f;
    float thumbHeight = 20.0f;
    float rounding = 4.0f;
    float thumbRounding = 4.0f;
    float textOffsetX = 5.0f;
    float textOffsetY = 2.0f;

    // ��ɫ����
    ImU32 bgColor = IM_COL32(70, 70, 70, 255);
    ImU32 fillColor = IM_COL32(255, 182, 193, 255);
    ImU32 thumbColor = IM_COL32(220, 220, 220, 255);
    ImU32 hoverColor = IM_COL32(255, 255, 255, 50);
    ImU32 textColor = IM_COL32(255, 255, 255, 255);
    ImU32 titleColor = IM_COL32(220, 220, 220, 255);

    // ��������
    float EaseOut(float t) {
        return 1.0f - (1.0f - t) * (1.0f - t);
    }

    // ����ʵ��
    float DrawImpl(const char* label, const char* identifier, float min, float max, float value) {
        ImGui::PushID(identifier);

        // ���µ�ǰֵ
        currentValue = value;

        // ���㲼��
        ImGui::BeginGroup();

        // ���Ʊ�ǩ������У� - �̶��ڻ������Ϸ�
        bool hasLabel = label && label[0] != '\0';
        if (hasLabel) {
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(titleColor), "%s", label);
        }

        // ����������
        ImVec2 p = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton("##slider", size);

        // ��������
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

        // ���¶���
        float delta = ImGui::GetIO().DeltaTime;
        displayValue = ImLerp(displayValue, currentValue, delta * animationSpeed);

        // ������ͣЧ��
        float hoverTarget = isHovered || isActive ? 1.0f : 0.0f;
        hoverAlpha = ImLerp(hoverAlpha, hoverTarget, delta * 15.0f);

        // �����ı�͸����
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

        // ������Ʋ���
        float fillRatio = (displayValue - min) / (max - min);
        thumbMin = ImVec2(p.x + fillRatio * (size.x - thumbWidth), p.y - (thumbHeight - size.y) / 2);
        thumbMax = ImVec2(thumbMin.x + thumbWidth, thumbMin.y + thumbHeight);

        // ���Ʊ���
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        // ����
        drawList->AddRectFilled(
            p,
            ImVec2(p.x + size.x, p.y + size.y),
            bgColor,
            rounding
        );

        // ��䲿��
        drawList->AddRectFilled(
            p,
            ImVec2(p.x + fillRatio * size.x, p.y + size.y),
            fillColor,
            rounding
        );

        // ��ͣЧ��
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

        // ���飨Ĵָ��
        drawList->AddRectFilled(
            thumbMin,
            thumbMax,
            thumbColor,
            thumbRounding,
            ImDrawFlags_RoundCornersAll
        );

        // �ڻ������½ǻ��Ƶ�ǰֵ
        char valueText[32];
        snprintf(valueText, sizeof(valueText), "%.2f", displayValue);
        ImVec2 textSize = ImGui::CalcTextSize(valueText);

        // �����ı�λ�ã����滬���ƶ���
        ImVec2 textPos = ImVec2(thumbMax.x + textOffsetX, thumbMax.y + textOffsetY);

        // ��ȡ���ڱ߽���Ϣ
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 windowSize = ImGui::GetWindowSize();

        // ȷ���ı����ᳬ�������ұ߽�
        if (textPos.x + textSize.x > windowPos.x + windowSize.x) {
            textPos.x = thumbMin.x - textSize.x - textOffsetX;
        }

        // ȷ���ı����ᳬ�����ڵײ��߽�
        if (textPos.y + textSize.y > windowPos.y + windowSize.y) {
            textPos.y = thumbMin.y - textSize.y - textOffsetY;
        }

        // �����ı�
        ImVec4 textCol = ImGui::ColorConvertU32ToFloat4(textColor);
        textCol.w *= fadeAlpha;
        drawList->AddText(
            textPos,
            ImGui::GetColorU32(textCol),
            valueText
        );

        // ===== ����ӵļ����ƴ��� =====
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
    // ��̬�ӿ�
    static float Slider(
        const char* label,                       // �������ı�ǩ�ı�����ʾ�ڻ������Ա߻򻬶�����
        const char* identifier,                  // ��������Ψһ��ʶ�����������ֲ�ͬ�Ļ�����
        float* value,                            // ָ�򻬶�����ǰֵ��ָ��
        float min,                                 // ����������Сֵ
        float max,                                 // �����������ֵ
        const ImVec2& size = ImVec2(300, 5),       // �������Ĵ�С��Ĭ�Ͽ��Ϊ300���أ��߶�Ϊ5����
        float thumbWidth = 10.0f,                  // ����Ŀ�ȣ�Ĭ��Ϊ10����
        float thumbHeight = 10.0f,                 // ����ĸ߶ȣ�Ĭ��Ϊ10����
        float thumbRounding = 20.0f,               // �����Բ�ǰ뾶��Ĭ��Ϊ20����
        ImU32 bgCol = IM_COL32(50, 50, 80, 255),   // ������������ɫ��Ĭ��Ϊ����ɫ
        ImU32 fillCol = IM_COL32(100, 200, 255, 255), // �����������ɫ��Ĭ��Ϊ����ɫ
        ImU32 thumbCol = IM_COL32(255, 255, 255, 255),// ������ɫ��Ĭ��Ϊ��ɫ
        ImU32 textCol = IM_COL32(255, 255, 255, 255), // �ı���ɫ��Ĭ��Ϊ��ɫ
        ImU32 titleCol = IM_COL32(220, 220, 220, 255) // ������ɫ��Ĭ��Ϊǳ��ɫ
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
};  //float������

class SmoothSliderInt {
private:
    // ����ֵ
    int currentValue = 0;
    float displayValue = 0.0f; // ����ƽ����ʾ�ĸ���ֵ
    bool isDragging = false;
    bool isHovered = false;
    bool hasFocus = false;
    float fadeAlpha = 1.0f;
    float focusLostTime = 0.0f;
    const float fadeDelay = 3.0f;

    // ��������
    float animationSpeed = 10.0f;
    float hoverIntensity = 0.0f;
    float hoverAlpha = 0.0f;
    bool isActive = false;

    // ��ʽ����
    ImVec2 size = ImVec2(200, 20);
    float thumbWidth = 12.0f;
    float thumbHeight = 20.0f;
    float rounding = 4.0f;
    float thumbRounding = 4.0f;
    float textOffsetX = 5.0f;
    float textOffsetY = 2.0f;

    // ��ɫ����
    ImU32 bgColor = IM_COL32(70, 70, 70, 255);
    ImU32 fillColor = IM_COL32(255, 182, 193, 255);
    ImU32 thumbColor = IM_COL32(220, 220, 220, 255);
    ImU32 hoverColor = IM_COL32(255, 255, 255, 50);
    ImU32 textColor = IM_COL32(255, 255, 255, 255);
    ImU32 titleColor = IM_COL32(220, 220, 220, 255);

    // ��������
    float EaseOut(float t) {
        return 1.0f - (1.0f - t) * (1.0f - t);
    }

    // ����ʵ��
    int DrawImpl(const char* label, const char* identifier, int min, int max, int value) {
        ImGui::PushID(identifier);

        // ���µ�ǰֵ
        currentValue = value;

        // ���㲼��
        ImGui::BeginGroup();

        // ���Ʊ�ǩ������У� - �̶��ڻ������Ϸ�
        bool hasLabel = label && label[0] != '\0';
        if (hasLabel) {
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(titleColor), "%s", label);
        }

        // ����������
        ImVec2 p = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton("##slider", size);

        // ��������
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

        // ���¶���
        float delta = ImGui::GetIO().DeltaTime;
        displayValue = ImLerp(displayValue, (float)currentValue, delta * animationSpeed);

        // ������ͣЧ��
        float hoverTarget = isHovered || isActive ? 1.0f : 0.0f;
        hoverAlpha = ImLerp(hoverAlpha, hoverTarget, delta * 15.0f);

        // �����ı�͸����
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

        // ������Ʋ���
        float fillRatio = (displayValue - min) / (max - min);
        thumbMin = ImVec2(p.x + fillRatio * (size.x - thumbWidth), p.y - (thumbHeight - size.y) / 2);
        thumbMax = ImVec2(thumbMin.x + thumbWidth, thumbMin.y + thumbHeight);

        // ���Ʊ���
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        // ����
        drawList->AddRectFilled(
            p,
            ImVec2(p.x + size.x, p.y + size.y),
            bgColor,
            rounding
        );

        // ��䲿��
        drawList->AddRectFilled(
            p,
            ImVec2(p.x + fillRatio * size.x, p.y + size.y),
            fillColor,
            rounding
        );

        // ��ͣЧ��
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

        // ���飨Ĵָ��
        drawList->AddRectFilled(
            thumbMin,
            thumbMax,
            thumbColor,
            thumbRounding,
            ImDrawFlags_RoundCornersAll
        );

        // �ڻ������½ǻ��Ƶ�ǰֵ
        char valueText[32];
        snprintf(valueText, sizeof(valueText), "%d", (int)displayValue);
        ImVec2 textSize = ImGui::CalcTextSize(valueText);

        // �����ı�λ�ã����滬���ƶ���
        ImVec2 textPos = ImVec2(thumbMax.x + textOffsetX, thumbMax.y + textOffsetY);

        // ��ȡ���ڱ߽���Ϣ
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 windowSize = ImGui::GetWindowSize();

        // ȷ���ı����ᳬ�������ұ߽�
        if (textPos.x + textSize.x > windowPos.x + windowSize.x) {
            textPos.x = thumbMin.x - textSize.x - textOffsetX;
        }

        // ȷ���ı����ᳬ�����ڵײ��߽�
        if (textPos.y + textSize.y > windowPos.y + windowSize.y) {
            textPos.y = thumbMin.y - textSize.y - textOffsetY;
        }

        // �����ı�
        ImVec4 textCol = ImGui::ColorConvertU32ToFloat4(textColor);
        textCol.w *= fadeAlpha;
        drawList->AddText(
            textPos,
            ImGui::GetColorU32(textCol),
            valueText
        );

        // ===== ����ӵļ����ƴ��� =====
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
    // ��̬�ӿ�
    static int Slider(
        const char* label,                     // �������ı�ǩ�ı�����ʾ�ڻ������Ա߻򻬶�����
        const char* identifier,                // ��������Ψһ��ʶ�����������ֲ�ͬ�Ļ�����
        int* value,                          // ָ�򻬶�����ǰֵ��ָ��
        int min,                             // ����������Сֵ
        int max,                             // �����������ֵ
        const ImVec2& size = ImVec2(300, 5),   // �������Ĵ�С��Ĭ�Ͽ��Ϊ300���أ��߶�Ϊ5����
        float thumbWidth = 10.0f,              // ����Ŀ�ȣ�Ĭ��Ϊ10����
        float thumbHeight = 10.0f,             // ����ĸ߶ȣ�Ĭ��Ϊ10����
        float thumbRounding = 20.0f,           // �����Բ�ǰ뾶��Ĭ��Ϊ20����
        ImU32 bgCol = IM_COL32(50, 50, 80, 255),// ������������ɫ��Ĭ��Ϊ����ɫ
        ImU32 fillCol = IM_COL32(100, 200, 255, 255),// �����������ɫ��Ĭ��Ϊ����ɫ
        ImU32 thumbCol = IM_COL32(255, 255, 255, 255),// ������ɫ��Ĭ��Ϊ��ɫ
        ImU32 textCol = IM_COL32(255, 255, 255, 255), // �ı���ɫ��Ĭ��Ϊ��ɫ
        ImU32 titleCol = IM_COL32(220, 220, 220, 255) // ������ɫ��Ĭ��Ϊǳ��ɫ
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
};  //int������    

class SmoothInput {
private:
    // ״̬����
    float hoverAlpha = 0.0f;
    float focusAlpha = 0.0f;
    bool hasFocus = false;

    // ��ʽ����
    ImVec2 size = ImVec2(200, 30);
    float rounding = 4.0f;
    float borderWidth = 1.5f;

    // ��ɫ����
    ImU32 bgColor = IM_COL32(40, 40, 40, 255);
    ImU32 borderColor = IM_COL32(100, 100, 100, 255);
    ImU32 hoverColor = IM_COL32(255, 255, 255, 30);
    ImU32 focusColor = IM_COL32(100, 150, 255, 150);

    // ���Ļ���ʵ��
    bool DrawImpl(const char* label, char* buf, size_t buf_size) {
        std::string thisId = label ? std::string(label) : "##input";
        ImGui::PushID(thisId.c_str());

        // 1. ���Ʊ����ͽ�������
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton("##input_area", size);

        bool hovered = ImGui::IsItemHovered();
        bool clicked = ImGui::IsItemClicked();

        // 2. �������
        if (clicked) {
            hasFocus = true;
        }

        // ����Ƿ�ʧȥ����
        if (hasFocus && !ImGui::IsItemActive() && ImGui::IsMouseClicked(0)) {
            hasFocus = false;
        }

        bool isActive = hasFocus;

        // 3. ���¶���״̬
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
        const char* label,                     // �����ı�ǩ�ı�����ʾ��������Ա߻��������
        char* buf,                             // ָ����������ݵ��ַ�����
        size_t buf_size,                       // ��������ݵĻ�������С
        const ImVec2& size = ImVec2(200, 30),  // �����Ĵ�С��Ĭ�Ͽ��Ϊ200���أ��߶�Ϊ30����
        float rounding = 4.0f,                 // ������Բ�ǰ뾶��Ĭ��Ϊ4����
        float borderWidth = 1.5f,              // �����߿�Ŀ�ȣ�Ĭ��Ϊ1.5����
        ImU32 bgCol = IM_COL32(40, 40, 40, 255),// ����򱳾���ɫ��Ĭ��Ϊ���ɫ
        ImU32 borderCol = IM_COL32(100, 100, 100, 255),// �����߿���ɫ��Ĭ��Ϊ��ɫ
        ImU32 hoverCol = IM_COL32(255, 255, 255, 30),// �������ͣʱ�ı�����ɫ��Ĭ��Ϊǳ��ɫ
        ImU32 focusCol = IM_COL32(100, 150, 255, 150) // ������ý���ʱ�ı�����ɫ��Ĭ��Ϊ����ɫ
    ) {

        // ʹ�ù�ϣֵ��Ϊ������Ϊÿ������򴴽�������ʵ��
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
};//�����

class SmoothCheckBox {
private:
    // ����״̬
    float checkAnim = 0.0f;      // ѡ�ж�������(0.0��1.0)
    bool currentState = false;   // ��ǰ��ѡ��״̬
    bool isAnimating = false;    // �Ƿ����ڲ��Ŷ���

    // ��ͣЧ��
    float hoverAlpha = 0.0f;     // ��ͣǿ��(0.0��1.0)

    // �����ٶ�
    float animSpeed = 8.0f;      // ѡ�ж����ٶ�
    float hoverSpeed = 12.0f;    // ��ͣ�����ٶ�

    // ��ʽ����
    ImVec2 size = ImVec2(20, 20); // Ĭ�ϸ�ѡ���С
    float rounding = 4.0f;        // �߿�Բ��
    float borderThickness = 2.0f; // �߿���
    float hoverBrightness = 0.2f; // ��ͣ��������ֵ

    // ��ɫ����
    ImU32 activeColor = IM_COL32(100, 200, 255, 255);   // ѡ��״̬��ɫ
    ImU32 inactiveColor = IM_COL32(100, 100, 100, 255); // δѡ��״̬��ɫ
    ImU32 borderColor = IM_COL32(200, 200, 200, 255);   // �߿���ɫ
    ImU32 textColor = IM_COL32(255, 255, 255, 255);     // �ı���ɫ

    // ��ɫ��ֵ����
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

    // ��������
    float SmoothStep(float t) {
        return t * t * (3.0f - 2.0f * t);
    }

    // Ӧ����ͣЧ��
    ImU32 ApplyHover(ImU32 color, float alpha) {
        if (alpha <= 0.0f) return color;
        ImVec4 c = ImGui::ColorConvertU32ToFloat4(color);
        float factor = hoverBrightness * SmoothStep(alpha);
        c.x = ImMin(c.x + factor, 1.0f);
        c.y = ImMin(c.y + factor, 1.0f);
        c.z = ImMin(c.z + factor, 1.0f);
        return ImGui::ColorConvertFloat4ToU32(c);
    }

    // ����ʵ��
    bool DrawImpl(const char* label, const char* identifier, bool state) {
        ImGui::PushID(identifier);

        // ״̬�仯ʱ��������
        if (state != currentState) {
            currentState = state;
            isAnimating = true;
        }

        // ���㲼��
        bool hasLabel = label && label[0] != '\0';
        float textWidth = hasLabel ? ImGui::CalcTextSize(label).x + ImGui::GetStyle().ItemInnerSpacing.x : 0.0f;
        ImVec2 p = ImGui::GetCursorScreenPos();
        ImVec2 totalSize(size.x + textWidth, ImMax(size.y, ImGui::GetTextLineHeight()));

        // ��������
        bool clicked = ImGui::InvisibleButton(identifier, totalSize);
        bool isHovered = ImGui::IsItemHovered();

        // ���¶���
        float delta = ImGui::GetIO().DeltaTime;
        float animTarget = currentState ? 1.0f : 0.0f;
        checkAnim = ImLerp(checkAnim, animTarget, delta * animSpeed);
        if (fabs(checkAnim - animTarget) < 0.001f) isAnimating = false;

        float hoverTarget = isHovered ? 1.0f : 0.0f;
        hoverAlpha = ImLerp(hoverAlpha, hoverTarget, delta * hoverSpeed);

        // ������ɫ
        ImU32 bgColor = ImLerpColor(inactiveColor, activeColor, checkAnim);
        bgColor = ApplyHover(bgColor, hoverAlpha);

        // ����
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        // ���Ʊ���(��������ɫ)
        drawList->AddRectFilled(p, ImVec2(p.x + size.x, p.y + size.y), bgColor, rounding);

        // ���Ʊ߿�
        drawList->AddRect(p, ImVec2(p.x + size.x, p.y + size.y), borderColor, rounding, 0, borderThickness);

        // �����ı�
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
    // ��̬�ӿ� - ģ��ImGui��񣬽������ID�ֿ�
    static bool Checkbox(
        const char* label,                     // ��ѡ��ı�ǩ�ı�����ʾ�ڸ�ѡ���Ա�
        const char* identifier,                // ��ѡ���Ψһ��ʶ�����������ֲ�ͬ�ĸ�ѡ��
        bool* state,                           // ָ��ѡ��ǰ״̬��ָ�루true ��ʾѡ�У�false ��ʾδѡ�У�
        const ImVec2& size = ImVec2(20, 20),   // ��ѡ��Ĵ�С��Ĭ��Ϊ 20x20 ����
        ImU32 activeCol = IM_COL32(100, 200, 255, 255),// ��ѡ��ѡ��ʱ����ɫ��Ĭ��Ϊ����ɫ
        ImU32 inactiveCol = IM_COL32(100, 100, 100, 255),// ��ѡ��δѡ��ʱ����ɫ��Ĭ��Ϊ��ɫ
        ImU32 borderCol = IM_COL32(200, 200, 200, 255),// ��ѡ��߿����ɫ��Ĭ��Ϊǳ��ɫ
        ImU32 textCol = IM_COL32(255, 255, 255, 255)   // ��ǩ�ı�����ɫ��Ĭ��Ϊ��ɫ
    ) {

        std::string key = identifier ? identifier : "";
        static std::unordered_map<std::string, SmoothCheckBox> instances;
        auto& instance = instances[key];

        // �״ε���ʱ��ʼ��
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
};//ѡ���

class SmoothToggleBox {
private:
    // ������ť״̬
    float switchPos = 0.0f;
    bool currentState = false;
    bool isAnimating = false;

    // ��ͣ�ͽ���״̬
    float hoverAlpha = 0.0f;
    float focusAnim = 0.0f;
    bool isHovered = false;
    bool isFocused = false;
    bool wasFocused = false;
    bool wasHovered = false;

    // ��������
    float switchSpeed = 5.0f;
    float hoverSpeed = 10.0f;
    float focusSpeed = 8.0f;

    // ��ʽ����
    ImVec2 boxSize = ImVec2(200, 40);
    ImVec2 toggleSize = ImVec2(50, 25);
    float boxRounding = 4.0f;
    float toggleRounding = 12.0f;
    float thumbSizeRatio = 0.8f;
    float thumbPadding = 4.0f;
    float hoverBrightness = 0.2f;
    float highlightIntensity = 0.5f;

    // ��ɫ����
    ImU32 boxBgColor = IM_COL32(50, 50, 50, 200);
    ImU32 activeBgColor = IM_COL32(0, 255, 255, 255);
    ImU32 inactiveBgColor = IM_COL32(100, 100, 100, 255);
    ImU32 thumbActiveColor = IM_COL32(255, 255, 255, 255);
    ImU32 thumbInactiveColor = IM_COL32(180, 180, 180, 255);
    ImU32 textColor = IM_COL32(200, 200, 200, 255);  // Ĭ���ı���ɫ
    ImU32 focusTextColor = IM_COL32(255, 255, 255, 255);  // �����ı���ɫ(����ɫ)

    // ��������
    float SmoothStep(float t) {
        return t * t * (3.0f - 2.0f * t);
    }

    // ���Բ�ֵ
    float ImLerp(float a, float b, float t) {
        return a + (b - a) * t;
    }

    // ��ɫ��ֵ
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

    // Ӧ����ͣЧ��
    ImU32 ApplyHover(ImU32 color, float alpha) {
        if (alpha <= 0.0f) return color;
        ImVec4 c = ImGui::ColorConvertU32ToFloat4(color);
        float factor = hoverBrightness * SmoothStep(alpha);
        c.x = ImMin(c.x + factor, 1.0f);
        c.y = ImMin(c.y + factor, 1.0f);
        c.z = ImMin(c.z + factor, 1.0f);
        return ImGui::ColorConvertFloat4ToU32(c);
    }

    // ����ʵ��
    bool DrawImpl(const char* label, const char* identifier, bool state) {
        ImGui::PushID(identifier);

        // ״̬�仯ʱ��������
        if (state != currentState) {
            currentState = state;
            isAnimating = true;
        }

        ImVec2 p = ImGui::GetCursorScreenPos();
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        // ����Box����
        drawList->AddRectFilled(p, ImVec2(p.x + boxSize.x, p.y + boxSize.y), boxBgColor, boxRounding);

        // ���㲼��
        bool hasLabel = label && label[0] != '\0';
        float textWidth = hasLabel ? ImGui::CalcTextSize(label).x + ImGui::GetStyle().ItemInnerSpacing.x : 0.0f;

        // ������ťλ��(����)
        ImVec2 togglePos = ImVec2(
            p.x + boxSize.x - toggleSize.x - ImGui::GetStyle().ItemInnerSpacing.x,
            p.y + (boxSize.y - toggleSize.y) * 0.5f
        );

        // ������������
        ImGui::InvisibleButton(identifier, boxSize);

        // ���½���״̬
        wasHovered = isHovered;
        wasFocused = isFocused;
        isHovered = ImGui::IsItemHovered();
        isFocused = isHovered; // �����ͣʱ��Ϊ�н���

        // �����ͣ״̬�仯
        if (isHovered != wasHovered) {
            focusAnim = isFocused ? 0.0f : 1.0f; // ���ý��㶯��
        }

        // ���¶���
        float delta = ImGui::GetIO().DeltaTime;

        // ������ť����
        float switchTarget = currentState ? 1.0f : 0.0f;
        switchPos = ImLerp(switchPos, switchTarget, delta * switchSpeed);
        if (fabs(switchPos - switchTarget) < 0.001f) isAnimating = false;

        // ��ͣ����
        float hoverTarget = isHovered ? 1.0f : 0.0f;
        hoverAlpha = ImLerp(hoverAlpha, hoverTarget, delta * hoverSpeed);

        // ���㶯��
        float focusTarget = isFocused ? 1.0f : 0.0f;
        focusAnim = ImLerp(focusAnim, focusTarget, delta * focusSpeed);

        // ������ɫ
        // ʹ�ô���ɫ��Ϊ����״̬�µ��ı���ɫ
        ImU32 currentTextColor = LerpColor(textColor, IM_COL32(255, 255, 255, 255), SmoothStep(focusAnim));

        ImU32 bgColor = currentState ? activeBgColor : inactiveBgColor;
        bgColor = ApplyHover(bgColor, hoverAlpha);
        ImU32 thumbColor = currentState ? thumbActiveColor : thumbInactiveColor;

        // ���ƻ�����ť
        drawList->AddRectFilled(togglePos, ImVec2(togglePos.x + toggleSize.x, togglePos.y + toggleSize.y), bgColor, toggleRounding);

        // ����
        float thumbSize = toggleSize.y * thumbSizeRatio;
        float maxOffset = toggleSize.x - thumbSize - thumbPadding * 2;
        float thumbX = togglePos.x + thumbPadding + maxOffset * switchPos;
        drawList->AddCircleFilled(
            ImVec2(thumbX + thumbSize / 2, togglePos.y + toggleSize.y / 2),
            thumbSize / 2, thumbColor, 32
        );

        // �����ı�(����)
        if (hasLabel) {
            ImVec2 textPos(
                p.x + ImGui::GetStyle().ItemInnerSpacing.x,
                p.y + (boxSize.y - ImGui::GetTextLineHeight()) * 0.5f
            );
            drawList->AddText(textPos, currentTextColor, label);
        }

        // ������
        bool clicked = ImGui::IsItemClicked();
        if (clicked) currentState = !currentState;

        // ȷ����ȷ�Ĳ��ּ���
        ImGui::SetCursorScreenPos(ImVec2(p.x, p.y + boxSize.y + ImGui::GetStyle().ItemSpacing.y));

        ImGui::PopID();
        return clicked;
    }

public:
    static bool ToggleBox(
        const char* label,                             // �л���ı�ǩ�ı�����ʾ���л����Ա�
        const char* identifier,                        // �л����Ψһ��ʶ�����������ֲ�ͬ���л���
        bool* state,                                   // ָ���л���ǰ״̬��ָ�루true ��ʾ������false ��ʾ�رգ�
        const ImVec2& boxSize = ImVec2(200, 40),       // �л���Ĵ�С��Ĭ��Ϊ 200x40 ����
        const ImVec2& toggleSize = ImVec2(50, 25),     // �л���ť�Ĵ�С��Ĭ��Ϊ 50x25 ����
        ImU32 boxBgCol = IM_COL32(50, 50, 50, 200),    // �л��򱳾���ɫ��Ĭ��Ϊ��͸�����ɫ
        ImU32 activeBgCol = IM_COL32(0, 255, 255, 255),// �л�����ʱ�ı�����ɫ��Ĭ��Ϊ����ɫ
        ImU32 inactiveBgCol = IM_COL32(100, 100, 100, 255),// �л���ر�ʱ�ı�����ɫ��Ĭ��Ϊ��ɫ
        ImU32 thumbActiveCol = IM_COL32(255, 255, 255, 255),// �л���ť����ʱ����ɫ��Ĭ��Ϊ��ɫ
        ImU32 thumbInactiveCol = IM_COL32(180, 180, 180, 255),// �л���ť�ر�ʱ����ɫ��Ĭ��Ϊǳ��ɫ
        ImU32 textCol = IM_COL32(200, 200, 200, 255),  // ��ǩ�ı�����ɫ��Ĭ��Ϊǳ��ɫ
        ImU32 focusTextCol = IM_COL32(255, 255, 255, 255)// �л����ý���ʱ�ı�����ɫ��Ĭ��Ϊ��ɫ
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
    // ����״̬
    float openAnim = 0.0f;          // ������򿪶�������(0.0��1.0)
    float scrollAnim = 0.0f;        // ��������������
    float hoverAlpha = 0.0f;        // ��ͣǿ��(0.0��1.0)
    bool isOpen = false;            // �Ƿ��������
    bool isAnimating = false;       // �Ƿ����ڲ��Ŷ���
    float currentScroll = 0.0f;     // ��ǰ����λ��
    float targetScroll = 0.0f;      // Ŀ�����λ��

    // �����ٶ�
    float openSpeed = 10.0f;        // ��/�رն����ٶ�
    float scrollSpeed = 15.0f;      // ���������ٶ�
    float hoverSpeed = 12.0f;       // ��ͣ�����ٶ�

    // ��ʽ����
    ImVec2 size = ImVec2(200, 30);  // ���ؼ���С
    float dropdownRounding = 4.0f;   // ������Բ��
    float itemRounding = 2.0f;      // ѡ��Բ��
    float arrowSize = 8.0f;         // ��ͷ��С
    float checkmarkSize = 12.0f;    // �Թ���С
    float hoverBrightness = 0.2f;   // ��ͣ��������ֵ
    float scrollbarWidth = 6.0f;    // ���������

    // ��ɫ����
    ImU32 bgColor = IM_COL32(50, 50, 50, 255);      // ������ɫ
    ImU32 borderColor = IM_COL32(80, 80, 80, 255);  // �߿���ɫ
    ImU32 textColor = IM_COL32(255, 255, 255, 255); // �ı���ɫ
    ImU32 hoverColor = IM_COL32(70, 70, 70, 255);   // ��ͣ��ɫ
    ImU32 activeColor = IM_COL32(100, 200, 255, 255); // ������ɫ
    ImU32 checkmarkColor = IM_COL32(255, 255, 255, 255); // �Թ���ɫ
    ImU32 scrollbarColor = IM_COL32(100, 100, 100, 150); // ��������ɫ
    ImU32 scrollbarHoverColor = IM_COL32(120, 120, 120, 200); // ��������ͣ��ɫ

    // ��������
    float SmoothStep(float t) {
        return t * t * (3.0f - 2.0f * t);
    }

    // ��ɫ��ֵ
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

    // Ӧ����ͣЧ��
    ImU32 ApplyHover(ImU32 color, float alpha) {
        if (alpha <= 0.0f) return color;
        ImVec4 c = ImGui::ColorConvertU32ToFloat4(color);
        float factor = hoverBrightness * SmoothStep(alpha);
        c.x = ImMin(c.x + factor, 1.0f);
        c.y = ImMin(c.y + factor, 1.0f);
        c.z = ImMin(c.z + factor, 1.0f);
        return ImGui::ColorConvertFloat4ToU32(c);
    }

    // ���ƶԹ�
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

    // ����������ͷ
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

    // ����ʡ�Ժ�
    void DrawEllipsis(ImDrawList* drawList, ImVec2 pos, ImU32 color, float size) {
        float radius = size * 0.1f;
        float spacing = size * 0.3f;

        drawList->AddCircleFilled(ImVec2(pos.x + radius, pos.y + size * 0.5f), radius, color);
        drawList->AddCircleFilled(ImVec2(pos.x + radius + spacing, pos.y + size * 0.5f), radius, color);
        drawList->AddCircleFilled(ImVec2(pos.x + radius + spacing * 2, pos.y + size * 0.5f), radius, color);
    }

    // ����ʵ��
    bool DrawImpl(const char* identifier, const char* items[], bool selected[], int itemCount, int maxVisibleItems = 5) {
        ImGui::PushID(identifier);
        bool valueChanged = false;

        ImVec2 p = ImGui::GetCursorScreenPos();
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImGuiIO& io = ImGui::GetIO();

        // ���ؼ���������
        ImGui::InvisibleButton(identifier, size);
        bool isHovered = ImGui::IsItemHovered();
        bool isClicked = ImGui::IsItemClicked();

        // ���¶���״̬
        if (isClicked) {
            isOpen = !isOpen;
            isAnimating = true;
        }

        // ���´�/�رն���
        float delta = io.DeltaTime;
        float openTarget = isOpen ? 1.0f : 0.0f;
        openAnim = ImLerp(openAnim, openTarget, delta * openSpeed);
        if (fabs(openAnim - openTarget) < 0.001f) isAnimating = false;

        // ������ͣ����
        float hoverTarget = isHovered ? 1.0f : 0.0f;
        hoverAlpha = ImLerp(hoverAlpha, hoverTarget, delta * hoverSpeed);

        // �������ؼ���ɫ
        ImU32 currentBgColor = ApplyHover(bgColor, hoverAlpha);
        ImU32 currentTextColor = textColor;

        // �������ؼ�����
        drawList->AddRectFilled(p, ImVec2(p.x + size.x, p.y + size.y), currentBgColor, dropdownRounding);
        drawList->AddRect(p, ImVec2(p.x + size.x, p.y + size.y), borderColor, dropdownRounding, 0, 1.0f);

        // ���㲢����ѡ������ı�
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

        // �����ı����
        float textWidth = ImGui::CalcTextSize(selectedText.c_str()).x;
        float availableWidth = size.x - arrowSize - ImGui::GetStyle().ItemInnerSpacing.x * 2;

        // �����ı�(��ʡ�ԺŴ���)
        if (textWidth <= availableWidth) {
            ImVec2 textPos(p.x + ImGui::GetStyle().ItemInnerSpacing.x, p.y + (size.y - ImGui::GetTextLineHeight()) * 0.5f);
            drawList->AddText(textPos, currentTextColor, selectedText.c_str());
        }
        else {
            // ���ҿ�����ʾ���ı�����
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

        // ����������ͷ
        ImVec2 arrowPos(p.x + size.x - arrowSize - ImGui::GetStyle().ItemInnerSpacing.x,
            p.y + (size.y - arrowSize) * 0.5f);
        DrawArrow(drawList, arrowPos, currentTextColor, arrowSize, 1.0f, !isOpen);

        // �������߼�
        if (openAnim > 0.0f) {
            float dropdownHeight = ImMin(static_cast<float>(maxVisibleItems) * size.y,
                static_cast<float>(itemCount) * size.y);
            float animHeight = dropdownHeight * SmoothStep(openAnim);

            // ����������λ��
            ImVec2 dropdownPos(p.x, p.y + size.y + 2.0f);
            ImVec2 dropdownSize(size.x, animHeight);

            // ����������ü�����
            ImGui::PushClipRect(dropdownPos, ImVec2(dropdownPos.x + dropdownSize.x, dropdownPos.y + dropdownSize.y), true);

            // ���������򱳾� - ȷ����͸��
            ImU32 opaqueBgColor = (bgColor & IM_COL32_A_MASK) ? bgColor : IM_COL32(50, 50, 50, 255);
            drawList->AddRectFilled(dropdownPos, ImVec2(dropdownPos.x + dropdownSize.x, dropdownPos.y + dropdownSize.y),
                opaqueBgColor, dropdownRounding);
            drawList->AddRect(dropdownPos, ImVec2(dropdownPos.x + dropdownSize.x, dropdownPos.y + dropdownSize.y),
                borderColor, dropdownRounding, 0, 1.0f);

            // �������
            if (ImGui::IsWindowHovered() && io.MouseWheel != 0.0f) {
                targetScroll = ImClamp(targetScroll - io.MouseWheel * 20.0f, 0.0f,
                    ImMax(0.0f, static_cast<float>(itemCount) * size.y - dropdownHeight));
            }

            // ���¹�������
            currentScroll = ImLerp(currentScroll, targetScroll, delta * scrollSpeed);

            // ����ɼ��Χ
            int firstVisible = static_cast<int>(currentScroll / size.y);
            int lastVisible = ImMin(firstVisible + maxVisibleItems + 1, itemCount);

            // ���ƿɼ���
            for (int i = firstVisible; i < lastVisible; ++i) {
                ImVec2 itemPos(dropdownPos.x, dropdownPos.y + i * size.y - currentScroll);

                // �������ɼ���
                if (itemPos.y + size.y < dropdownPos.y || itemPos.y > dropdownPos.y + dropdownSize.y) continue;

                bool itemHovered = ImGui::IsMouseHoveringRect(itemPos, ImVec2(itemPos.x + size.x, itemPos.y + size.y));
                ImU32 itemBgColor = itemHovered ? hoverColor : bgColor;

                // �������
                drawList->AddRectFilled(itemPos, ImVec2(itemPos.x + size.x, itemPos.y + size.y),
                    itemBgColor, itemRounding);

                // ����ѡ��״̬�ĶԹ�
                if (selected[i]) {
                    ImVec2 checkPos(itemPos.x + 5.0f, itemPos.y + (size.y - checkmarkSize) * 0.5f);
                    DrawCheckmark(drawList, checkPos, checkmarkColor, checkmarkSize);
                }

                // �����ı�
                ImVec2 textPos(itemPos.x + checkmarkSize + 10.0f, itemPos.y + (size.y - ImGui::GetTextLineHeight()) * 0.5f);
                drawList->AddText(textPos, textColor, items[i]);

                // ��������
                if (itemHovered && ImGui::IsMouseClicked(0)) {
                    selected[i] = !selected[i];
                    valueChanged = true;
                }
            }

            // ���ƹ�����(�����Ҫ)
            if (itemCount > maxVisibleItems) {
                float scrollRatio = dropdownHeight / (static_cast<float>(itemCount) * size.y);
                float scrollbarHeight = dropdownHeight * scrollRatio;
                float scrollbarPos = (currentScroll / (static_cast<float>(itemCount) * size.y - dropdownHeight)) * (dropdownHeight - scrollbarHeight);

                ImVec2 scrollbarStart(dropdownPos.x + size.x - scrollbarWidth - 2.0f, dropdownPos.y + scrollbarPos);
                ImVec2 scrollbarEnd(scrollbarStart.x + scrollbarWidth, scrollbarStart.y + scrollbarHeight);

                bool scrollbarHovered = ImGui::IsMouseHoveringRect(scrollbarStart, scrollbarEnd);
                ImU32 scrollbarCol = scrollbarHovered ? scrollbarHoverColor : scrollbarColor;

                drawList->AddRectFilled(scrollbarStart, scrollbarEnd, scrollbarCol, scrollbarWidth * 0.5f);

                // ����������϶�
                if (scrollbarHovered && ImGui::IsMouseDown(0)) {
                    float mousePosY = io.MousePos.y - dropdownPos.y;
                    float normalizedPos = mousePosY / dropdownHeight;
                    targetScroll = normalizedPos * (static_cast<float>(itemCount) * size.y - dropdownHeight);
                    targetScroll = ImClamp(targetScroll, 0.0f, static_cast<float>(itemCount) * size.y - dropdownHeight);
                }
            }

            ImGui::PopClipRect();

            // ����ⲿ�ر�������
            if (ImGui::IsMouseClicked(0) && !ImGui::IsItemHovered() &&
                !ImGui::IsMouseHoveringRect(dropdownPos, ImVec2(dropdownPos.x + dropdownSize.x, dropdownPos.y + dropdownSize.y))) {
                isOpen = false;
                isAnimating = true;
            }

            // ��ֹ���²�����Ľ���
            ImGui::SetItemAllowOverlap();
            ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPos().x, ImGui::GetCursorPos().y + animHeight + 2.0f));
        }

        ImGui::PopID();
        return valueChanged;
    }

public:
    // ��̬�ӿ�
    static bool MultiSelect(
        const char* identifier,                     // ��ѡ���Ψһ��ʶ�����������ֲ�ͬ�Ķ�ѡ��
        const char* items[],                        // ѡ�����飬�������п�ѡ����Ŀ
        bool selected[],                            // ѡ��״̬���飬��ʾÿ����Ŀ��ѡ��״̬
        int itemCount,                              // ѡ������Ĵ�С����ѡ�������
        const ImVec2& size = ImVec2(200, 30),       // ��ѡ��Ĵ�С��Ĭ�Ͽ��Ϊ200���أ��߶�Ϊ30����
        int maxVisibleItems = 5,                    // �����ʾ��ѡ��������Ĭ��Ϊ5
        ImU32 bgCol = IM_COL32(50, 50, 50, 255),    // ��ѡ�򱳾���ɫ��Ĭ��Ϊ���ɫ
        ImU32 borderCol = IM_COL32(80, 80, 80, 255),// ��ѡ��߿���ɫ��Ĭ��Ϊ��ɫ
        ImU32 textCol = IM_COL32(255, 255, 255, 255),// �ı���ɫ��Ĭ��Ϊ��ɫ
        ImU32 hoverCol = IM_COL32(70, 70, 70, 255), // ��ͣʱ�ı�����ɫ��Ĭ��Ϊǳ��ɫ
        ImU32 activeCol = IM_COL32(100, 200, 255, 255),// ѡ����ı�����ɫ��Ĭ��Ϊ����ɫ
        ImU32 checkmarkCol = IM_COL32(255, 255, 255, 255),// �Թ���ɫ��Ĭ��Ϊ��ɫ
        float rounding = 4.0f                       // ��ѡ���Բ�ǰ뾶��Ĭ��Ϊ4����
    ) {

        static std::unordered_map<std::string, SmoothMultiSelectDropdown> instances;
        auto& instance = instances[identifier];

        // �״ε���ʱ��ʼ��
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