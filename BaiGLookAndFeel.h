#pragma once
#include <JuceHeader.h>

class BaiGLookAndFeel : public juce::LookAndFeel_V4
{
public:
    BaiGLookAndFeel()
    {
        setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffffbd45)); 
        setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff9c4a8c)); 
        setColour(juce::Slider::thumbColourId, juce::Colours::white);
        setColour(juce::Label::textColourId, juce::Colours::white);
        setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff171614));
        setColour(juce::ComboBox::outlineColourId, juce::Colour(0xffffbd45));
        setColour(juce::ComboBox::textColourId, juce::Colours::white);
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, const float rotaryStartAngle,
                           const float rotaryEndAngle, juce::Slider& slider) override
    {
        auto radius = (float) juce::jmin (width / 2, height / 2) - 4.0f;
        auto centreX = (float) x + (float) width  * 0.5f;
        auto centreY = (float) y + (float) height * 0.5f;
        auto rx = centreX - radius;
        auto ry = centreY - radius;
        auto rw = radius * 2.0f;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        g.setColour (juce::Colour(0xff211d17));
        g.fillEllipse (rx, ry, rw, rw);
        
        g.setColour (juce::Colour(0xffffbd45).withAlpha(0.3f));
        g.drawEllipse (rx, ry, rw, rw, 2.0f);

        juce::Path arc;
        arc.addCentredArc(centreX, centreY, radius, radius, 0.0f, rotaryStartAngle, angle, true);
        g.setColour (juce::Colour(0xffffbd45)); 
        g.strokePath(arc, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        juce::Path p;
        auto pointerLength = radius * 0.6f;
        auto pointerThickness = 3.0f;
        p.addRoundedRectangle (-pointerThickness * 0.5f, -radius + 4.0f, pointerThickness, pointerLength, pointerThickness * 0.5f);
        p.applyTransform (juce::AffineTransform::rotation (angle).translated (centreX, centreY));
        g.setColour (juce::Colours::white);
        g.fillPath (p);
    }

    // --- AJOUT POUR DESSINER LES PANNEAUX (BOX) ---
    void drawGroupComponentOutline (juce::Graphics& g, int width, int height, const juce::String& text, const juce::Justification& position, juce::GroupComponent& group) override
    {
        // Fond sombre
        g.setColour(juce::Colour(0xff111111)); 
        g.fillRoundedRectangle(0, 0, (float)width, (float)height, 8.0f);
        
        // Bordure dorée
        g.setColour(juce::Colour(0xffffbd45).withAlpha(0.6f)); 
        g.drawRoundedRectangle(0, 0, (float)width, (float)height, 8.0f, 1.5f);
        
        // Texte du titre
        g.setColour(juce::Colour(0xffffdf91)); 
        g.setFont(juce::Font(juce::FontOptions(14.0f, juce::Font::bold)));
        g.drawText(text, 15, 5, width - 30, 20, juce::Justification::centredLeft);
    }
};
