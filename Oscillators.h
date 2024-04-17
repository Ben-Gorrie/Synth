#pragma once

#include <cmath>


/**
 *  Class that controls basic oscillation. This class is inherited to create more interesting oscillatons
 */
class Phasor {
public:

    /**
     *  Increment the phase and wrap it back to 0 if it exceeds 1
     *  @return The phase passed through the output function. For this specific class, this just returns phase
     */
    float process()
    {
        // Increment phase
        phase += phaseDelta;

        // Warp phase if greater than 1
        if (phase > 1.0)
        {
            phase -= 1.0f;
        }

        // Return the phase passed through output()
        return output(phase);
    }

    /**
     *  Virtual function to change the output of process()
     *  @param p Phase of the phasor
     *  @return Phase here. Will be overwritten when inherited 
     */
    virtual float output(float p)
    {
        return p;
    }

    /**
     *  Set the sample rate of the phasor
     *  @param SR Sample rate to set phasor
     */
    void setSampleRate(float SR)
    {
        sampleRate = SR;
    }

    /**
     *  Sets frequency and phasedelta of phasor
     *  @param freq Frequency to set phasor to 
     */
    void setFrequency(float freq)
    {
        frequency = freq;

        // Compute the phase delta using frequency and sample rate
        phaseDelta = frequency / sampleRate;
    }

    /**
     *  Return frequency of the phasor
     *  @return Frequency of phasor
     */
    float getFrequency()
    {
        return frequency;    
    }

    /**
     *  Sets the phase of the phasor
     *  @param _phase Phase to set the phasor to
     */
    void setPhase(float _phase)
    {
        phase = _phase;
    }

    /**
     *  Returns the phase of the phasor
     *  @return Phase of phasor
     */
    float getPhase()
    {
        return phase;  
    }

private:
    // Variables that store frequency, sample rate, phase and phase delta of phasor respectively
    float frequency;
    float sampleRate;
    float phase = 0.0f;
    float phaseDelta;
};


/**
 *  Class which inherits from Phasor. Creates a triangular oscillator
 */
class TriOsc : public Phasor
{
    /**
     *  Override the base phasor function to generate a triangular wave
     *  @param p Phase of the phasor
     */
    float output(float p) override
    {
        return fabsf(p - 0.5f) - 0.5f;
    }
};

/**
 *  Class which inherits from Phasor. Creates a sinusoidal oscillator
 */
class SinOsc : public Phasor
{
    /**
     *  Override the base phasor function to generate a sine wave
     *  @param p Phase of the phasor
     */
    float output(float p) override
    {
        return sin(p * 2.0 * M_PI);
    }
};

/**
 *  Class which inherits from Phasor. Creates a square oscillator
 */
class SquareOsc : public Phasor
{
public:
    /**
     *  Override the base phasor function to generate a square wave. Also tracks when the square wave has just flipped
     *  @param p Phase of the phasor
     */
    float output(float p) override
    {
        // If the phase is less than the pulsewidth return 0.5. If not, return -0.5
        float outVal = 0.5;

        if (p > pulseWidth)
        {
            outVal = -0.5;
        }
        
        // Compare the current value with the previous value. If they are close together, the wave has not flipped. If not, set hasJustTurnedOn to true
        if (abs(previousOutVal - outVal) < 0.001)
        {
            hasJustTurnedOn = false;
        }
        else 
        {
            hasJustTurnedOn = true;    
        }

        // Store the current value and return the current value
        previousOutVal = outVal;
        return outVal;
    }

    /**
     *  Set the puslsewidth of the square wave (how long it is "on" for)
     *  @param pw The pulsewidth the square wave should have
     * */
    void setPulseWidth(float pw)
    {
        pulseWidth = pw;
    }

    /**
     *  Wrapper function to determine if the wave has just flipped or not
     *  @return Boolean which states if tehe wave has just flipped
     */
    bool getHasJustTurnedOn()
    {
        return hasJustTurnedOn;
    }

private:
    // Variable which store pulse width of the square wave, its previous output value and whether or not it has just been turned on
    float pulseWidth = 0.5f;
    float previousOutVal = 0; 
    bool hasJustTurnedOn = false;
};

