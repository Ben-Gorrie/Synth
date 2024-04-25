/*
  ==============================================================================

    LifeSynth.h
    Created: 7 Mar 2020 4:27:57pm
    Author: Ben Gorrie
    Inspired by: Tom Mudd

  ==============================================================================
*/

#pragma once
#include "Oscillators.h"
#include "ToneMatrix.h"
#include "AllPhasorsVec.h"
#include "ChangingFilter.h"

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
// Synthesiser Voice

/*!
 @class LifeSynthVoice
 @abstract struct defining the DSP associated with a specific voice.
 @discussion multiple LifeSynthVoice objects will be created by the Synthesiser so that it can be played polyphicially
 
 @namespace none
 */
class LifeSynthVoice : public juce::SynthesiserVoice
{
public:
    LifeSynthVoice() {}
    //--------------------------------------------------------------------------

    
    void setParametersFromAPVTS(juce::AudioProcessorValueTreeState& apvts)
    {
        lifeNoteParam = apvts.getRawParameterValue("lifeNote");
        lifeResetParam = apvts.getRawParameterValue("lifeResetChoice");
        lifeInitStateParam = apvts.getRawParameterValue("lifeInitState");
        lifeRandomNumberParam = apvts.getRawParameterValue("lifeRandomNumber");

        attackParam = apvts.getRawParameterValue("attack");    
        decayParam = apvts.getRawParameterValue("decay");    
        sustainParam = apvts.getRawParameterValue("sustain");    
        releaseParam = apvts.getRawParameterValue("release");    

        pitchAttackParam = apvts.getRawParameterValue("attackPitch");
        pitchDecayParam = apvts.getRawParameterValue("decayPitch");    
        pitchSustainParam = apvts.getRawParameterValue("sustainPitch");    
        pitchReleaseParam = apvts.getRawParameterValue("releasePitch");    

        pitchBendRangeParam = apvts.getRawParameterValue("pitchRange");

        sinPropParam = apvts.getRawParameterValue("sinProp");
        triPropParam = apvts.getRawParameterValue("triProp");
        squarePropParam = apvts.getRawParameterValue("squareProp");
        sawPropParam = apvts.getRawParameterValue("sawProp");

        phaseModIndexParam = apvts.getRawParameterValue("phaseModulationIndex");
        phaseModFreqParam = apvts.getRawParameterValue("phaseModulationFreq");

        filterChoiceParam = apvts.getRawParameterValue("filterChoice");
        filterLFOFreqsParam = apvts.getRawParameterValue("filterLFOFreqs");
        filterLFOFreqsOffsetParam = apvts.getRawParameterValue("filterLFOFreqsOffset");
        filterBaseCutoffParam = apvts.getRawParameterValue("filterBaseCutoff");
        filterModulationDepthParam = apvts.getRawParameterValue("filterModulationDepth");

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

        currentMidiNoteNumber = midiNoteNumber;
        playing = true;
        float freq = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);

        phasors.initPhasors(getSampleRate(), sinPropParam, triPropParam, squarePropParam, sawPropParam, phaseModIndexParam, phaseModFreqParam);

        //phasors.setFrequencies(freq);

        env.setSampleRate(getSampleRate());
        pitchEnv.setSampleRate(getSampleRate());


        juce::ADSR::Parameters envParams;
        envParams.attack = attackParam->load();
        envParams.decay = decayParam->load();
        envParams.sustain = sustainParam->load();
        envParams.release = releaseParam->load();

        env.setParameters(envParams);
        
        juce::ADSR::Parameters pitchEnvParams;
        pitchEnvParams.attack = pitchAttackParam->load();
        pitchEnvParams.decay = pitchDecayParam->load();
        pitchEnvParams.sustain = pitchSustainParam->load();
        pitchEnvParams.release = pitchReleaseParam->load();

        pitchEnv.setParameters(pitchEnvParams);

        if (lifeResetParam->load() == 1)
        {
            toneMatrix.setInitialState(lifeInitStateParam, lifeRandomNumberParam, getSampleRate());
        }

        // Initialise the filter which changes cutoff based on LFOs 
        changingFilter.createLFOs(getSampleRate());
        changingFilter.setLFOFrequencies(filterLFOFreqsParam->load(), filterLFOFreqsOffsetParam->load());
        changingFilter.initFilter();

        env.noteOn();
        pitchEnv.noteOn();
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
        pitchEnv.noteOff();
    }
    
    //--------------------------------------------------------------------------
    /**
     The Main DSP Block
     
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

                float toneMatrixSample = 0;
                int toneMatrixVolumeBalancer = 1;
                if (lifeNoteParam->load())
                {
                    toneMatrixSample = toneMatrix.process();
                    toneMatrixVolumeBalancer++;
                }

                float floatMidiNote = currentMidiNoteNumber + pitchBendRangeParam->load() * pitchEnv.getNextSample();

                float frequency = 440 * pow(2, (floatMidiNote - 69) / 12);

                phasors.setFrequencies(frequency);

                float synthMixSample = phasors.process(); 
                float envValue = env.getNextSample();

                float combinedSample = (synthMixSample + toneMatrixSample ) / toneMatrixVolumeBalancer;

                if (filterChoiceParam->load() == 1)
                {
                    changingFilter.setCutoff(filterBaseCutoffParam->load(), filterModulationDepthParam->load());
                    changingFilter.setFilterCoefs();
                    combinedSample = changingFilter.process(combinedSample);
                }
                
                // for each channel, write the currentSample float to the output
                for (int chan = 0; chan < outputBuffer.getNumChannels(); chan++)
                {
                    outputBuffer.addSample(chan, sampleIndex, combinedSample * envValue);
                }

                if (!env.isActive())
                {
                    playing = false;
                    clearCurrentNote();
                }
            }

            if (playing == false)
            {
                toneMatrix.incrementColumnAndWrap();
            }
        }
    }

    void setInitialState(std::atomic<float>* choiceParam, std::atomic<float>* randomNumberOfCellsParam, float sampleRate)
    {
        toneMatrix.setInitialState(choiceParam, randomNumberOfCellsParam, sampleRate);
    }

    //--------------------------------------------------------------------------
    void pitchWheelMoved(int) override {}
    //--------------------------------------------------------------------------
    void controllerMoved(int, int) override {}
    //--------------------------------------------------------------------------
    /**
     Can this voice play a sound. Currently unused. 

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

    juce::ADSR pitchEnv;

    int currentMidiNoteNumber;

    std::atomic<float>* lifeNoteParam;
    std::atomic<float>* lifeResetParam;
    std::atomic<float>* lifeInitStateParam;
    std::atomic<float>* lifeRandomNumberParam;

    std::atomic<float>* attackParam;
    std::atomic<float>* decayParam;
    std::atomic<float>* sustainParam;
    std::atomic<float>* releaseParam;

    std::atomic<float>* pitchAttackParam;
    std::atomic<float>* pitchDecayParam;
    std::atomic<float>* pitchSustainParam;
    std::atomic<float>* pitchReleaseParam;

    std::atomic<float>* pitchBendRangeParam;

    std::atomic<float>* sinPropParam;
    std::atomic<float>* triPropParam;
    std::atomic<float>* squarePropParam;
    std::atomic<float>* sawPropParam;

    std::atomic<float>* phaseModIndexParam;
    std::atomic<float>* phaseModFreqParam;

    ToneMatrix toneMatrix;
    ChangingFilter changingFilter;
    std::atomic<float>* filterChoiceParam;

    std::atomic<float>* filterLFOFreqsParam;
    std::atomic<float>* filterLFOFreqsOffsetParam;
    std::atomic<float>* filterBaseCutoffParam;
    std::atomic<float>* filterModulationDepthParam;



};
