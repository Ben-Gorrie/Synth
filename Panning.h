/*
  ==============================================================================

    Panning.h
    Created: 20 Apr 2024 6:09:50pm
    Author:  ben

  ==============================================================================
*/
#include "Oscillators.h"
#include<vector>
#pragma once


/**
 *  Controls basic panning. Takes an input sample and return two samples. This will slowly pan in a circle
 */
class Panning
{
public:
    /**
     *  Set the sample rate of the lfo
     *  @param sampleRate Sample rate to set lfo to
     */
    void setSampleRate(float sampleRate)
    {
       lfo.setSampleRate(sampleRate); 
    }

    /**
     *  Set the frequency of the lfo
     *  @param frequency Frequency to set the lfo to
     */
    void setFrequency(float frequency)
    {
        lfo.setFrequency(frequency);
    }

    /**
     *  Processes a single sample into two panning samples. The panning is controlled by an LFO
     *  @param sample Input sample to turn into two
     *  @return Vector holding two panning samples
     */
    std::vector<float> process(float sample)
    {
        // Store a float between 0 and 1 which changes with an LFO
        float lfo_val = (lfo.process() + 1) / 2;

        // If else block to prevent one ear from being completly off
        if (lfo_val > 0.1)
        {
            // Compute the left and right sample based on the value of the LFO
            float sampleL = sample * lfo_val;
            float sampleR =  sample * (1 - lfo_val); 
            return {sampleL, sampleR};
        }
        else 
        { 
            // Compute the left and right sample based on the value of the LFO
            lfo_val = 0.1;
            float sampleL = sample * lfo_val;
            float sampleR =  sample * (1 - lfo_val); 
            return {sampleL, sampleR};
        }
    }

private:
    // Sine shaped LFO
    SinOsc lfo;
};
