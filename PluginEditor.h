#pragma once
#include "PluginProcessor.h"
class BaiGSubBassAudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit BaiGSubBassAudioProcessorEditor(BaiGSubBassAudioProcessor&);
    ~BaiGSubBassAudioProcessorEditor() override = default;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    void timerCallback() override;
    BaiGSubBassAudioProcessor& audioProcessor;
    juce::Image heroImage;
    juce::ComboBox presetBox;
    juce::Label title, subtitle;
    float meter = 0.0f, phase = 0.0f;
    struct Control {
        std::unique_ptr<juce::Slider> slider;
        std::unique_ptr<juce::Label> label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };
    std::vector<Control> controls;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BaiGSubBassAudioProcessorEditor)
};
