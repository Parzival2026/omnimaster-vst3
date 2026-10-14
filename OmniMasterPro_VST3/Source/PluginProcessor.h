#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <atomic>

class OmniMasterProAudioProcessor : public juce::AudioProcessor
{
public:
    OmniMasterProAudioProcessor();
    ~OmniMasterProAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Real-time Metering Atomics for Editor UI
    std::atomic<float> inputPeakL { -60.0f }, inputPeakR { -60.0f };
    std::atomic<float> outputPeakL { -60.0f }, outputPeakR { -60.0f };
    std::atomic<float> lufsIntegrated { -24.0f };
    std::atomic<float> grLowDb { 0.0f }, grMidDb { 0.0f }, grHighDb { 0.0f }, grLimiterDb { 0.0f };
    std::atomic<float> stereoCorrelation { 1.0f };

private:
    using Filter = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>;
    using LinkwitzRiley = juce::dsp::LinkwitzRileyFilter<float>;

    double currentSampleRate = 48000.0;

    // Stage 1: 5-Band Mastering Parametric EQ (HPF, Low Shelf, Low-Mid Bell, High-Mid Bell, Air Shelf)
    Filter eqHpf, eqLowShelf, eqLowMidPeak, eqHighMidPeak, eqAirShelf;

    // Stage 2: 3-Band Linkwitz-Riley Crossover + Multiband Compressors
    LinkwitzRiley xoverLowMid1, xoverLowMid2;
    LinkwitzRiley xoverMidHigh1, xoverMidHigh2;
    juce::dsp::Compressor<float> compLow, compMid, compHigh;

    // Envelope followers for GR metering
    float envLow = 0.0f, envMid = 0.0f, envHigh = 0.0f, envLim = 0.0f;
    float dcBlockX1[2] = { 0.0f, 0.0f }, dcBlockY1[2] = { 0.0f, 0.0f };

    // Mono Maker low-pass filter for Mid-Side Sub-Bass
    juce::dsp::IIR::Filter<float> sideHighPass;

    // Stage 5: Brickwall Limiter
    juce::dsp::Limiter<float> brickwallLimiter;

    void updateFilterCoefficients();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OmniMasterProAudioProcessor)
};
