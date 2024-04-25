/*
  ==============================================================================

    FilteredWaves.h
    Created: 29 Feb 2024 8:56:30pm
    Author:  ben

  ==============================================================================
*/

#pragma once
#include <vector>
#include "Oscillators.h"


/**
 * Class that applies a changing low pass filter to a sample 
 */
class ChangingFilter
{
public:
    
    /**
     *  Creates LFOs
     *  @param sampleRate Sample rate to set LFOs to
     *  @param numWaves Number of LFOs to create
     */
    void createLFOs(float sampleRate, int numWaves = 5)
    {
        // Initialise the lfos vector
        lfos.clear();
        for (int i = 0; i < numWaves; i++)
        {
            // Create an LFO, set its sample rate and add it to a vector
            SinOsc lfo;
            lfo.setSampleRate(sampleRate);
            lfos.push_back(lfo);
        }

        // Store the sample rate for later use
        sr = sampleRate;
    }
    
    /**
     *  Sets the frequencies of the LFOs
     *  @param frequency The base frequency of the LFOs
     *  @param frequencyDiff The difference in consecutive frequency between the LFOs
     */
    void setLFOFrequencies(float frequency, float frequencyDiff)
    {
        for (int i = 0; i < lfos.size(); i++)
        {
            // Set the frequency of the LFO, taking frequencyDiff into account
            lfos[i].setFrequency(frequency + i * frequencyDiff); 
        }
    }

    /**
     *  Set the cutoff of the low pass filter. This can either be done dynamically (in the DSP) loop or only once depending on whether one wants a changing filter or not 
     *  @param baseCutoff Base cutoff of the low pass filter
     *  @param modulationDepth Measure of how much the cutoff can change
     */
    void setCutoff(float baseCutoff, float modulationDepth)
    {
        // Compute the average modulation using all LFOs in the vector
        float lfoModulation = 0;
        for (int i = 0; i < lfos.size(); i++)
        {
             lfoModulation += lfos[i].process();
        }

        float averageLFOModulation = lfoModulation / lfos.size();

        // Set cutoff based on the baseCutoff, averageLFOModulation and modulationDepth
        cutoff = baseCutoff + averageLFOModulation * modulationDepth; 
    }

    /**
     *  To be called whenever the filter should be reset. Wrapper function
     */
    void initFilter()
    {
        filter.reset();
    }

    /**
     *  Set the coefficients of the low pass filter.
     *  @param resonance Resonance to apply to the low pass filter
     */
    void setFilterCoefs(float resonance)
    {
        filter.setCoefficients(juce::IIRCoefficients::makeLowPass(sr, cutoff, resonance));
    }

    /**
     *  Set the coefficients of the low pass filter.
     */
    void setFilterCoefs()
    {
        filter.setCoefficients(juce::IIRCoefficients::makeLowPass(sr, cutoff));
    }

    /**
     *  Processes a single sample through the low pass filter
     *  @param sample Sample to pass through the filter
     */
    float process(float sample)
    {
        return filter.processSingleSampleRaw(sample);
    }

private:
    // Vector to hold LFOs
    std::vector<SinOsc> lfos;

    // Filter to use
    juce::IIRFilter filter;

    // Cutoff value for filter
    float cutoff;

    // Sample rate. We store this as it prevents having to ask for sample rate as a parameter in setFilterCoefs()
    float sr;
};
