#pragma once
#include "PluginProcessor.h"
#include "BaiGLookAndFeel.h"

class BaiGSubBassAudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit BaiGSubBassAudioProcessorEditor(BaiGSubBassAudioProcessor&);
    ~BaiGSubBassAudioProcessorEditor() override;
    
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    BaiGSubBassAudioProcessor& audioProcessor;
    BaiGLookAndFeel baiGLookAndFeel;

    juce::Image heroImage;
    juce::ComboBox presetBox;
    juce::Label title, subtitle;
    float meter = 0.0f, phase = 0.0f;

    juce::TabbedComponent tabs { juce::TabbedButtonBar::TabsAtLeft };
    juce::Component mainTab, fxTab, modTab, presetsTab; 
    juce::MidiKeyboardState keyboardState;
    juce::MidiKeyboardComponent keyboard { keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard };

    // --- NOS PANNEAUX (GROUPES) ---
    juce::GroupComponent oscGroup { "osc", "OSCILLATOR" };
    juce::GroupComponent filterGroup { "filter", "FILTER" };
    juce::GroupComponent envGroup { "env", "ENVELOPE (AMP)" };
    juce::GroupComponent fxGroup { "fx", "FX SECTION" };
    juce::GroupComponent modGroup { "mod", "LFO / MOD" };

    struct Control {
        std::unique_ptr<juce::Slider> slider;
        std::unique_ptr<juce::Label> label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };
    std::vector<Control> controls;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BaiGSubBassAudioProcessorEditor)
};
