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
    // synth object with max number of voices
    juce::Synthesiser synth;
    int voiceCount = 16;

    // Audio Processor Value Tree State which stores user-controllable parameters
    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
    {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;
        // ADSR parameters for envelope
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("attack", 1), "Attack", 0.001, 4.0, 0.01));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("decay", 1), "Decay", 0.001, 4.0, 0.25));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("sustain", 1), "Sustain", 0.001, 1.0, 0.5));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("release", 1), "Release", 0.001, 8.0, 1));

        // Parameters which control the proportion of wave types that are played when a key is pressed
        // Example: Setting sinProp = 1, triProp = 0.3, squareProp = 0.2 and sawProp = 0 results in a sound created in the ratio 0.2:0.3:1 by square, triangle and sine waves respectively
        // Note that maxing all of these out will result in a sound which is 4 times too loud. This is as intended, so users should attempt to sum these parameters to 1.
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("sinProp", 1), "Sine wave Proportion", 0, 1.0, 1.0));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("triProp", 1), "Triangle wave Proportion", 0, 1.0, 0));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("squareProp", 1), "Square wave Proportion", 0, 1.0, 0));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("sawProp", 1), "Sawtooth wave Proportion", 0, 1.0, 0));

        // Controls whether the Tone Matrix, which changes according to the rules of the game of life, plays a note when a cell is live in a particular column
        layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("lifeNote", 1), "Game of Life note toggle", juce::StringArray({"Off", "On"}), 1));

        // Toggles reverb on or off
        layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("reverbChoice", 1), "Reverb toggle", juce::StringArray({"Off", "On"}), 0));

        // Parameters for reverb.
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("reverbDry", 1), "Reverb Dry Level", 0, 1, 0));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("reverbWet", 1), "Reverb Wet Level", 0, 1, 0));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("reverbRoomSize", 1), "Reverb Room Size", 0, 1, 0));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("reverbWidth", 1), "Reverb Width", 0, 1, 0));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("reverbDamping", 1), "Reverb Damping", 0, 1, 0));

        // Parameters for chorus
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("rate", 1), "Rate", 0.1f, 10.0f, 1.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("depth", 1), "Depth", 0.1f, 1.0f, 0.5f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("mix", 1),"Mix", 0.0f, 1.0f, 0.5f));

        return layout;
    }

    // Reverb
    juce::Reverb reverb;

    // Chorus
    juce::dsp::Chorus<float> chorus;
};
