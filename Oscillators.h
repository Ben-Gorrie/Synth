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


        float modulatedPhase = phase + modulationIndex * modulationValue;

        // Return the phase passed through output()
        return output(modulatedPhase);
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

    void setModulationIndex(float mi) 
    {
        modulationIndex = mi;
    }

    void setModulationValue(float mv)
    {
        modulationValue = mv;
    }


private:
    // Variables that store frequency, sample rate, phase and phase delta of phasor respectively
    float frequency;
    float sampleRate;
    float phase = 0.0f;
    float phaseDelta;

protected:
    float modulationIndex = 0.0f;
    float modulationValue = 0.0f;
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
    float output(float modulatedPhase) override
    {
        // Wrap the modulatedPhase back into the 0 to 1 range if needed.
        modulatedPhase = modulatedPhase - floor(modulatedPhase);

        return 4 * (fabsf(modulatedPhase - 0.5f) - 0.25f);
    }
};

/**
 *  Class which inherits from Phasor. Creates a sinusoidal oscillator
 */
class SinOsc : public Phasor
{
    /**
     *  Override the base phasor function to generate a sine wave
     *  Allows for phase modulation if the variables modulationIndex and modulationValue are not 0
     *  @param p Phase of the phasor
     */
    float output(float modulatedPhase) override
    {
        return sin(modulatedPhase * 2.0 * M_PI);
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
    float output(float modulatedPhase) override
    {
        // Wrap the modulatedPhase back into the 0 to 1 range if needed.
        modulatedPhase = modulatedPhase - floor(modulatedPhase);

        // If the phase is less than the pulsewidth return 0.5. If not, return -0.5
        float outVal = 0.5;

        if (modulatedPhase > pulseWidth)
        {
            outVal = -0.5;
        }
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

private:
    // Variable which store pulse width of the square wave
    float pulseWidth = 0.5f;
};

class SawOsc : public Phasor
{
   /**
     *  Override the base phasor function to generate a sawtooth wave
     *  Allows for phase modulation if the variables modulationIndex and modulationValue are not 0
     *  @param p Phase of the phasor
     */
    float output(float modulatedPhase) override
    {
        // Wrap the modulatedPhase back into the 0 to 1 range if needed.
        modulatedPhase = modulatedPhase - floor(modulatedPhase);

        return modulatedPhase;
    }
};
