/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Synthesiser Starting code (sound and voice).h"

//==============================================================================
/**
*/
class SynthAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    SynthAudioProcessor();
    ~SynthAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SynthAudioProcessor)
    juce::Synthesiser synth;
    int voiceCount = 16;

    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
    {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("masterVol", 1), "Master Volume", 0, 2.0, 0.2));

        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("attack", 1), "Attack", 0.001, 4.0, 0.01));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("decay", 1), "Decay", 0.001, 4.0, 0.25));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("sustain", 1), "Sustain", 0.001, 1.0, 0.5));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("release", 1), "Release", 0.001, 8.0, 1));

        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("sinProp", 1), "Sin wave Proportion", 0, 1.0, 1.0));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("triProp", 1), "Triangle wave Proportion", 0, 1.0, 0));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("squareProp", 1), "Square wave Proportion", 0, 1.0, 0));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("sawProp", 1), "Sawtooth wave Proportion", 0, 1.0, 0));

        return layout;
    }

    juce::Random random;
};
