/*
  ==============================================================================

    YourSynthesiser.h
    Created: 7 Mar 2020 4:27:57pm
    Author:  Tom Mudd

  ==============================================================================
*/

#pragma once
#include "Oscillators.h"
#include "ToneMatrix.h"
#include "AllPhasorsVec.h"

// ===========================
// ===========================
// SOUND
class LifeSynthSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote      (int) override      { return true; }
    //--------------------------------------------------------------------------
    bool appliesToChannel   (int) override      { return true; }
};




// =================================
// =================================
// Synthesiser Voice - your synth code goes in here

/*!
 @class LifeSynthVoice
 @abstract struct defining the DSP associated with a specific voice.
 @discussion multiple LifeSynthVoice objects will be created by the Synthesiser so that it can be played polyphicially
 
 @namespace none
 @updated 2019-06-18
 */
class LifeSynthVoice : public juce::SynthesiserVoice
{
public:
    LifeSynthVoice() {}
    //--------------------------------------------------------------------------

    
    void setParametersFromAPVTS(juce::AudioProcessorValueTreeState& apvts)
    {
        lifeNoteParam = apvts.getRawParameterValue("lifeNote");

        attackParam = apvts.getRawParameterValue("attack");    
        decayParam = apvts.getRawParameterValue("decay");    
        sustainParam = apvts.getRawParameterValue("sustain");    
        releaseParam = apvts.getRawParameterValue("release");    

        sinPropParam = apvts.getRawParameterValue("sinProp");
        triPropParam = apvts.getRawParameterValue("triProp");
        squarePropParam = apvts.getRawParameterValue("squareProp");
        sawPropParam = apvts.getRawParameterValue("sawProp");
    }


    /**
     What should be done when a note starts

     @param midiNoteNumber
     @param velocity
     @param SynthesiserSound unused variable
     @param / unused variable
     */
    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound*, int /*currentPitchWheelPosition*/) override
    {

        playing = true;
        float freq = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);

        phasors.initPhasors(getSampleRate(), sinPropParam, triPropParam, squarePropParam, sawPropParam);

        phasors.setFrequencies(freq);

        env.setSampleRate(getSampleRate());

        juce::ADSR::Parameters envParams;
        envParams.attack = *attackParam;
        envParams.decay = *decayParam;
        envParams.sustain = *sustainParam;
        envParams.release = *releaseParam;

        env.setParameters(envParams);

        env.noteOn();

        
    }
    //--------------------------------------------------------------------------
    /// Called when a MIDI noteOff message is received
    /**
     What should be done when a note stops

     @param / unused variable
     @param allowTailOff bool to decie if the should be any volume decay
     */
    void stopNote(float /*velocity*/, bool allowTailOff) override
    {
        env.noteOff();
    }
    
    //--------------------------------------------------------------------------
    /**
     The Main DSP Block: Put your DSP code in here
     
     If the sound that the voice is playing finishes during the course of this rendered block, it must call clearCurrentNote(), to tell the synthesiser that it has finished

     @param outputBuffer pointer to output
     @param startSample position of first sample in buffer
     @param numSamples number of smaples in output buffer
     */
    void renderNextBlock(juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override
    {
        if (playing) // check to see if this voice should be playing
        {
            // iterate through the necessary number of samples (from startSample up to startSample + numSamples)
            for (int sampleIndex = startSample; sampleIndex < (startSample + numSamples); sampleIndex++)
            {
                // your sample-by-sample DSP code here!

                float toneMatrixSample = 0;
                if (lifeNoteParam->load() == 1)
                {
                    toneMatrixSample = toneMatrix.process();
                }
                
                float synthMixSample = phasors.process(); 
                float envValue = env.getNextSample();
                
                // for each channel, write the currentSample float to the output
                for (int chan = 0; chan<outputBuffer.getNumChannels(); chan++)
                {
                    // The output sample is scaled by 0.2 so that it is not too loud by default
                    outputBuffer.addSample(chan, sampleIndex, (synthMixSample + toneMatrixSample) * envValue);
                }

                if (!env.isActive())
                {
                    toneMatrix.incrementColumnAndWrap();
                    playing = false;
                    clearCurrentNote();
                }
            }
        }
    }

    void setInitialState(const std::vector<std::pair<int, int>>& liveCells, int sampleRate)
    {
        toneMatrix.setInitialState(liveCells, sampleRate);
    }

    //--------------------------------------------------------------------------
    void pitchWheelMoved(int) override {}
    //--------------------------------------------------------------------------
    void controllerMoved(int, int) override {}
    //--------------------------------------------------------------------------
    /**
     Can this voice play a sound. I wouldn't worry about this for the time being

     @param sound a juce::SynthesiserSound* base class pointer
     @return sound cast as a pointer to an instance of LifeSynthSound
     */
    bool canPlaySound (juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<LifeSynthSound*> (sound) != nullptr;
    }
    //--------------------------------------------------------------------------
private:
    //--------------------------------------------------------------------------
    // Set up any necessary variables here
    /// Should the voice be playing?
    bool playing = false;

    AllPhasorsVec phasors;

    juce::ADSR env;

    std::atomic<float>* lifeNoteParam;

    std::atomic<float>* attackParam;
    std::atomic<float>* decayParam;
    std::atomic<float>* sustainParam;
    std::atomic<float>* releaseParam;


    std::atomic<float>* sinPropParam;
    std::atomic<float>* triPropParam;
    std::atomic<float>* squarePropParam;
    std::atomic<float>* sawPropParam;



    ToneMatrix toneMatrix;

};
