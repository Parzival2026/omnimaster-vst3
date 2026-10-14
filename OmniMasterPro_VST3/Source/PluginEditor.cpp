#include "PluginProcessor.h"
#include "PluginEditor.h"

OmniMasterProAudioProcessorEditor::OmniMasterProAudioProcessorEditor (OmniMasterProAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (1080, 620);

    auto addToggle = [this] (juce::ToggleButton& btn, const juce::String& paramID) {
        addAndMakeVisible (btn);
        buttonAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
            audioProcessor.apvts, paramID, btn));
    };

    addToggle (eqToggle, "EQ_ENABLE");
    addToggle (mbToggle, "MB_ENABLE");
    addToggle (satToggle, "SAT_ENABLE");
    addToggle (imgToggle, "IMG_ENABLE");
    addToggle (limToggle, "LIM_ENABLE");
    addToggle (bypassToggle, "BYPASS");

    // Stage 1 Knobs
    setupKnob (hpfKnob, hpfLbl, "Sub Cut Hz", "EQ_HPF_FREQ");
    setupKnob (lowGainKnob, lowGainLbl, "Low Shelf dB", "EQ_LOW_GAIN");
    setupKnob (lmidGainKnob, lmidGainLbl, "Mud 320Hz dB", "EQ_LMID_GAIN");
    setupKnob (hmidGainKnob, hmidGainLbl, "Presence dB", "EQ_HMID_GAIN");
    setupKnob (airGainKnob, airGainLbl, "Air 12kHz dB", "EQ_AIR_GAIN");

    // Stage 2 Knobs
    setupKnob (mbLowKnob, mbLowLbl, "Low Thresh", "MB_LOW_THRESH");
    setupKnob (mbMidKnob, mbMidLbl, "Mid Thresh", "MB_MID_THRESH");
    setupKnob (mbHighKnob, mbHighLbl, "High Thresh", "MB_HIGH_THRESH");
    setupKnob (mbRatioKnob, mbRatioLbl, "Comp Ratio", "MB_RATIO");
    setupKnob (mbMakeupKnob, mbMakeupLbl, "Glue Makeup", "MB_MAKEUP");

    // Stage 3 Knobs
    setupKnob (satDriveKnob, satDriveLbl, "Drive %", "SAT_DRIVE");
    setupKnob (satCharKnob, satCharLbl, "Tube/Tape", "SAT_EVEN_ODD");
    setupKnob (satMixKnob, satMixLbl, "Mix %", "SAT_MIX");

    // Stage 4 Knobs
    setupKnob (imgWidthKnob, imgWidthLbl, "Width %", "IMG_WIDTH");
    setupKnob (imgMonoKnob, imgMonoLbl, "Mono Bass Hz", "IMG_MONO_BASS");

    // Stage 5 Knobs
    setupKnob (clipDriveKnob, clipDriveLbl, "Soft Clip %", "CLIP_DRIVE");
    setupKnob (limPushKnob, limPushLbl, "Maximizer dB", "LIM_PUSH");
    setupKnob (limCeilKnob, limCeilLbl, "Ceiling dBTP", "LIM_CEILING");
    setupKnob (limRelKnob, limRelLbl, "Release ms", "LIM_RELEASE");

    startTimerHz (30);
}

OmniMasterProAudioProcessorEditor::~OmniMasterProAudioProcessorEditor()
{
    stopTimer();
}

void OmniMasterProAudioProcessorEditor::setupKnob (juce::Slider& slider, juce::Label& label,
                                                   const juce::String& text, const juce::String& paramId)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 68, 18);
    slider.setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (0xfff59e0b));
    slider.setColour (juce::Slider::thumbColourId, juce::Colour (0xff38bdf8));
    addAndMakeVisible (slider);

    label.setText (text, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setFont (juce::Font (12.0f, juce::Font::bold));
    addAndMakeVisible (label);

    sliderAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        audioProcessor.apvts, paramId, slider));
}

void OmniMasterProAudioProcessorEditor::timerCallback()
{
    repaint();
}

void OmniMasterProAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff0b0f19));

    // Top Header Bar
    g.setColour (juce::Colour (0xff111827));
    g.fillRect (0, 0, getWidth(), 54);
    g.setColour (juce::Colour (0xfff59e0b));
    g.setFont (juce::Font (20.0f, juce::Font::bold));
    g.drawText ("OMNIMASTER PRO // ALL-IN-ONE MIXING & MASTERING CONSOLE", 20, 12, 620, 30, juce::Justification::centredLeft);

    // Live LUFS & Peak Readout in Header
    float lufs = audioProcessor.lufsIntegrated.load();
    float outPeak = juce::jmax (audioProcessor.outputPeakL.load(), audioProcessor.outputPeakR.load());
    float corr = audioProcessor.stereoCorrelation.load();

    g.setColour (juce::Colour (0xff38bdf8));
    g.setFont (juce::Font (14.0f, juce::Font::bold));
    juce::String meterText = juce::String::formatted ("LUFS: %.1f   |   TRUE PEAK: %.1f dBTP   |   PHASE CORR: %+.2f   |   LIM GR: %.1f dB",
                                                      lufs, outPeak, corr, audioProcessor.grLimiterDb.load());
    g.drawText (meterText, 520, 12, 410, 30, juce::Justification::centredRight);

    // Draw 5 Module Cards
    const int cardY = 68;
    const int cardH = 532;
    const int widths[5] = { 210, 210, 195, 195, 210 };
    int x = 15;
    for (int i = 0; i < 5; ++i)
    {
        g.setColour (juce::Colour (0xff161f30));
        g.fillRoundedRectangle ((float)x, (float)cardY, (float)widths[i], (float)cardH, 8.0f);
        g.setColour (juce::Colour (0xff2d3748));
        g.drawRoundedRectangle ((float)x, (float)cardY, (float)widths[i], (float)cardH, 8.0f, 1.5f);
        x += widths[i] + 10;
    }
}

void OmniMasterProAudioProcessorEditor::resized()
{
    bypassToggle.setBounds (getWidth() - 145, 12, 130, 30);

    const int cardY = 76;
    const int widths[5] = { 210, 210, 195, 195, 210 };
    int x0 = 15;
    int x1 = x0 + widths[0] + 10;
    int x2 = x1 + widths[1] + 10;
    int x3 = x2 + widths[2] + 10;
    int x4 = x3 + widths[3] + 10;

    eqToggle.setBounds  (x0 + 12, cardY, 180, 26);
    mbToggle.setBounds  (x1 + 12, cardY, 180, 26);
    satToggle.setBounds (x2 + 12, cardY, 175, 26);
    imgToggle.setBounds (x3 + 12, cardY, 175, 26);
    limToggle.setBounds (x4 + 12, cardY, 185, 26);

    auto placeKnob = [] (juce::Slider& s, juce::Label& l, int cx, int cy) {
        l.setBounds (cx, cy, 90, 18);
        s.setBounds (cx, cy + 18, 90, 95);
    };

    // Column 1: EQ
    placeKnob (hpfKnob, hpfLbl, x0 + 12, 115);
    placeKnob (lowGainKnob, lowGainLbl, x0 + 108, 115);
    placeKnob (lmidGainKnob, lmidGainLbl, x0 + 12, 245);
    placeKnob (hmidGainKnob, hmidGainLbl, x0 + 108, 245);
    placeKnob (airGainKnob, airGainLbl, x0 + 60, 375);

    // Column 2: Multiband
    placeKnob (mbLowKnob, mbLowLbl, x1 + 12, 115);
    placeKnob (mbMidKnob, mbMidLbl, x1 + 108, 115);
    placeKnob (mbHighKnob, mbHighLbl, x1 + 12, 245);
    placeKnob (mbRatioKnob, mbRatioLbl, x1 + 108, 245);
    placeKnob (mbMakeupKnob, mbMakeupLbl, x1 + 60, 375);

    // Column 3: Harmonic Exciter
    placeKnob (satDriveKnob, satDriveLbl, x2 + 52, 125);
    placeKnob (satCharKnob, satCharLbl, x2 + 52, 260);
    placeKnob (satMixKnob, satMixLbl, x2 + 52, 395);

    // Column 4: Stereo Imager
    placeKnob (imgWidthKnob, imgWidthLbl, x3 + 52, 160);
    placeKnob (imgMonoKnob, imgMonoLbl, x3 + 52, 320);

    // Column 5: Limiter
    placeKnob (clipDriveKnob, clipDriveLbl, x4 + 12, 130);
    placeKnob (limPushKnob, limPushLbl, x4 + 108, 130);
    placeKnob (limCeilKnob, limCeilLbl, x4 + 12, 285);
    placeKnob (limRelKnob, limRelLbl, x4 + 108, 285);
}
