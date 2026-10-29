#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

namespace rui
{
    constexpr int W = 1280, H = 860;

    struct Theme
    {
        juce::Colour bgTop, bgBot, glow, card, cardLine, text, textDim, textFaint, track, knobHi, knobLo, knobEdge, pointer;
        juce::Colour accDelay, accRev, accTape, accMod, accLofi, accMaster, chip, chipText, pillA, pillB, pillText, swatch, off;
        bool light = false;
    };
    const Theme& theme (int vibe);
    juce::Font ui (float size, bool bold = false);
    void drawTracked (juce::Graphics& g, const juce::String& s, float x, float y, float tracking, juce::Justification j);

    class Knob : public juce::Slider
    {
    public:
        Knob (juce::AudioProcessorValueTreeState& a, const juce::String& id, const juce::String& label, float radius,
              const Theme*& themeRef, std::function<juce::Colour (const Theme&)> accent, bool big = false);
        void paint (juce::Graphics&) override;
        void rebuild();
        juce::Point<float> centre() const { return { (float) getWidth() * 0.5f, cy }; }
        static juce::Rectangle<int> boundsFor (float cx, float cy, float radius);
        std::function<juce::String (float)> valueText;
    private:
        juce::String lbl; float r; float cy; bool bigKnob; const Theme*& th; std::function<juce::Colour (const Theme&)> acc;
        juce::Image body; juce::RangedAudioParameter* param;
    };

    // Clickable toggle bound to a bool parameter (pill / lamp / switch styles)
    class TogButton : public juce::Component
    {
    public:
        enum class Style { PillLeft, PillRight, Lamp, Switch };
        TogButton (juce::AudioProcessorValueTreeState& a, const juce::String& id, const juce::String& text, Style s,
                      const Theme*& themeRef, std::function<juce::Colour (const Theme&)> accent = {});
        void paint (juce::Graphics&) override;
        void mouseDown (const juce::MouseEvent&) override;
        bool isOn() const { return param->getValue() > 0.5f; }
    private:
        juce::RangedAudioParameter* param; juce::String txt; Style style; const Theme*& th; std::function<juce::Colour (const Theme&)> acc;
        juce::ParameterAttachment att;
    };

    class ChoicePill : public juce::Component
    {
    public:
        ChoicePill (juce::AudioProcessorValueTreeState& a, const juce::String& id, const Theme*& themeRef, std::function<juce::Colour (const Theme&)> accent);
        void paint (juce::Graphics&) override;
        void mouseDown (const juce::MouseEvent&) override;
    private:
        juce::RangedAudioParameter* param; const Theme*& th; std::function<juce::Colour (const Theme&)> acc; juce::ParameterAttachment att;
    };
}

class RippleEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit RippleEditor (RippleProcessor&);
    ~RippleEditor() override;
    void paint (juce::Graphics&) override {}
    void resized() override;

private:
    struct Content : public juce::Component
    {
        explicit Content (RippleEditor& e) : ed (e) {}
        void paint (juce::Graphics& g) override { ed.paintContent (g); }
        void mouseDown (const juce::MouseEvent& e) override { ed.contentClick (e); }
        RippleEditor& ed;
    };
    friend struct Content;

    void timerCallback() override;
    void paintContent (juce::Graphics&);
    void contentClick (const juce::MouseEvent&);
    void rebuildStatic();
    void drawDisplay (juce::Graphics&);
    void applyPreset (int idx);
    void stepPreset (int delta);
    void setVibe (int v);
    void showPresetMenu();
    juce::String delayDesc() const;
    float rawParam (const char* id) const { return proc.apvts.getRawParameterValue (id)->load(); }

    RippleProcessor& proc;
    Content content { *this };
    const rui::Theme* th = nullptr;
    int vibeIdx = -1;
    juce::Image staticLayer;

    juce::OwnedArray<rui::Knob> knobs;
    juce::OwnedArray<rui::TogButton> toggles;
    juce::OwnedArray<rui::ChoicePill> pills;
    std::vector<std::unique_ptr<juce::SliderParameterAttachment>> attach;
    std::unique_ptr<juce::ParameterAttachment> vibeAtt;
    bool linkGuard = false; float mixPrev[3] { 0, 0, 0 };

    juce::Rectangle<int> presetPill { 500, 32, 280, 32 }, chipBar { 410, 316, 460, 40 }, vibeArea { 836, 34, 110, 30 },
                         modeBar { 748, 758, 270, 20 }, menuBtn { 1166, 34, 30, 30 };
    juce::Rectangle<int> display { 30, 84, 1220, 198 };
    float outShown = 0.f, pulse = 0.f, phase = 0.f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RippleEditor)
};
