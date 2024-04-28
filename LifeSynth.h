/*
  ==============================================================================

    LifeSynth.h
    Created: 15 Apr 2024 4:27:57pm
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

/**
 @class LifeSynthVoice
 @abstract struct defining the DSP associated with a specific voice.
 @discussion multiple LifeSynthVoice objects will be created by the Synthesiser so that it can be played polyphicially
 
 @namespace none
 */
class LifeSynthVoice : public juce::SynthesiserVoice
{
public:
    /**
     *  Constructor for the synth. Set the sample rate of the oscillators playing notes caused by the game of life. Also set the sample rate for the low-pass filter
     * */
    LifeSynthVoice() 
    {
        phasors.setSampleRate(getSampleRate());
        changingFilter.createLFOs(getSampleRate());
    }
    
    /**
     *  Dynamically sets the user controllable parameters from the audio processor value tree state.
     *  Note that some parameters here are stored using getRawParameterValue() and some are using getParameter().
     *  Those using the latter can be changed by the game of life itself if the option is toggled, leading to a "zero player synth" of sorts.
     * */
    void setParametersFromAPVTS(juce::AudioProcessorValueTreeState& apvts)
    {
        // This parameter determines whether or not the game of life can control the synth
        lifeControlParam = apvts.getRawParameterValue("lifeControl");

        // Determines whether the tone matrix should play a note if the game of life has live cells in the right column
        lifeNoteParam = apvts.getRawParameterValue("lifeNote");

        // Determines whether the state of the game of life should be reset to some initial configuration. 
        // Please note that this parameter is intended to be toggled on and then immediately off 
        // (unless the user wants the state of the game of life to stay the same, except if the "random" option is chosen), 
        // and this should be done every time when changing between presets to ensure that the state of the board matches the chosen option.
        lifeResetParam = apvts.getRawParameterValue("lifeResetChoice");

        // Determines what state the game of life starts in. This is used if the above parameter is set to true.
        lifeInitStateParam = apvts.getRawParameterValue("lifeInitState");

        // Determines the initial number of live cells on the game of life is "random" is chosen for the above parameter.
        lifeRandomNumberParam = apvts.getRawParameterValue("lifeRandomNumber");

        // Volume ADSR envelope parameters
        attackParam = apvts.getParameter("attack");
        decayParam = apvts.getParameter("decay");    
        sustainParam = apvts.getParameter("sustain");    
        releaseParam = apvts.getParameter("release");    

        // Pitch ADSR envelope parameters
        pitchAttackParam = apvts.getParameter("attackPitch");
        pitchDecayParam = apvts.getParameter("decayPitch");    
        pitchSustainParam = apvts.getParameter("sustainPitch");    
        pitchReleaseParam = apvts.getParameter("releasePitch");    

        // Pitch beding range parameter
        pitchBendRangeParam = apvts.getParameter("pitchRange");

        // Parameters that determine in what proportion the 4 oscillator types should be played when a note is pressed.
        sinPropParam = apvts.getParameter("sinProp");
        triPropParam = apvts.getParameter("triProp");
        squarePropParam = apvts.getParameter("squareProp");
        sawPropParam = apvts.getParameter("sawProp");

        // Parameter for phase modulation index
        phaseModIndexParam = apvts.getParameter("phaseModulationIndex");

        // Parameter which determines the frequency of a sine oscillator, whose output in turn dictates phase modulation value.
        phaseModFreqParam = apvts.getParameter("phaseModulationFreq");

        // This parameter determines whether a low-pass filter should be applied
        filterChoiceParam = apvts.getRawParameterValue("filterChoice");
        
        // These parameters determine aspects for the low-pass filter
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
        // Create objects to store the parameters for the amplitude and pitch ADSR envelopes.
        juce::ADSR::Parameters envParams;
        juce::ADSR::Parameters pitchEnvParams;

        // Set the sample rate for the amplitude and pitch envelopes
        env.setSampleRate(getSampleRate());
        pitchEnv.setSampleRate(getSampleRate());

        // If the game of life is allowed to control parameters, update the following parameters according to the relevant column in the game of life grid.
        if (lifeControlParam->load())
        {
            toneMatrix.changeSliderParamAccordingToColumn(attackParam);
            toneMatrix.changeSliderParamAccordingToColumn(decayParam);
            toneMatrix.changeSliderParamAccordingToColumn(sustainParam);
            toneMatrix.changeSliderParamAccordingToColumn(releaseParam);

            toneMatrix.changeSliderParamAccordingToColumn(pitchAttackParam);
            toneMatrix.changeSliderParamAccordingToColumn(pitchDecayParam);
            toneMatrix.changeSliderParamAccordingToColumn(pitchSustainParam);
            toneMatrix.changeSliderParamAccordingToColumn(pitchReleaseParam);

            toneMatrix.changeSliderParamAccordingToColumn(sinPropParam);
            toneMatrix.changeSliderParamAccordingToColumn(triPropParam);
            toneMatrix.changeSliderParamAccordingToColumn(squarePropParam);
            toneMatrix.changeSliderParamAccordingToColumn(sawPropParam);

            toneMatrix.changeSliderParamAccordingToColumn(phaseModIndexParam);
            toneMatrix.changeSliderParamAccordingToColumn(phaseModFreqParam);

            toneMatrix.changeSliderParamAccordingToColumn(pitchBendRangeParam);
        }

        // Set the parameters of the amplitude and pitch ADSR envelopes according to the relevant controllable parameters.
        envParams.attack = attackParam->getValue();
        envParams.decay = decayParam->getValue();
        envParams.sustain = sustainParam->getValue();
        envParams.release = releaseParam->getValue();

        pitchEnvParams.attack = pitchAttackParam->getValue();
        pitchEnvParams.decay = pitchDecayParam->getValue();
        pitchEnvParams.sustain = pitchSustainParam->getValue();
        pitchEnvParams.release = pitchReleaseParam->getValue();

        // Save the parameters to the relevant objects
        env.setParameters(envParams);
        pitchEnv.setParameters(pitchEnvParams);

        // Save the current note being played
        currentMidiNoteNumber = midiNoteNumber;

        // Set the phasor proportions to use when a note is hit and the phase modulation parameters
        phasors.setPhasorProportions(sinPropParam, triPropParam, squarePropParam, sawPropParam);
        phasors.setPhaseModulationParams(phaseModIndexParam, phaseModFreqParam);

        // If the filter is turned on, allow the frequencies of the lfos controlling the low-pass filter cutoff to change
        if (filterChoiceParam->load())
        {
            changingFilter.setLFOFrequencies(filterLFOFreqsParam->load(), filterLFOFreqsOffsetParam->load());
        }

        // Set playing to true
        playing = true;

        // Turn on the envelopes for amplitude and pitch
        env.noteOn();
        pitchEnv.noteOn();
    }

    //--------------------------------------------------------------------------
    /// Called when a MIDI noteOff message is received
    /**
     What should be done when a note stops

     @param / unused variable
     @param allowTailOff bool to decie if the should be any volume decay. Currently unused.
     */
    void stopNote(float /*velocity*/, bool allowTailOff) override
    {
        // Turn off both envelopes (start the release phase)
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

                float floatMidiNote = currentMidiNoteNumber + pitchBendRangeParam->getValue() * pitchEnv.getNextSample();

                float frequency = 440 * pow(2, (floatMidiNote - 69) / 12);

                phasors.setFrequencies(frequency);

                float synthMixSample = phasors.process(); 
                float envValue = env.getNextSample();

                float combinedSample = (synthMixSample + toneMatrixSample ) / toneMatrixVolumeBalancer;

                if (filterChoiceParam->load())
                {
                    changingFilter.setCutoff(filterBaseCutoffParam->load(), filterModulationDepthParam->load());
                    changingFilter.setFilterCoefs();
                    combinedSample = changingFilter.process(combinedSample);
                }
                
                // for each channel, write the currentSample float to the output
                for (int chan = 0; chan < outputBuffer.getNumChannels(); chan++)
                {
                    outputBuffer.addSample(chan, sampleIndex, combinedSample * envValue * 0.5f);
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

        if (lifeResetParam->load())
        {
            toneMatrix.setInitialState(lifeInitStateParam, lifeRandomNumberParam, getSampleRate());
        }
    }

    void setToneMatrixInitialState(std::atomic<float>* choiceParam, std::atomic<float>* randomNumberOfCellsParam, float sampleRate)
    {
        toneMatrix.setInitialState(choiceParam, randomNumberOfCellsParam, sampleRate);
    }

    ToneMatrix getToneMatrix()
    {
        return toneMatrix;
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

    std::atomic<float>* lifeControlParam;

    std::atomic<float>* lifeNoteParam;
    std::atomic<float>* lifeResetParam;
    std::atomic<float>* lifeInitStateParam;
    std::atomic<float>* lifeRandomNumberParam;

    juce::RangedAudioParameter* attackParam;
    juce::RangedAudioParameter* decayParam;
    juce::RangedAudioParameter* sustainParam;
    juce::RangedAudioParameter* releaseParam;

    juce::RangedAudioParameter* pitchAttackParam;
    juce::RangedAudioParameter* pitchDecayParam;
    juce::RangedAudioParameter* pitchSustainParam;
    juce::RangedAudioParameter* pitchReleaseParam;

    juce::RangedAudioParameter* pitchBendRangeParam;

    juce::RangedAudioParameter* sinPropParam;
    juce::RangedAudioParameter* triPropParam;
    juce::RangedAudioParameter* squarePropParam;
    juce::RangedAudioParameter* sawPropParam;

    juce::RangedAudioParameter* phaseModIndexParam;
    juce::RangedAudioParameter* phaseModFreqParam;

    ToneMatrix toneMatrix;
    ChangingFilter changingFilter;
    std::atomic<float>* filterChoiceParam;

    std::atomic<float>* filterLFOFreqsParam;
    std::atomic<float>* filterLFOFreqsOffsetParam;
    std::atomic<float>* filterBaseCutoffParam;
    std::atomic<float>* filterModulationDepthParam;


};
