#include "PluginEditor.h"
#include "BinaryData.h"

namespace {
const juce::Colour bg       (0xff090909);
const juce::Colour panel    (0xff171614);
const juce::Colour panel2   (0xff211d17);
const juce::Colour gold     (0xffffbd45);
const juce::Colour goldLite (0xffffdf91);
const juce::Colour purple   (0xff9c4a8c);
}

// --- AJOUT DU DESTRUCTEUR POUR LE LOOKANDFEEL ---
BaiGSubBassAudioProcessorEditor::~BaiGSubBassAudioProcessorEditor()
{
    setLookAndFeel(nullptr); 
}

BaiGSubBassAudioProcessorEditor::BaiGSubBassAudioProcessorEditor(BaiGSubBassAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // --- APPLICATION DU NOUVEAU DESIGN ---
    setLookAndFeel(&baiGLookAndFeel);

    heroImage = juce::ImageCache::getFromMemory(BinaryData::bass_hero_jpg, BinaryData::bass_hero_jpgSize);
    setSize(1200, 800);

    title.setText("Baï-G Sub Bass", juce::dontSendNotification);
    title.setFont(juce::Font(juce::FontOptions(28.0f, juce::Font::bold)));
    title.setColour(juce::Label::textColourId, goldLite);
    addAndMakeVisible(title);

    subtitle.setText("WORLD CLASS EDITION  •  DEEP / PURE / POWERFUL  •  Baï-G Production", juce::dontSendNotification);
    subtitle.setColour(juce::Label::textColourId, gold);
    subtitle.setFont(juce::Font(juce::FontOptions(13.0f)));
    addAndMakeVisible(subtitle);

    presetBox.addSectionHeading("AFROBEAT");
    const char* names[] = { "Afro Deep Massive", "Afro Groove Bass", "Afro Punch", "Afro Smooth", "Afro Rise", "Afro Nebula" };
    for (int i = 0; i < 64; ++i)
        presetBox.addItem(i < 6 ? names[i] : juce::String("Afrobeat / Amapiano Preset ") + juce::String(i + 1), i + 1);
    const char* family[] = { "Amapiano", "808 & Trap", "Club", "Analog", "Deep & Pure", "Cinematic", "Experimental", "Ethnic & Afro" };
    for (int i = 64; i < 160; ++i)
        presetBox.addItem(juce::String(family[(i - 64) / 12]) + "  •  Preset " + juce::String(i + 1), i + 1);
    presetBox.setSelectedId(1);
    presetBox.onChange = [this] { audioProcessor.loadPreset(presetBox.getSelectedId() - 1); };
    presetBox.setColour(juce::ComboBox::backgroundColourId, panel2);
    presetBox.setColour(juce::ComboBox::outlineColourId, gold.withAlpha(0.8f));
    presetBox.setColour(juce::ComboBox::textColourId, juce::Colours::white);
    presetBox.setColour(juce::ComboBox::arrowColourId, gold);
    addAndMakeVisible(presetBox);

    const std::array<std::pair<const char*, const char*>, 13> defs {{
        {"SUB LEVEL", "sub"}, {"HARMONICS", "harm"}, {"SATURATION", "sat"}, {"PUNCH", "punch"},
        {"RELEASE ms", "release"}, {"GLIDE ms", "glide"}, {"DRIVE", "drive"}, {"CUTOFF Hz", "cutoff"},
        {"OUTPUT", "output"}, {"COMP", "comp"}, {"TONE", "tone"}, {"SUB OCTAVE", "octave"}, {"LFO RATE", "lfoRate"}, {"LFO DEPTH", "lfoDepth"}
    }};

    for (auto d : defs)
    {
        Control c;
        c.slider = std::make_unique<juce::Slider>();
        c.slider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        c.slider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 78, 20);
        c.slider->setColour(juce::Slider::rotarySliderFillColourId, gold);
        c.slider->setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff514332));
        c.slider->setColour(juce::Slider::thumbColourId, goldLite);
        c.slider->setColour(juce::Slider::textBoxTextColourId, goldLite);
        c.slider->setColour(juce::Slider::textBoxBackgroundColourId, bg);
        c.slider->setColour(juce::Slider::textBoxOutlineColourId, gold.withAlpha(0.25f));

        c.label = std::make_unique<juce::Label>();
        c.label->setText(d.first, juce::dontSendNotification);
        c.label->setJustificationType(juce::Justification::centred);
        c.label->setColour(juce::Label::textColourId, juce::Colours::white);
        c.label->setFont(juce::Font(juce::FontOptions(12.0f, juce::Font::bold)));
        c.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, d.second, *c.slider);
        addAndMakeVisible(*c.slider);
        addAndMakeVisible(*c.label);
        controls.push_back(std::move(c));
    }
    startTimerHz(24);
}

void BaiGSubBassAudioProcessorEditor::timerCallback()
{
    meter = 0.88f * meter + 0.12f * (0.15f + 0.12f * (float) std::sin(phase));
    phase += 0.19f;
    repaint();
}

void BaiGSubBassAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(bg);
    auto frame = getLocalBounds().toFloat().reduced(7.0f);
    g.setColour(panel);
    g.fillRoundedRectangle(frame, 15.0f);
    g.setColour(gold.withAlpha(0.85f));
    g.drawRoundedRectangle(frame, 15.0f, 1.4f);

    // Studio hero image, cropped from the approved visual concept.
    if (heroImage.isValid())
        g.drawImage(heroImage, juce::Rectangle<float>(18, 78, 1164, 278), juce::RectanglePlacement::centred);
    g.setColour(gold.withAlpha(0.85f));
    g.drawLine(18.0f, 358.0f, 1182.0f, 358.0f, 2.0f);

    // Control surface.
    g.setColour(panel2);
    g.fillRoundedRectangle(18.0f, 370.0f, 1164.0f, 360.0f, 12.0f);
    g.setColour(gold.withAlpha(0.65f));
    g.drawRoundedRectangle(18.0f, 370.0f, 1164.0f, 360.0f, 12.0f, 1.2f);

    // Left preset bank and central instrument panels.
    g.setColour(bg.withAlpha(0.9f));
    g.fillRoundedRectangle(28.0f, 382.0f, 185.0f, 335.0f, 9.0f);
    g.setColour(gold.withAlpha(0.55f));
    g.drawRoundedRectangle(28.0f, 382.0f, 185.0f, 335.0f, 9.0f, 1.0f);
    g.setColour(goldLite);
    g.setFont(juce::Font(juce::FontOptions(14.0f, juce::Font::bold)));
    g.drawText("PRESET BANK  •  160 SOUNDS", 38, 389, 165, 22, juce::Justification::centred);

    g.setColour(bg.withAlpha(0.8f));
    g.fillRoundedRectangle(225.0f, 382.0f, 825.0f, 335.0f, 9.0f);
    g.setColour(gold.withAlpha(0.35f));
    g.drawRoundedRectangle(225.0f, 382.0f, 825.0f, 335.0f, 9.0f, 1.0f);

    // Live waveform and output meter.
    auto wave = juce::Rectangle<float>(1060.0f, 390.0f, 108.0f, 92.0f);
    g.setColour(bg);
    g.fillRoundedRectangle(wave, 6.0f);
    g.setColour(gold.withAlpha(0.7f));
    g.drawRoundedRectangle(wave, 6.0f, 1.0f);
    juce::Path trace;
    trace.startNewSubPath(wave.getX() + 5.0f, wave.getCentreY());
    for (int i = 1; i < 98; ++i)
    {
        const float x = wave.getX() + 5.0f + (float) i;
        const float y = wave.getCentreY() - std::sin((float) i * 0.09f + phase) * (12.0f + 20.0f * meter);
        trace.lineTo(x, y);
    }
    g.setColour(gold);
    g.strokePath(trace, juce::PathStrokeType(2.0f));
    g.setColour(juce::Colours::white);
    g.setFont(11.0f);
    g.drawText("SUB WAVE", 1060, 484, 108, 18, juce::Justification::centred);

    g.setColour(bg);
    g.fillRoundedRectangle(1060.0f, 510.0f, 108.0f, 190.0f, 7.0f);
    g.setColour(gold.withAlpha(0.55f));
    g.drawRoundedRectangle(1060.0f, 510.0f, 108.0f, 190.0f, 7.0f, 1.0f);
    g.setColour(goldLite);
    g.drawText("OUTPUT", 1060, 517, 108, 18, juce::Justification::centred);
    for (int col = 0; col < 3; ++col)
    {
        for (int i = 0; i < 12; ++i)
        {
            const float h = 9.0f;
            const float x = 1075.0f + col * 26.0f;
            const float y = 680.0f - i * 13.0f;
            g.setColour(i < 7 ? juce::Colour(0xff9bc45a) : (i < 10 ? gold : juce::Colour(0xffd76c3d)));
            g.fillRoundedRectangle(x, y, 15.0f, h, 2.0f);
        }
    }

    g.setColour(gold.withAlpha(0.75f));
    g.drawLine(28.0f, 738.0f, 1172.0f, 738.0f, 1.0f);
    g.setColour(goldLite);
    g.setFont(juce::Font(juce::FontOptions(12.0f, juce::Font::bold)));
    g.drawText("Baï-G Production   •   FEEL THE SUB. BE A LEGEND.", 360, 742, 480, 20, juce::Justification::centred);
}

void BaiGSubBassAudioProcessorEditor::resized()
{
    title.setBounds(30, 8, 380, 40);
    subtitle.setBounds(32, 47, 570, 22);
    presetBox.setBounds(690, 20, 470, 38);

    // Compact two-row control layout, leaving breathing room around every knob.
    for (size_t i = 0; i < controls.size(); ++i)
    {
        const int col = (int) i % 5;
        const int row = (int) i / 5;
        const int x = 235 + col * 160;
        const int y = 398 + row * 105;
        controls[i].label->setBounds(x, y, 145, 18);
        controls[i].slider->setBounds(x + 27, y + 17, 90, 82);
    }
}
