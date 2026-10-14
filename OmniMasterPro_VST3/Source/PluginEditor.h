#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

class OmniMasterProAudioProcessorEditor : public juce::AudioProcessorEditor,
                                          private juce::Timer
{
public:
    OmniMasterProAudioProcessorEditor (OmniMasterProAudioProcessor&);
    ~OmniMasterProAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void setupKnob (juce::Slider& slider, juce::Label& label, const juce::String& text, const juce::String& paramId);

    OmniMasterProAudioProcessor& audioProcessor;

    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sliderAttachments;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> buttonAttachments;

    // Module Toggle Buttons
    juce::ToggleButton eqToggle { "1. MASTERING EQ" };
    juce::ToggleButton mbToggle { "2. MULTIBAND GLUE" };
    juce::ToggleButton satToggle { "3. HARMONIC EXCITER" };
    juce::ToggleButton imgToggle { "4. M/S IMAGER" };
    juce::ToggleButton limToggle { "5. MAXIMIZER & CLIPPER" };
    juce::ToggleButton bypassToggle { "MASTER BYPASS" };

    // Knobs & Labels
    juce::Slider hpfKnob, lowGainKnob, lmidGainKnob, hmidGainKnob, airGainKnob;
    juce::Label  hpfLbl,  lowGainLbl,  lmidGainLbl,  hmidGainLbl,  airGainLbl;

    juce::Slider mbLowKnob, mbMidKnob, mbHighKnob, mbRatioKnob, mbMakeupKnob;
    juce::Label  mbLowLbl,  mbMidLbl,  mbHighLbl,  mbRatioLbl,  mbMakeupLbl;

    juce::Slider satDriveKnob, satCharKnob, satMixKnob;
    juce::Label  satDriveLbl,  satCharLbl,  satMixLbl;

    juce::Slider imgWidthKnob, imgMonoKnob;
    juce::Label  imgWidthLbl,  imgMonoLbl;

    juce::Slider clipDriveKnob, limPushKnob, limCeilKnob, limRelKnob;
    juce::Label  clipDriveLbl,  limPushLbl,  limCeilLbl,  limRelLbl;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OmniMasterProAudioProcessorEditor)
};
