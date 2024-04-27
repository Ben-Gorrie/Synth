/*
  ==============================================================================

    AllPhasorsVec.h
    Created: 18 Apr 2024 3:48:24pm
    Author:  ben

  ==============================================================================
*/
#include "Oscillators.h"
#pragma once
/**
 *  Holds 4 types of phasors: sine, triangle, square and saw. Also holds information about in what proportion these should be played.
 *  Is used to output a combined normalised sample when process() is called.
 * */
class AllPhasorsVec
{
public:
    /**
     *  Constructor. Populates the phasors vectors with 4 different oscillators.
     * */
    AllPhasorsVec()
    {
        // Empty the vector
        phasors.clear();

        // Populate the vector with each kind of oscillator
        phasors.push_back(new SinOsc());
        phasors.push_back(new TriOsc());
        phasors.push_back(new SquareOsc());
        phasors.push_back(new SawOsc());
    }

    /**
     *  Set sample rates of each phasor and the phase modulation oscillator. This function gets called in the synth's constructor, so assumes that sample rate does not change.
     *  @param sampleRate Sample rate to use
     * */
    void setSampleRate(float sampleRate)
    {
        // Set the sample rate for each phasor
        for (auto& phasor : phasors)
        {
            phasor->setSampleRate(sampleRate);
        }

        // Set the sample rate for the phase modulation oscillator
        sinOscPhaseModulator.setSampleRate(sampleRate);
    }

    /**
     *  Sets the proportions in which each phasor should be played.
     * Example: Setting sinProp = 1, triProp = 0.3, squareProp = 0.2 and sawProp = 0 results in a sound created in the ratio 0.2:0.3:1 by square, triangle and sine waves respectively
     * @param sinProp The proportion of sine oscillators to be included.
     * @param triProp The proportion of triangular oscillators to be included.
     * @param squareProp The proportion of square oscillators to be included.
     * @param sawProp The proportion of saw oscillators to be included.
     * Note that all of the parameters here are exposed to the host, so they can manually be controlled.
     * */
    void setPhasorProportions(juce::RangedAudioParameter* sinProp, juce::RangedAudioParameter* triProp, 
                              juce::RangedAudioParameter* squareProp, juce::RangedAudioParameter* sawProp)
    {
        // Set each element in the phasorProportions vector according to the parameters.
        phasorProportions[0] = sinProp->getValue();
        phasorProportions[1] = triProp->getValue();
        phasorProportions[2] = squareProp->getValue();
        phasorProportions[3] = sawProp->getValue();
    }

    /**
     *  Sets the parameters for phase modulation to take place. When the phase of an oscillator has been incremented by its phaseDelta, 
     *  we compute <float modulatedPhase = phase + modulationIndex * |modulationValue|>. 
     *  modulationValue is the output of a sine oscillator whose frequency is determined by phaseModFreq. 
     *  Note that setting any of these to 0 will result in no phase modulation taking place.  
     *  @param phaseModIndex The phase mdulation index. More extreme values will result in more phase modulation taking place.
     *  @param phaseModFreq Determines the frequency of a sine oscillator whose output is the modulationValue we use.
     * */
    void setPhaseModulationParams(juce::RangedAudioParameter* phaseModIndex, juce::RangedAudioParameter* phaseModFreq)
    {
        // Set the modulation index for each phasor
        for (auto& phasor : phasors)
        {
            phasor->setModulationIndex(phaseModIndex->getValue());
        }

        // Set the frequency of the sine oscillator responsible for generating the modulation value.
        sinOscPhaseModulator.setFrequency(phaseModFreq->getValue());
    }

    /**
     *  Sets the frequencies of the phasors.
     *  @param frequency Frequency which the phasors will use.
     * */
    void setFrequencies(float frequency)
    {
        // Set the frequency for each phasor
        for (auto& phasor : phasors)
        {
            phasor->setFrequency(frequency);
        }
    }

    /**
     *  The main function of the class, this gets called every time a note needs to be played.
     *  First, this sets the phase modulation value for each phasor by calculating the output of a sine oscillator.
     *  Then, this processes each phasor, multiplies the relevant phasor output by the relevant phasor proportion and divides it by the sum of each phasor proportion.
     *  This ensures that the output is always less than 1, but still weighted by the correct phasor proportions. If all proportions are 0, return 0.  
     *  @return The combined normalised output of each oscillator in the right proportions.
     * */
    float process()
    {
        // Initialise the raw sample and the sum of the proportions.
        float rawSample = 0;
        float sumProportion = 0;

        // Calculate the sum of the proportions
        for (int i = 0; i < phasorProportions.size(); i++)
        {
            sumProportion += phasorProportions[i];
        }

        // If each proportion is 0, return 0 to avoid division error.
        if (sumProportion == 0)
        {
            return 0;
        }

        // Compute the phase modulation value 
        float modulationValue = sinOscPhaseModulator.process();

        for (int i = 0; i < phasors.size(); i++)
        {
            // Set the phase modulation value for each phasor
            phasors[i]->setModulationValue(modulationValue);

            // Increment the sample by the scaled output of each phasor, ensuring normalisation takes place
            rawSample += phasors[i]->process() * (phasorProportions[i] / sumProportion);
        }

        return rawSample;
    }
    
private:
    // Vector to hold each type of phasor
    std::vector<Phasor*> phasors;

    // Vector to hold the values for each phasor proportion we should use
    std::vector<float> phasorProportions = {1.0f, 0.0f, 0.0f, 0.0f};

    // Sine oscillator to modulate the phase of each oscillator
    SinOsc sinOscPhaseModulator;

};
