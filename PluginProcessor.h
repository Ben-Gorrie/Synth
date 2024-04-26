/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "LifeSynth.h"
#include "Panning.h"

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
    int voiceCount = 8;

    // Audio Processor Value Tree State which stores user-controllable parameters
    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
    {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;
        
        // Allows the game of life to control the synth on its own
        layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID("lifeControl", 1), "Game of Life synth control toggle", false));

        // ADSR parameters for amplitude envelope
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("attack", 1), "Volume Attack", 0.01, 4.0, 0.1));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("decay", 1), "Volume Decay", 0.01, 4.0, 0.25));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("sustain", 1), "Volume Sustain", 0.01, 1.0, 0.5));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("release", 1), "Volume Release", 0.01, 8.0, 1));

        // ADSR parameters for pitch envelope
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("attackPitch", 1), "Pitch Attack", 0.01, 4.0, 0.1));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("decayPitch", 1), "Pitch Decay", 0.01, 4.0, 0.25));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("sustainPitch", 1), "Pitch Sustain", 0.01, 1.0, 0.5));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("releasePitch", 1), "Pitch Release", 0.01, 8.0, 1));

        // Pitch bending range parameter
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("pitchRange", 1), "Pitch Bending Range", 0, 50.0f, 5.0f));

        // Parameters which control the proportion of wave types that are played when a key is pressed
        // Example: Setting sinProp = 1, triProp = 0.3, squareProp = 0.2 and sawProp = 0 results in a sound created in the ratio 0.2:0.3:1 by square, triangle and sine waves respectively
        // Note that maxing all of these out will result in a sound which is 4 times too loud. This is as intended, so users should attempt to sum these parameters to 1.
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("sinProp", 1), "Sine wave Proportion", 0, 1.0, 1.0));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("triProp", 1), "Triangle wave Proportion", 0, 1.0, 0));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("squareProp", 1), "Square wave Proportion", 0, 1.0, 0));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("sawProp", 1), "Sawtooth wave Proportion", 0, 1.0, 0));

        // Controls whether the Tone Matrix, which changes according to the rules of the game of life, plays a note when a cell is live in a particular column
        layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID("lifeNote", 1), "Game of Life note toggle", true));
        // Reset game of life toggle
        layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID("lifeResetChoice", 1), "Game of Life reset toggle", false));
        // Presets for the game of life
        layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("lifeInitState", 1), "Game of Life initial state", 
                                                                juce::StringArray({"Cube", "Pentadecathlon", "Tumbler", "Figure eight", "Octagon 2", "Random"}), 2));
        // Number of cells to populate if the initial choice is Random
        layout.add(std::make_unique<juce::AudioParameterInt>(juce::ParameterID("lifeRandomNumber", 1), "Game of Life initial number of random cells", 0, 128, 40));

        // Toggles reverb on or off
        layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID("reverbChoice", 1), "Reverb toggle", false));

        // Parameters for reverb.
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("reverbDry", 1), "Reverb Dry Level", 0, 1, 0));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("reverbWet", 1), "Reverb Wet Level", 0, 1, 0));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("reverbRoomSize", 1), "Reverb Room Size", 0, 1, 0));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("reverbWidth", 1), "Reverb Width", 0, 1, 0));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("reverbDamping", 1), "Reverb Damping", 0, 1, 0));

        // Parameters for chorus
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("rate", 1), "Chorus Rate", 0.1f, 10.0f, 1.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("depth", 1), "Chorus Depth", 0.1f, 1.0f, 0.5f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("mix", 1), "Chorus Mix", 0.0f, 1.0f, 0.5f));

        // Parameters for panning
        // Toggles panning on or off
        layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID("panningChoice", 1), "Panning toggle", false));
        // Sets panning rate
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("panningRate", 1), "Panning Rate", 0.01f, 5.0f, 0.1f));


        // Sets modulation index
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("phaseModulationIndex", 1), "Phase Modulation Index", 0.0f, 10.0f, 1.0f));
        // Frequency that controls frequency of modulator oscillator
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("phaseModulationFreq", 1), "Phase Modulation Frequency", 0.0f, 20.0f, 1.0f));

        // Filter parameters 
        layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID("filterChoice", 1), "Low-Pass filter toggle", false));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("filterLFOFreqs", 1), "Frequency determining changing low-pass filter cutoff", 0.0f, 10.0f, 1.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("filterLFOFreqsOffset", 1), "Frequency offset for LFOs", 0.0f, 10.0f, 1.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("filterBaseCutoff", 1), "Base cutoff for low-pass filter", 0.0f, 7000.0f, 700.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("filterModulationDepth", 1), "Modulation depth for cutoff", 0.0f, 300.0f, 100.0f));

        return layout;
    }

    // Reverb
    juce::Reverb reverb;

    // Chorus
    juce::dsp::Chorus<float> chorus;

    // Panning
    Panning panning;

};
