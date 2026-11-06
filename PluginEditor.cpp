#include "PluginEditor.h"
#include "BinaryData.h"

namespace {
const juce::Colour bg       (0xff090909);
const juce::Colour panel    (0xff171614);
const juce::Colour gold     (0xffffbd45);
const juce::Colour goldLite (0xffffdf91);
}

BaiGSubBassAudioProcessorEditor::~BaiGSubBassAudioProcessorEditor() { setLookAndFeel(nullptr); }

BaiGSubBassAudioProcessorEditor::BaiGSubBassAudioProcessorEditor(BaiGSubBassAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&baiGLookAndFeel);
    heroImage = juce::ImageCache::getFromMemory(BinaryData::bass_hero_jpg, BinaryData::bass_hero_jpgSize);
    setSize(1200, 950);

    title.setText("Baï-G Sub Bass", juce::dontSendNotification);
    title.setFont(juce::Font(juce::FontOptions(28.0f, juce::Font::bold)));
    title.setColour(juce::Label::textColourId, goldLite);
    addAndMakeVisible(title);

    subtitle.setText("WORLD CLASS EDITION  •  DEEP / PURE / POWERFUL  •  Baï-G Production", juce::dontSendNotification);
    subtitle.setColour(juce::Label::textColourId, gold);
    subtitle.setFont(juce::Font(juce::FontOptions(13.0f)));
    addAndMakeVisible(subtitle);

    tabs.setOutline(0);
    tabs.addTab("Main", juce::Colour(0xff171614), &mainTab, true);
    tabs.addTab("FX", juce::Colour(0xff171614), &fxTab, true);
    tabs.addTab("Mod", juce::Colour(0xff171614), &modTab, true);
    tabs.addTab("Presets", juce::Colour(0xff171614), &presetsTab, true);
    addAndMakeVisible(tabs);

    addAndMakeVisible(keyboard);
    keyboard.setAvailableRange(24, 96);
    keyboard.setKeyWidth(22.0f);

    presetBox.addSectionHeading("AFROBEAT");
    const char* names[] = { "Afro Deep Massive", "Afro Groove Bass", "Afro Punch", "Afro Smooth", "Afro Rise", "Afro Nebula" };
    for (int i = 0; i < 64; ++i) presetBox.addItem(i < 6 ? names[i] : "Preset " + juce::String(i + 1), i + 1);
    for (int i = 64; i < 160; ++i) presetBox.addItem("Preset " + juce::String(i + 1), i + 1);
    presetBox.setSelectedId(1);
    presetBox.onChange = [this] { audioProcessor.loadPreset(presetBox.getSelectedId() - 1); };
    addAndMakeVisible(presetBox);

    // --- AJOUT DES PANNEAUX AU MAIN TAB ---
    mainTab.addAndMakeVisible(oscGroup);
    mainTab.addAndMakeVisible(filterGroup);
    mainTab.addAndMakeVisible(envGroup);
    mainTab.addAndMakeVisible(fxGroup);
    mainTab.addAndMakeVisible(modGroup);

    // --- CRÉATION DES BOUTONS ET AFFECTATION AUX PANNEAUX ---
    struct ControlDef { const char* label; const char* paramId; juce::GroupComponent* group; };
    std::vector<ControlDef> defs = {
        {"SUB LEVEL", "sub", &oscGroup}, {"HARMONICS", "harm", &oscGroup}, {"SUB OCTAVE", "octave", &oscGroup},
        {"CUTOFF Hz", "cutoff", &filterGroup}, {"TONE", "tone", &filterGroup},
        {"PUNCH", "punch", &envGroup}, {"RELEASE ms", "release", &envGroup},
        {"SATURATION", "sat", &fxGroup}, {"DRIVE", "drive", &fxGroup}, {"COMP", "comp", &fxGroup},
        {"LFO RATE", "lfoRate", &modGroup}, {"LFO DEPTH", "lfoDepth", &modGroup}, {"GLIDE ms", "glide", &modGroup}
    };

    for (auto d : defs)
    {
        Control c;
        c.slider = std::make_unique<juce::Slider>();
        c.slider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        c.slider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 78, 20);
        
        c.label = std::make_unique<juce::Label>();
        c.label->setText(d.label, juce::dontSendNotification);
        c.label->setJustificationType(juce::Justification::centred);
        c.label->setColour(juce::Label::textColourId, juce::Colours::white);
        c.label->setFont(juce::Font(juce::FontOptions(12.0f, juce::Font::bold)));
        
        c.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, d.paramId, *c.slider);
        
        // On ajoute le slider et le label DANS le panneau (GroupComponent) correspondant
        d.group->addAndMakeVisible(*c.slider);
        d.group->addAndMakeVisible(*c.label);
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
    
    g.setColour(goldLite);
    g.setFont(juce::Font(juce::FontOptions(12.0f, juce::Font::bold)));
    g.drawText("Baï-G Production   •   FEEL THE SUB. BE A LEGEND.", 360, 850, 480, 20, juce::Justification::centred);
}

void BaiGSubBassAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    
    auto keyboardArea = area.removeFromBottom(120);
    keyboard.setBounds(keyboardArea.reduced(10));

    auto tabsArea = area.removeFromLeft(150);
    tabs.setBounds(tabsArea.reduced(5, 15));

    auto headerArea = area.removeFromTop(120);
    title.setBounds(170, 8, 380, 40);
    subtitle.setBounds(170, 47, 570, 22);
    presetBox.setBounds(690, 20, 470, 38);

    // --- DISPOSITION DES PANNEAUX DANS L'ONGLET MAIN ---
    auto mainBounds = mainTab.getLocalBounds().reduced(10);
    
    // Ligne 1 (Top)
    auto row1 = mainBounds.removeFromTop(180);
    oscGroup.setBounds(row1.removeFromLeft(row1.getWidth() / 3).reduced(5));
    filterGroup.setBounds(row1.removeFromLeft(row1.getWidth() / 2).reduced(5));
    envGroup.setBounds(row1.reduced(5));

    // Ligne 2 (Milieu)
    mainBounds.removeFromTop(10);
    auto row2 = mainBounds.removeFromTop(180);
    modGroup.setBounds(row2.removeFromLeft(row2.getWidth() / 2).reduced(5));
    fxGroup.setBounds(row2.reduced(5));

    // --- DISPOSITION DES BOUTONS À L'INTÉRIEUR DES PANNEAUX ---
    // Petite fonction locale pour placer les boutons côte à côte dans leur boîte
    auto layoutGroup = [this](juce::GroupComponent& group, int startIndex, int numControls) {
        auto bounds = group.getLocalBounds().reduced(10);
        bounds.removeFromTop(25); // Espace pour le titre du panneau
        int width = bounds.getWidth() / numControls;
        for (int i = 0; i < numControls; ++i) {
            auto ctrlArea = bounds.removeFromLeft(width);
            controls[startIndex + i].label->setBounds(ctrlArea.removeFromTop(20));
            controls[startIndex + i].slider->setBounds(ctrlArea.reduced(5));
        }
    };

    layoutGroup(oscGroup, 0, 3);    // Sub Level, Harmonics, Sub Octave
    layoutGroup(filterGroup, 3, 2); // Cutoff, Tone
    layoutGroup(envGroup, 5, 2);    // Punch, Release
    layoutGroup(fxGroup, 7, 3);     // Saturation, Drive, Comp
    layoutGroup(modGroup, 10, 3);   // LFO Rate, LFO Depth, Glide
}
