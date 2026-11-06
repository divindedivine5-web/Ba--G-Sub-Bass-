#include "PluginProcessor.h"
#include "PluginEditor.h"

class BaiGSubBassSound final : public juce::SynthesiserSound
{
public:
    bool appliesToNote(int) override { return true; }
    bool appliesToChannel(int) override { return true; }
};

void SubBassVoice::setParameters(float sub, float harm, float sat, float punch, float releaseMs,
                                 float glideMs, float tone, float subOctave, float lfoRate, float lfoDepth)
{
    level = sub; harmonics = harm; saturation = sat; punchAmount = punch;
    releaseTime = juce::jlimit(0.02f, 3.0f, releaseMs / 1000.0f);
    glideTime = juce::jlimit(0.0f, 1.0f, glideMs / 1000.0f);
    toneAmount = tone; octaveAmount = subOctave;
    lfoRateHz = lfoRate; lfoDepthAmount = lfoDepth;
}
bool SubBassVoice::canPlaySound(juce::SynthesiserSound* s)
{ return dynamic_cast<BaiGSubBassSound*>(s) != nullptr; }
void SubBassVoice::startNote(int note, float velocity, juce::SynthesiserSound*, int)
{
    targetFrequency = juce::MidiMessage::getMidiNoteInHertz(note);
    if (! isVoiceActive()) currentFrequency = targetFrequency;
    noteVelocity = velocity; phase = 0.0; age = 0; releasePosition = 0;
    lfoPhase = 0.0; releasing = false; active = true;
    attackSamples = (int)(getSampleRate() * (0.002 + (1.0f - punchAmount) * 0.035f));
    releaseSamples = (int)(getSampleRate() * releaseTime);
}
void SubBassVoice::stopNote(float, bool tail)
{
    if (tail) releasing = true;
    else { active = false; clearCurrentNote(); }
}
void SubBassVoice::renderNextBlock(juce::AudioBuffer<float>& out, int start, int count)
{
    if (!active || getSampleRate() <= 0.0) return;
    const double sr = getSampleRate();
    for (int i = 0; i < count; ++i)
    {
        if (glideTime > 0.0001f)
            currentFrequency += (targetFrequency - currentFrequency) / juce::jmax(1.0, glideTime * sr);
        else currentFrequency = targetFrequency;
        const float lfo = (float)std::sin(lfoPhase) * lfoDepthAmount;
        const double freq = juce::jlimit(18.0, 1200.0, currentFrequency * (1.0 + lfo * 0.035));
        const float fundamental = (float)std::sin(phase);
        const float triangle = (float)((2.0 / juce::MathConstants<double>::pi) * std::asin(std::sin(phase)));
        const float sampleBase = fundamental * (1.0f - toneAmount * 0.35f) + triangle * toneAmount * 0.35f;
        float sample = sampleBase
            + (float)std::sin(phase * 2.0) * harmonics * 0.32f
            + (float)std::sin(phase * 3.0) * harmonics * 0.12f
            + (float)std::sin(phase * 5.0) * harmonics * 0.035f
            + (float)std::sin(phase * 0.5) * octaveAmount * 0.30f;
        const float drive = 1.0f + saturation * 14.0f;
        sample = std::tanh(sample * drive) / std::tanh(drive);
        const float attack = juce::jmin(1.0f, (float)age / (float)juce::jmax(1, attackSamples));
        float env = attack;
        if (releasing)
        {
            ++releasePosition;
            env *= juce::jmax(0.0f, 1.0f - (float)releasePosition / (float)juce::jmax(1, releaseSamples));
        }
        const float punchEnv = 1.0f + punchAmount * 0.35f *
            std::exp(-(float)age / (float)juce::jmax(1, (int)(sr * 0.045)));
        const float value = sample * env * punchEnv * noteVelocity * level * 0.22f;
        for (int ch = 0; ch < out.getNumChannels(); ++ch) out.addSample(ch, start + i, value);
        phase += juce::MathConstants<double>::twoPi * freq / sr;
        lfoPhase += juce::MathConstants<double>::twoPi * lfoRateHz / sr;
        if (phase >= juce::MathConstants<double>::twoPi) phase = std::fmod(phase, juce::MathConstants<double>::twoPi);
        if (lfoPhase >= juce::MathConstants<double>::twoPi) lfoPhase = std::fmod(lfoPhase, juce::MathConstants<double>::twoPi);
        ++age;
        if (releasing && releasePosition >= releaseSamples) { active = false; clearCurrentNote(); break; }
    }
}

BaiGSubBassAudioProcessor::BaiGSubBassAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
{
    for (int i = 0; i < 8; ++i) synth.addVoice(new SubBassVoice());
    synth.addSound(new BaiGSubBassSound());
}
juce::AudioProcessorValueTreeState::ParameterLayout BaiGSubBassAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
    auto add = [&p](const char* id, const char* name, float lo, float hi, float def, float step = 0.01f)
    {
        p.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{id,1}, name,
            juce::NormalisableRange<float>(lo,hi,step),def));
    };
    add("sub","Sub Level",0,1,0.84f); add("harm","Harmonics",0,1,0.18f);
    add("sat","Saturation",0,1,0.12f); add("punch","Punch",0,1,0.55f);
    add("release","Release",20,1500,250,1); add("glide","Glide",0,500,0,1);
    add("drive","Drive",0,1,0.08f); add("cutoff","Low-pass Cutoff",40,1200,180,1);
    add("output","Output",0,1,0.82f); add("tone","Tone",0,1,0.20f);
    add("octave","Sub Octave",0,1,0.0f); add("lfoRate","LFO Rate",0.05f,12.0f,0.25f,0.01f);
    add("lfoDepth","LFO Depth",0,1,0.0f);
    return {p.begin(),p.end()};
}
bool BaiGSubBassAudioProcessor::isBusesLayoutSupported(const BusesLayout& b) const
{
    auto c=b.getMainOutputChannelSet();
    return c==juce::AudioChannelSet::mono() || c==juce::AudioChannelSet::stereo();
}
void BaiGSubBassAudioProcessor::prepareToPlay(double sr, int block)
{
    synth.setCurrentPlaybackSampleRate(sr);
    juce::dsp::ProcessSpec spec{sr,(juce::uint32)block,(juce::uint32)getTotalNumOutputChannels()};
    limiter.prepare(spec); limiter.setThreshold(-1.0f); limiter.setRelease(80.0f);
    for (auto& f:filters) f.setCoefficients(juce::IIRCoefficients::makeLowPass(sr, 180.0));
}
void BaiGSubBassAudioProcessor::processBlock(juce::AudioBuffer<float>& b, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    b.clear();
    auto val=[this](const char* id){return parameters.getRawParameterValue(id)->load();};
    for(int i=0;i<synth.getNumVoices();++i)
        if(auto* v=dynamic_cast<SubBassVoice*>(synth.getVoice(i)))
            v->setParameters(val("sub"),val("harm"),juce::jlimit(0.0f,1.0f,val("sat")+val("drive")*0.65f),
                val("punch"),val("release"),val("glide"),val("tone"),val("octave"),val("lfoRate"),val("lfoDepth"));
    synth.renderNextBlock(b,midi,0,b.getNumSamples());
    for(int ch=0;ch<juce::jmin(2,b.getNumChannels());++ch)
    {
        filters[ch].setCoefficients(juce::IIRCoefficients::makeLowPass(getSampleRate(), val("cutoff")));
        filters[ch].processSamples(b.getWritePointer(ch),b.getNumSamples());
    }
    b.applyGain(val("output"));
    juce::dsp::AudioBlock<float> block(b);
    limiter.process(juce::dsp::ProcessContextReplacing<float>(block));
    // Keep the sub foundation centered.
    if(b.getNumChannels()>1) b.copyFrom(1,0,b,0,0,b.getNumSamples());
}
juce::AudioProcessorEditor* BaiGSubBassAudioProcessor::createEditor(){return new BaiGSubBassAudioProcessorEditor(*this);}
void BaiGSubBassAudioProcessor::getStateInformation(juce::MemoryBlock& dest)
{
    std::unique_ptr<juce::XmlElement> xml(parameters.copyState().createXml()); copyXmlToBinary(*xml,dest);
}
void BaiGSubBassAudioProcessor::setStateInformation(const void* d,int n)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(d,n));
    if(xml && xml->hasTagName(parameters.state.getType())) parameters.replaceState(juce::ValueTree::fromXml(*xml));
}
void BaiGSubBassAudioProcessor::loadPreset(int index)
{
    struct P { float sub,harm,sat,punch,rel,glide,drive,cutoff,tone,octave; };
    static const P bank[64] = {
        {0.90f,0.04f,0.02f,0.30f,280,0,0.02f,115,0.05f,0.00f},
        {0.94f,0.00f,0.00f,0.35f,350,0,0.00f,100,0.00f,0.00f},
        {0.86f,0.10f,0.05f,0.45f,220,0,0.04f,145,0.10f,0.00f},
        {0.88f,0.16f,0.08f,0.55f,180,0,0.06f,180,0.15f,0.00f},
        {0.92f,0.06f,0.03f,0.70f,420,0,0.03f,130,0.08f,0.05f},
        {0.82f,0.22f,0.12f,0.50f,260,20,0.10f,220,0.25f,0.00f},
        {0.90f,0.03f,0.01f,0.25f,650,0,0.00f,105,0.00f,0.00f},
        {0.85f,0.28f,0.16f,0.60f,300,45,0.14f,260,0.35f,0.05f},
        {0.90f,0.22f,0.12f,0.72f,180,0,0.12f,260,0.25f,0.00f},
        {0.86f,0.30f,0.15f,0.62f,240,0,0.16f,300,0.30f,0.00f},
        {0.92f,0.18f,0.10f,0.82f,160,0,0.10f,240,0.20f,0.00f},
        {0.82f,0.38f,0.22f,0.70f,210,25,0.20f,340,0.45f,0.00f},
        {0.88f,0.12f,0.08f,0.88f,130,0,0.08f,190,0.15f,0.00f},
        {0.84f,0.34f,0.18f,0.58f,280,50,0.18f,320,0.40f,0.10f},
        {0.90f,0.26f,0.14f,0.76f,190,0,0.14f,290,0.32f,0.00f},
        {0.86f,0.42f,0.24f,0.68f,250,30,0.24f,380,0.50f,0.05f},
        {0.94f,0.34f,0.28f,0.88f,480,90,0.30f,520,0.65f,0.15f},
        {0.90f,0.42f,0.32f,0.82f,380,60,0.34f,600,0.70f,0.10f},
        {0.88f,0.30f,0.22f,0.95f,140,0,0.28f,420,0.55f,0.00f},
        {0.92f,0.48f,0.38f,0.78f,320,110,0.40f,680,0.80f,0.15f},
        {0.84f,0.55f,0.42f,0.90f,220,45,0.42f,720,0.85f,0.10f},
        {0.90f,0.36f,0.25f,0.86f,500,0,0.32f,480,0.60f,0.00f},
        {0.86f,0.52f,0.40f,0.72f,260,80,0.44f,750,0.90f,0.20f},
        {0.92f,0.40f,0.30f,0.92f,180,20,0.36f,560,0.68f,0.05f},
        {0.90f,0.18f,0.12f,0.65f,210,0,0.10f,260,0.22f,0.00f},
        {0.86f,0.26f,0.16f,0.78f,160,0,0.14f,320,0.32f,0.00f},
        {0.92f,0.12f,0.08f,0.58f,300,0,0.06f,190,0.12f,0.00f},
        {0.84f,0.32f,0.20f,0.85f,140,0,0.20f,380,0.42f,0.00f},
        {0.90f,0.22f,0.14f,0.72f,230,0,0.12f,290,0.28f,0.00f},
        {0.88f,0.40f,0.26f,0.82f,190,30,0.28f,460,0.55f,0.05f},
        {0.94f,0.08f,0.04f,0.42f,400,0,0.04f,150,0.08f,0.00f},
        {0.86f,0.28f,0.18f,0.66f,260,40,0.18f,350,0.38f,0.05f},
        {0.90f,0.14f,0.08f,0.50f,700,0,0.06f,180,0.15f,0.00f},
        {0.82f,0.36f,0.24f,0.74f,520,100,0.26f,420,0.50f,0.10f},
        {0.88f,0.24f,0.16f,0.60f,330,0,0.14f,300,0.30f,0.00f},
        {0.92f,0.44f,0.30f,0.80f,260,60,0.32f,520,0.62f,0.10f},
        {0.86f,0.30f,0.20f,0.92f,150,0,0.22f,390,0.45f,0.00f},
        {0.90f,0.20f,0.12f,0.56f,420,0,0.10f,250,0.24f,0.00f},
        {0.84f,0.50f,0.34f,0.86f,200,80,0.38f,650,0.78f,0.10f},
        {0.92f,0.10f,0.06f,0.36f,900,0,0.02f,130,0.05f,0.00f},
        {0.88f,0.34f,0.22f,0.70f,360,40,0.22f,410,0.48f,0.05f},
        {0.86f,0.42f,0.30f,0.82f,600,120,0.34f,570,0.66f,0.15f},
        {0.90f,0.16f,0.10f,0.48f,250,0,0.08f,220,0.18f,0.00f},
        {0.84f,0.46f,0.32f,0.90f,180,30,0.36f,610,0.72f,0.10f},
        {0.92f,0.24f,0.14f,0.64f,480,0,0.12f,300,0.30f,0.00f},
        {0.86f,0.38f,0.26f,0.76f,320,70,0.28f,470,0.58f,0.10f},
        {0.90f,0.28f,0.18f,0.58f,220,0,0.16f,340,0.38f,0.00f},
        {0.82f,0.54f,0.40f,0.94f,160,50,0.44f,760,0.92f,0.15f},
        {0.92f,0.06f,0.02f,0.28f,1100,0,0.00f,100,0.00f,0.00f},
        {0.88f,0.32f,0.20f,0.62f,420,30,0.20f,390,0.42f,0.05f},
        {0.86f,0.48f,0.34f,0.84f,280,90,0.38f,630,0.76f,0.12f},
        {0.90f,0.18f,0.10f,0.54f,520,0,0.08f,230,0.18f,0.00f},
        {0.84f,0.40f,0.28f,0.88f,190,40,0.32f,540,0.62f,0.08f},
        {0.92f,0.26f,0.16f,0.68f,360,0,0.14f,330,0.34f,0.00f},
        {0.86f,0.56f,0.42f,0.92f,240,110,0.46f,800,0.95f,0.18f},
        {0.90f,0.22f,0.14f,0.60f,300,20,0.12f,280,0.26f,0.00f},
        {0.84f,0.44f,0.30f,0.80f,460,70,0.34f,590,0.70f,0.10f},
        {0.92f,0.12f,0.06f,0.40f,780,0,0.04f,160,0.08f,0.00f},
        {0.88f,0.36f,0.24f,0.74f,340,50,0.24f,440,0.52f,0.05f},
        {0.86f,0.52f,0.38f,0.90f,220,100,0.42f,700,0.86f,0.14f},
        {0.90f,0.20f,0.12f,0.56f,560,0,0.10f,250,0.22f,0.00f},
        {0.84f,0.46f,0.32f,0.86f,260,60,0.36f,620,0.74f,0.10f},
        {0.92f,0.30f,0.18f,0.72f,400,30,0.18f,370,0.40f,0.05f},
        {0.86f,0.58f,0.44f,0.96f,180,80,0.48f,850,1.0f,0.20f}
    };
    // 64 authored base voicings expanded into 160 nuanced variations.
    const int baseIndex = juce::jlimit(0,63,index % 64);
    const int variation = juce::jmax(0,index / 64);
    P x = bank[baseIndex];
    if (variation > 0)
    {
        const float v = (float)variation;
        x.harm = juce::jlimit(0.0f,1.0f,x.harm + 0.035f*v);
        x.sat = juce::jlimit(0.0f,1.0f,x.sat + 0.025f*v);
        x.punch = juce::jlimit(0.0f,1.0f,x.punch + 0.02f*v);
        x.rel = juce::jlimit(40.0f,1500.0f,x.rel + 35.0f*v);
        x.cutoff = juce::jlimit(60.0f,1200.0f,x.cutoff + 45.0f*v);
        x.tone = juce::jlimit(0.0f,1.0f,x.tone + 0.04f*v);
    }
    auto set=[this](const char* id,float value){auto* q=parameters.getParameter(id);q->setValueNotifyingHost(q->convertTo0to1(value));};
    set("sub",x.sub);set("harm",x.harm);set("sat",x.sat);set("punch",x.punch);
    set("release",x.rel);set("glide",x.glide);set("drive",x.drive);set("cutoff",x.cutoff);
    set("tone",x.tone);set("octave",x.octave);
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new BaiGSubBassAudioProcessor();}
