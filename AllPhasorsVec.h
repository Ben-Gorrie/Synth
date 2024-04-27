/*
  ==============================================================================

    AllPhasorsVec.h
    Created: 18 Apr 2024 3:48:24pm
    Author:  ben

  ==============================================================================
*/
#include "Oscillators.h"
#pragma once
class AllPhasorsVec
{
public:
    AllPhasorsVec()
    {
        phasors.clear();

        phasors.push_back(new SinOsc());
        phasors.push_back(new TriOsc());
        phasors.push_back(new SquareOsc());
        phasors.push_back(new SawOsc());
    }
    void initPhasors(float sampleRate,
                     juce::RangedAudioParameter* sinProp, juce::RangedAudioParameter* triProp, juce::RangedAudioParameter* squareProp, juce::RangedAudioParameter* sawProp,
                     juce::RangedAudioParameter* phaseModIndex, juce::RangedAudioParameter* phaseModFreq)
    {
        //phasors.clear();
        phasorProportions.clear();

        //phasors.push_back(new SinOsc());
        //phasors.push_back(new TriOsc());
        //phasors.push_back(new SquareOsc());
        //phasors.push_back(new SawOsc());
        for (auto& phasor : phasors)
        {
            phasor->setSampleRate(sampleRate);
            phasor->setModulationIndex(phaseModIndex->getValue());
        }

        phasorProportions.push_back(sinProp->getValue());
        phasorProportions.push_back(triProp->getValue());
        phasorProportions.push_back(squareProp->getValue());
        phasorProportions.push_back(sawProp->getValue());

        sinOscPhaseModulator.setSampleRate(sampleRate);
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

    std::vector<float> phasorProportions;

    SinOsc sinOscPhaseModulator;

};
