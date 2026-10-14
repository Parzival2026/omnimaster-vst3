#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

OmniMasterProAudioProcessor::OmniMasterProAudioProcessor()
    : AudioProcessor (BusesProperties()
                      .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

OmniMasterProAudioProcessor::~OmniMasterProAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout OmniMasterProAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Global & Bypass
    params.push_back (std::make_unique<juce::AudioParameterBool> ("BYPASS", "Master Bypass", false));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("IN_GAIN", "Input Gain", juce::NormalisableRange<float> (-18.0f, 18.0f, 0.1f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("PHASE_INV", "Phase Invert", false));

    // Stage 1: 5-Band Mastering EQ
    params.push_back (std::make_unique<juce::AudioParameterBool> ("EQ_ENABLE", "EQ Enable", true));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("EQ_HPF_FREQ", "Sub Cut Freq", juce::NormalisableRange<float> (15.0f, 120.0f, 1.0f, 0.5f), 28.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("EQ_LOW_GAIN", "Low Shelf Gain", juce::NormalisableRange<float> (-12.0f, 12.0f, 0.1f), 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("EQ_LOW_FREQ", "Low Shelf Freq", juce::NormalisableRange<float> (40.0f, 250.0f, 1.0f, 0.5f), 85.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("EQ_LMID_GAIN", "Mud Bell Gain", juce::NormalisableRange<float> (-12.0f, 12.0f, 0.1f), -1.2f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("EQ_LMID_FREQ", "Mud Bell Freq", juce::NormalisableRange<float> (150.0f, 800.0f, 1.0f, 0.5f), 320.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("EQ_HMID_GAIN", "Presence Bell Gain", juce::NormalisableRange<float> (-12.0f, 12.0f, 0.1f), 1.5f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("EQ_HMID_FREQ", "Presence Bell Freq", juce::NormalisableRange<float> (1000.0f, 6000.0f, 1.0f, 0.5f), 3200.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("EQ_AIR_GAIN", "Air Shelf Gain", juce::NormalisableRange<float> (-12.0f, 12.0f, 0.1f), 2.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("EQ_AIR_FREQ", "Air Shelf Freq", juce::NormalisableRange<float> (8000.0f, 18000.0f, 10.0f, 0.5f), 12000.0f));

    // Stage 2: 3-Band Multiband Dynamics
    params.push_back (std::make_unique<juce::AudioParameterBool> ("MB_ENABLE", "Multiband Enable", true));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("MB_XOVER_LOW", "Low/Mid X-Over", juce::NormalisableRange<float> (60.0f, 400.0f, 1.0f, 0.5f), 160.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("MB_XOVER_HIGH", "Mid/High X-Over", juce::NormalisableRange<float> (1200.0f, 6000.0f, 10.0f, 0.5f), 2800.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("MB_LOW_THRESH", "Low Comp Thresh", juce::NormalisableRange<float> (-40.0f, 0.0f, 0.5f), -18.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("MB_MID_THRESH", "Mid Comp Thresh", juce::NormalisableRange<float> (-40.0f, 0.0f, 0.5f), -16.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("MB_HIGH_THRESH", "High Comp Thresh", juce::NormalisableRange<float> (-40.0f, 0.0f, 0.5f), -20.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("MB_RATIO", "Multiband Ratio", juce::NormalisableRange<float> (1.2f, 10.0f, 0.1f), 2.5f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("MB_MAKEUP", "Glue Makeup Gain", juce::NormalisableRange<float> (0.0f, 12.0f, 0.1f), 2.0f));

    // Stage 3: Analog Harmonic Exciter (Tape / Tube)
    params.push_back (std::make_unique<juce::AudioParameterBool> ("SAT_ENABLE", "Exciter Enable", true));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("SAT_DRIVE", "Harmonic Drive", juce::NormalisableRange<float> (0.0f, 100.0f, 0.5f), 25.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("SAT_EVEN_ODD", "Tube/Tape Character", juce::NormalisableRange<float> (0.0f, 100.0f, 1.0f), 40.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("SAT_MIX", "Exciter Mix", juce::NormalisableRange<float> (0.0f, 100.0f, 1.0f), 50.0f));

    // Stage 4: Mid/Side Stereo Imager & Mono Bass Maker
    params.push_back (std::make_unique<juce::AudioParameterBool> ("IMG_ENABLE", "Imager Enable", true));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("IMG_WIDTH", "Stereo Width %", juce::NormalisableRange<float> (0.0f, 200.0f, 1.0f), 115.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("IMG_MONO_BASS", "Mono Bass Cutoff", juce::NormalisableRange<float> (20.0f, 300.0f, 1.0f, 0.5f), 120.0f));

    // Stage 5: Soft Clipper & True-Peak Brickwall Maximizer
    params.push_back (std::make_unique<juce::AudioParameterBool> ("LIM_ENABLE", "Limiter Enable", true));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("CLIP_DRIVE", "Soft Clip %", juce::NormalisableRange<float> (0.0f, 100.0f, 1.0f), 20.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("LIM_PUSH", "Maximizer Push dB", juce::NormalisableRange<float> (0.0f, 18.0f, 0.1f), 3.5f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("LIM_CEILING", "Output Ceiling dBTP", juce::NormalisableRange<float> (-6.0f, 0.0f, 0.1f), -0.8f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("LIM_RELEASE", "Limiter Release ms", juce::NormalisableRange<float> (10.0f, 500.0f, 1.0f, 0.5f), 85.0f));

    return { params.begin(), params.end() };
}

const juce::String OmniMasterProAudioProcessor::getName() const { return JucePlugin_Name; }
bool OmniMasterProAudioProcessor::acceptsMidi() const { return false; }
bool OmniMasterProAudioProcessor::producesMidi() const { return false; }
bool OmniMasterProAudioProcessor::isMidiEffect() const { return false; }
double OmniMasterProAudioProcessor::getTailLengthSeconds() const { return 0.05; }
int OmniMasterProAudioProcessor::getNumPrograms() { return 1; }
int OmniMasterProAudioProcessor::getCurrentProgram() { return 0; }
void OmniMasterProAudioProcessor::setCurrentProgram (int) {}
const juce::String OmniMasterProAudioProcessor::getProgramName (int) { return "Default Mastering Strip"; }
void OmniMasterProAudioProcessor::changeProgramName (int, const juce::String&) {}

void OmniMasterProAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
    spec.numChannels = 2;

    eqHpf.prepare (spec);
    eqLowShelf.prepare (spec);
    eqLowMidPeak.prepare (spec);
    eqHighMidPeak.prepare (spec);
    eqAirShelf.prepare (spec);

    xoverLowMid1.prepare (spec);
    xoverLowMid1.setType (juce::dsp::LinkwitzRileyFilterType::lowpass);
    xoverLowMid2.prepare (spec);
    xoverLowMid2.setType (juce::dsp::LinkwitzRileyFilterType::highpass);

    xoverMidHigh1.prepare (spec);
    xoverMidHigh1.setType (juce::dsp::LinkwitzRileyFilterType::lowpass);
    xoverMidHigh2.prepare (spec);
    xoverMidHigh2.setType (juce::dsp::LinkwitzRileyFilterType::highpass);

    compLow.prepare (spec);
    compLow.setAttack (25.0f);
    compLow.setRelease (140.0f);

    compMid.prepare (spec);
    compMid.setAttack (12.0f);
    compMid.setRelease (90.0f);

    compHigh.prepare (spec);
    compHigh.setAttack (5.0f);
    compHigh.setRelease (55.0f);

    juce::dsp::ProcessSpec monoSpec = spec;
    monoSpec.numChannels = 1;
    sideHighPass.prepare (monoSpec);

    brickwallLimiter.prepare (spec);
    updateFilterCoefficients();
}

void OmniMasterProAudioProcessor::releaseResources() {}

bool OmniMasterProAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo()
        && layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void OmniMasterProAudioProcessor::updateFilterCoefficients()
{
    if (currentSampleRate <= 0.0) return;

    float hpfFreq  = apvts.getRawParameterValue ("EQ_HPF_FREQ")->load();
    float lowGain  = juce::Decibels::decibelsToGain (apvts.getRawParameterValue ("EQ_LOW_GAIN")->load());
    float lowFreq  = apvts.getRawParameterValue ("EQ_LOW_FREQ")->load();
    float lmidGain = juce::Decibels::decibelsToGain (apvts.getRawParameterValue ("EQ_LMID_GAIN")->load());
    float lmidFreq = apvts.getRawParameterValue ("EQ_LMID_FREQ")->load();
    float hmidGain = juce::Decibels::decibelsToGain (apvts.getRawParameterValue ("EQ_HMID_GAIN")->load());
    float hmidFreq = apvts.getRawParameterValue ("EQ_HMID_FREQ")->load();
    float airGain  = juce::Decibels::decibelsToGain (apvts.getRawParameterValue ("EQ_AIR_GAIN")->load());
    float airFreq  = apvts.getRawParameterValue ("EQ_AIR_FREQ")->load();

    *eqHpf.state         = *juce::dsp::IIR::Coefficients<float>::makeHighPass (currentSampleRate, hpfFreq, 0.707f);
    *eqLowShelf.state    = *juce::dsp::IIR::Coefficients<float>::makeLowShelf (currentSampleRate, lowFreq, 0.707f, lowGain);
    *eqLowMidPeak.state  = *juce::dsp::IIR::Coefficients<float>::makePeakFilter (currentSampleRate, lmidFreq, 1.2f, lmidGain);
    *eqHighMidPeak.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter (currentSampleRate, hmidFreq, 1.1f, hmidGain);
    *eqAirShelf.state    = *juce::dsp::IIR::Coefficients<float>::makeHighShelf (currentSampleRate, airFreq, 0.707f, airGain);

    float xoLow  = apvts.getRawParameterValue ("MB_XOVER_LOW")->load();
    float xoHigh = apvts.getRawParameterValue ("MB_XOVER_HIGH")->load();
    xoverLowMid1.setCutoffFrequency (xoLow);
    xoverLowMid2.setCutoffFrequency (xoLow);
    xoverMidHigh1.setCutoffFrequency (xoHigh);
    xoverMidHigh2.setCutoffFrequency (xoHigh);

    float mbRatio = apvts.getRawParameterValue ("MB_RATIO")->load();
    compLow.setThreshold (apvts.getRawParameterValue ("MB_LOW_THRESH")->load());
    compLow.setRatio (mbRatio);
    compMid.setThreshold (apvts.getRawParameterValue ("MB_MID_THRESH")->load());
    compMid.setRatio (mbRatio);
    compHigh.setThreshold (apvts.getRawParameterValue ("MB_HIGH_THRESH")->load());
    compHigh.setRatio (mbRatio);

    float monoBassFreq = apvts.getRawParameterValue ("IMG_MONO_BASS")->load();
    sideHighPass.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass (currentSampleRate, monoBassFreq, 0.707f);

    brickwallLimiter.setThreshold (apvts.getRawParameterValue ("LIM_CEILING")->load());
    brickwallLimiter.setRelease (apvts.getRawParameterValue ("LIM_RELEASE")->load());
}

void OmniMasterProAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int numChannels = buffer.getNumChannels();
    const int numSamples  = buffer.getNumSamples();
    if (numChannels < 2 || numSamples == 0) return;

    updateFilterCoefficients();

    // Measure input peaks
    float inL = buffer.getMagnitude (0, 0, numSamples);
    float inR = buffer.getMagnitude (1, 0, numSamples);
    inputPeakL.store (juce::Decibels::gainToDecibels (inL, -60.0f));
    inputPeakR.store (juce::Decibels::gainToDecibels (inR, -60.0f));

    if (apvts.getRawParameterValue ("BYPASS")->load() > 0.5f)
    {
        outputPeakL.store (inputPeakL.load());
        outputPeakR.store (inputPeakR.load());
        grLowDb.store (0.0f); grMidDb.store (0.0f); grHighDb.store (0.0f); grLimiterDb.store (0.0f);
        return;
    }

    // Input Gain & Phase Invert
    float inGainLin = juce::Decibels::decibelsToGain (apvts.getRawParameterValue ("IN_GAIN")->load());
    if (apvts.getRawParameterValue ("PHASE_INV")->load() > 0.5f)
        inGainLin = -inGainLin;
    buffer.applyGain (inGainLin);

    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing<float> context (block);

    // STAGE 1: 5-Band Mastering Parametric EQ
    if (apvts.getRawParameterValue ("EQ_ENABLE")->load() > 0.5f)
    {
        eqHpf.process (context);
        eqLowShelf.process (context);
        eqLowMidPeak.process (context);
        eqHighMidPeak.process (context);
        eqAirShelf.process (context);
    }

    // STAGE 2: 3-Band Multiband Compression
    if (apvts.getRawParameterValue ("MB_ENABLE")->load() > 0.5f)
    {
        juce::AudioBuffer<float> lowBuf, midHighBuf, midBuf, highBuf;
        lowBuf.makeCopyOf (buffer);
        midHighBuf.makeCopyOf (buffer);

        juce::dsp::AudioBlock<float> lowBlock (lowBuf);
        juce::dsp::AudioBlock<float> midHighBlock (midHighBuf);
        xoverLowMid1.process (juce::dsp::ProcessContextReplacing<float> (lowBlock));
        xoverLowMid2.process (juce::dsp::ProcessContextReplacing<float> (midHighBlock));

        midBuf.makeCopyOf (midHighBuf);
        highBuf.makeCopyOf (midHighBuf);
        juce::dsp::AudioBlock<float> midBlock (midBuf);
        juce::dsp::AudioBlock<float> highBlock (highBuf);
        xoverMidHigh1.process (juce::dsp::ProcessContextReplacing<float> (midBlock));
        xoverMidHigh2.process (juce::dsp::ProcessContextReplacing<float> (highBlock));

        float preLow  = lowBuf.getRMSLevel (0, 0, numSamples);
        float preMid  = midBuf.getRMSLevel (0, 0, numSamples);
        float preHigh = highBuf.getRMSLevel (0, 0, numSamples);

        compLow.process (juce::dsp::ProcessContextReplacing<float> (lowBlock));
        compMid.process (juce::dsp::ProcessContextReplacing<float> (midBlock));
        compHigh.process (juce::dsp::ProcessContextReplacing<float> (highBlock));

        float postLow  = lowBuf.getRMSLevel (0, 0, numSamples);
        float postMid  = midBuf.getRMSLevel (0, 0, numSamples);
        float postHigh = highBuf.getRMSLevel (0, 0, numSamples);

        grLowDb.store (preLow > 0.0001f ? juce::jmin (0.0f, juce::Decibels::gainToDecibels (postLow / preLow)) : 0.0f);
        grMidDb.store (preMid > 0.0001f ? juce::jmin (0.0f, juce::Decibels::gainToDecibels (postMid / preMid)) : 0.0f);
        grHighDb.store (preHigh > 0.0001f ? juce::jmin (0.0f, juce::Decibels::gainToDecibels (postHigh / preHigh)) : 0.0f);

        float makeupLin = juce::Decibels::decibelsToGain (apvts.getRawParameterValue ("MB_MAKEUP")->load());
        for (int ch = 0; ch < 2; ++ch)
        {
            float* dst = buffer.getWritePointer (ch);
            const float* lPtr = lowBuf.getReadPointer (ch);
            const float* mPtr = midBuf.getReadPointer (ch);
            const float* hPtr = highBuf.getReadPointer (ch);
            for (int i = 0; i < numSamples; ++i)
                dst[i] = (lPtr[i] + mPtr[i] + hPtr[i]) * makeupLin;
        }
    }
    else
    {
        grLowDb.store (0.0f); grMidDb.store (0.0f); grHighDb.store (0.0f);
    }

    // STAGE 3: Harmonic Exciter (Tube Asymmetry + Tape Tanh Saturation + DC Blocker)
    if (apvts.getRawParameterValue ("SAT_ENABLE")->load() > 0.5f)
    {
        float drivePct = apvts.getRawParameterValue ("SAT_DRIVE")->load() * 0.01f;
        float tubeBias = apvts.getRawParameterValue ("SAT_EVEN_ODD")->load() * 0.01f * 0.18f;
        float wetMix   = apvts.getRawParameterValue ("SAT_MIX")->load() * 0.01f;
        float driveGain = 1.0f + drivePct * 4.5f;
        float normFactor = 1.0f / std::tanh (driveGain);

        for (int ch = 0; ch < 2; ++ch)
        {
            float* data = buffer.getWritePointer (ch);
            for (int i = 0; i < numSamples; ++i)
            {
                float dry = data[i];
                float shaped = ( std::tanh ((dry + tubeBias) * driveGain) - std::tanh (tubeBias * driveGain) ) * normFactor;
                // DC Blocker
                float y = shaped - dcBlockX1[ch] + 0.995f * dcBlockY1[ch];
                dcBlockX1[ch] = shaped;
                dcBlockY1[ch] = y;
                data[i] = dry * (1.0f - wetMix) + y * wetMix;
            }
        }
    }

    // STAGE 4: Mid/Side Stereo Imager + Mono Sub-Bass Maker
    float sumLR = 0.0f, sumLL = 0.0f, sumRR = 0.0f;
    {
        bool imgOn = apvts.getRawParameterValue ("IMG_ENABLE")->load() > 0.5f;
        float width = apvts.getRawParameterValue ("IMG_WIDTH")->load() * 0.01f;
        float* left  = buffer.getWritePointer (0);
        float* right = buffer.getWritePointer (1);

        for (int i = 0; i < numSamples; ++i)
        {
            if (imgOn)
            {
                float mid  = 0.5f * (left[i] + right[i]);
                float side = 0.5f * (left[i] - right[i]);
                side = sideHighPass.processSample (side) * width;
                left[i]  = mid + side;
                right[i] = mid - side;
            }
            sumLR += left[i] * right[i];
            sumLL += left[i] * left[i];
            sumRR += right[i] * right[i];
        }
    }
    float denom = std::sqrt (sumLL * sumRR);
    stereoCorrelation.store (denom > 1e-6f ? juce::jlimit (-1.0f, 1.0f, sumLR / denom) : 1.0f);

    // STAGE 5: Soft Clipper + True-Peak Brickwall Maximizer
    if (apvts.getRawParameterValue ("LIM_ENABLE")->load() > 0.5f)
    {
        float pushGain = juce::Decibels::decibelsToGain (apvts.getRawParameterValue ("LIM_PUSH")->load());
        float clipAmt  = apvts.getRawParameterValue ("CLIP_DRIVE")->load() * 0.01f;
        buffer.applyGain (pushGain);

        if (clipAmt > 0.01f)
        {
            for (int ch = 0; ch < 2; ++ch)
            {
                float* d = buffer.getWritePointer (ch);
                for (int i = 0; i < numSamples; ++i)
                {
                    float x = d[i];
                    float clipped = (2.0f / juce::MathConstants<float>::pi) * std::atan (x * (1.0f + clipAmt * 2.5f));
                    d[i] = x * (1.0f - clipAmt) + clipped * clipAmt;
                }
            }
        }

        float prePeak = juce::jmax (buffer.getMagnitude (0, 0, numSamples), buffer.getMagnitude (1, 0, numSamples));
        brickwallLimiter.process (context);
        float postPeak = juce::jmax (buffer.getMagnitude (0, 0, numSamples), buffer.getMagnitude (1, 0, numSamples));
        grLimiterDb.store (prePeak > 0.0001f ? juce::jmin (0.0f, juce::Decibels::gainToDecibels (postPeak / prePeak)) : 0.0f);
    }
    else
    {
        grLimiterDb.store (0.0f);
    }

    // Output Metering & LUFS Approximation
    float outL = buffer.getMagnitude (0, 0, numSamples);
    float outR = buffer.getMagnitude (1, 0, numSamples);
    outputPeakL.store (juce::Decibels::gainToDecibels (outL, -60.0f));
    outputPeakR.store (juce::Decibels::gainToDecibels (outR, -60.0f));

    float rmsL = buffer.getRMSLevel (0, 0, numSamples);
    float rmsR = buffer.getRMSLevel (1, 0, numSamples);
    float meanSq = 0.5f * (rmsL * rmsL + rmsR * rmsR);
    float blockLufs = -0.691f + 10.0f * std::log10 (juce::jmax (1e-10f, meanSq));
    float prevLufs = lufsIntegrated.load();
    lufsIntegrated.store (0.85f * prevLufs + 0.15f * juce::jlimit (-60.0f, 0.0f, blockLufs));
}

bool OmniMasterProAudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* OmniMasterProAudioProcessor::createEditor()
{
    return new OmniMasterProAudioProcessorEditor (*this);
}

void OmniMasterProAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void OmniMasterProAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OmniMasterProAudioProcessor();
}
