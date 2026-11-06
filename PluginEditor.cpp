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

BaiGSubBassAudioProcessorEditor::~BaiGSubBassAudioProcessorEditor()
{
    setLookAndFeel(nullptr); 
}

BaiGSubBassAudioProcessorEditor::BaiGSubBassAudioProcessorEditor(BaiGSubBassAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&baiGLookAndFeel);

    heroImage = juce::ImageCache::getFromMemory(BinaryData::bass_hero_jpg, BinaryData::bass_hero_jpgSize);
    setSize(1200, 950); // Augmenté en hauteur pour faire de la place au clavier

    title.setText("Baï-G Sub Bass", juce::dontSendNotification);
    title.setFont(juce::Font(juce::FontOptions(28.0f, juce::Font::bold)));
    title.setColour(juce::Label::textColourId, goldLite);
    addAndMakeVisible(title);

    subtitle.setText("WORLD CLASS EDITION  •  DEEP / PURE / POWERFUL  •  Baï-G Production", juce::dontSendNotification);
    subtitle.setColour(juce::Label::textColourId, gold);
    subtitle.setFont(juce::Font(juce::FontOptions(13.0f)));
    addAndMakeVisible(subtitle);

    // --- CONFIGURATION DES ONGLETS ---
    tabs.setOutline(0);
    tabs.addTab("Main", juce::Colour(0xff171614), &mainTab, true);
    tabs.addTab("FX", juce::Colour(0xff171614), &fxTab, true);
    tabs.addTab("Mod", juce::Colour(0xff171614), &modTab, true);
    tabs.addTab("Presets", juce::Colour(0xff171614), &presetsTab, true);
    addAndMakeVisible(tabs);

    // --- CONFIGURATION DU CLAVIER ---
    addAndMakeVisible(keyboard);
    keyboard.setAvailableRange(24, 96);
    keyboard.setKeyWidth(22.0f);
    
    // Connexion basique du clavier (pour tester visuellement)
    keyboardState.addListener(&audioProcessor); // Note: nécessite que le processeur accepte les MIDI

    presetBox.addSectionHeading("AFROBEAT");
    const char* names[] = { "Afro Deep Massive", "Afro Groove Bass", "Afro Punch", "Afro Smooth", "Afro Rise", "Afro Nebula" };
    for (int i = 0; i < 64; ++i)
        presetBox.addItem(i < 6 ? names[i] : juce::String("Afrobeat / Amapiano Preset ") + juce::String(i + 1), i + 1);
    const char* family[] = { "Amapiano", "808 & Trap", "Club", "Analog", "Deep & Pure", "Cinematic", "Experimental", "Ethnic & Afro" };
    for (int i = 64; i < 160; ++i)
        presetBox.addItem(juce::String(family[(i - 64) / 12]) + "  •  Preset " + juce::String(i + 1), i + 1);
    presetBox.setSelectedId(1);
    presetBox.onChange = [this] { audioProcessor.loadPreset(presetBox.getSelectedId() - 1); };
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
        c.label = std::make_unique<juce::Label>();
        c.label->setText(d.first, juce::dontSendNotification);
        c.label->setJustificationType(juce::Justification::centred);
        c.label->setColour(juce::Label::textColourId, juce::Colours::white);
        c.label->setFont(juce::Font(juce::FontOptions(12.0f, juce::Font::bold)));
        c.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, d.second, *c.slider);
        
        // Note: On les ajoute au mainTab pour le moment
        mainTab.addAndMakeVisible(*c.slider);
        mainTab.addAndMakeVisible(*c.label);
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

    if (heroImage.isValid())
        g.drawImage(heroImage, juce::Rectangle<float>(170, 78, 1000, 200), juce::RectanglePlacement::centred);
    
    g.setColour(gold.withAlpha(0.85f));
    g.drawLine(160.0f, 290.0f, 1182.0f, 290.0f, 2.0f);

    g.setColour(goldLite);
    g.setFont(juce::Font(juce::FontOptions(12.0f, juce::Font::bold)));
    g.drawText("Baï-G Production   •   FEEL THE SUB. BE A LEGEND.", 360, 850, 480, 20, juce::Justification::centred);
}

void BaiGSubBassAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    
    // 1. Placer le clavier tout en bas
    auto keyboardArea = area.removeFromBottom(120);
    keyboard.setBounds(keyboardArea.reduced(10));

    // 2. Placer le menu des onglets à gauche
    auto tabsArea = area.removeFromLeft(150);
    tabs.setBounds(tabsArea.reduced(5, 15));

    // 3. Placer le reste (Header)
    auto headerArea = area.removeFromTop(120);
    title.setBounds(170, 8, 380, 40);
    subtitle.setBounds(170, 47, 570, 22);
    presetBox.setBounds(690, 20, 470, 38);

    // 4. Disposition des boutons à l'intérieur de l'onglet "Main"
    auto mainBounds = mainTab.getLocalBounds();
    for (size_t i = 0; i < controls.size(); ++i)
    {
        const int col = (int) i % 5;
        const int row = (int) i / 5;
        const int x = 20 + col * 160; // Ajusté pour l'onglet
        const int y = 20 + row * 105; // Ajusté pour l'onglet
        controls[i].label->setBounds(x, y, 145, 18);
        controls[i].slider->setBounds(x + 27, y + 17, 90, 82);
    }
}
