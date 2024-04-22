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
    void initPhasors(float sampleRate, std::atomic<float>* sinProp, std::atomic<float>* triProp, std::atomic<float>* squareProp, std::atomic<float>* sawProp)
    {
        phasors.clear();
        phasorProportions.clear();

        phasors.push_back(new SinOsc());
        phasors.push_back(new TriOsc());
        phasors.push_back(new SquareOsc());
        phasors.push_back(new Phasor());
        for (auto& phasor : phasors)
        {
            phasor->setSampleRate(sampleRate);
        }

        phasorProportions.push_back(sinProp->load());
        phasorProportions.push_back(triProp->load());
        phasorProportions.push_back(squareProp->load());
        phasorProportions.push_back(sawProp->load());
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
            rawWave += phasors[i]->process() * (phasorProportions[i] / sumProportion);
        }
        return rawWave;
    }
    
private:
    std::vector<Phasor*> phasors;

    std::vector<float> phasorProportions;

};
