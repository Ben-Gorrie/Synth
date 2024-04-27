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
 *  Holds 4 types of phasors: sine, triangle, square and saw. Also holds information about in what proportion these should be played, and can then output a combined sample
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

    void setSampleRate(float sampleRate)
    {
        for (auto& phasor : phasors)
        {
            phasor->setSampleRate(sampleRate);
        }

        sinOscPhaseModulator.setSampleRate(sampleRate);
    }

    void initPhasors(
                     juce::RangedAudioParameter* sinProp, juce::RangedAudioParameter* triProp, juce::RangedAudioParameter* squareProp, juce::RangedAudioParameter* sawProp,
                     juce::RangedAudioParameter* phaseModIndex, juce::RangedAudioParameter* phaseModFreq)
    {

        for (auto& phasor : phasors)
        {
            //phasor->setSampleRate(sampleRate);
            phasor->setModulationIndex(phaseModIndex->getValue());
        }

        phasorProportions[0] = sinProp->getValue();
        phasorProportions[1] = triProp->getValue();
        phasorProportions[2] = squareProp->getValue();
        phasorProportions[3] = sawProp->getValue();

        //sinOscPhaseModulator.setSampleRate(sampleRate);
        sinOscPhaseModulator.setFrequency(phaseModFreq->getValue());
    }

    void setFrequencies(float frequency)
    {
        for (auto& phasor : phasors)
        {
            phasor->setFrequency(frequency);
        }
    }

    float process()
    {
        float rawWave = 0;
        float sumProportion = 0;
        for (int i = 0; i < phasorProportions.size(); i++)
        {
            sumProportion += phasorProportions[i];
        }

        if (sumProportion == 0)
        {
            return 0;
        }

        for (int i = 0; i < phasors.size(); i++)
        {
            phasors[i]->setModulationValue(sinOscPhaseModulator.process());
            rawWave += phasors[i]->process() * (phasorProportions[i] / sumProportion);
        }
        return rawWave;
    }
    
private:
    std::vector<Phasor*> phasors;

    std::vector<float> phasorProportions = {1.0f, 0.0f, 0.0f, 0.0f};

    SinOsc sinOscPhaseModulator;

};
