#pragma once
#include <JuceHeader.h>

class SubBassVoice final : public juce::SynthesiserVoice
{
public:
    void setParameters(float sub, float harm, float sat, float punch, float releaseMs,
                       float glideMs, float tone, float subOctave, float lfoRate, float lfoDepth);
    bool canPlaySound(juce::SynthesiserSound*) override;
    void startNote(int, float, juce::SynthesiserSound*, int) override;
    void stopNote(float, bool) override;
    void pitchWheelMoved(int) override {}
    void controllerMoved(int, int) override {}
    void renderNextBlock(juce::AudioBuffer<float>&, int, int) override;

private:
    double phase = 0.0, currentFrequency = 55.0, targetFrequency = 55.0, lfoPhase = 0.0;
    int age = 0, attackSamples = 100, releaseSamples = 22050, releasePosition = 0;
    float noteVelocity = 0.8f, level = 0.8f, harmonics = 0.15f, saturation = 0.1f;
    float punchAmount = 0.5f, releaseTime = 0.25f, glideTime = 0.0f;
    float toneAmount = 0.2f, octaveAmount = 0.0f, lfoRateHz = 0.25f, lfoDepthAmount = 0.0f;
    bool active = false, releasing = false;
};

class BaiGSubBassAudioProcessor final : public juce::AudioProcessor
{
public:
    BaiGSubBassAudioProcessor();
    ~BaiGSubBassAudioProcessor() override = default;
    void prepareToPlay(double, int) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 4.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    juce::AudioProcessorValueTreeState parameters;
    void loadPreset(int);
private:
    juce::Synthesiser synth;
    juce::IIRFilter filters[2];
    juce::dsp::Limiter<float> limiter;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BaiGSubBassAudioProcessor)
};
