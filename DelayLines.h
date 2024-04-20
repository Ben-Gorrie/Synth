/*
  ==============================================================================

    DelayLines.h
    Created: 14 Mar 2024 5:30:03pm
    Author:  ben

  ==============================================================================
*/
#include <vector>
#pragma once
class DelayLines
{
public:
    void setSize(int _newSize)
    {
        size = _newSize;
        buffer.resize(size, 0.0f);
    }

    void setDelayTime(float _delayTimeInSamples)
    {
        delayTime = _delayTimeInSamples;
        readPos = writePos - delayTime;
        if (readPos < 0) readPos += size;
    }

    void setFeedback(float _feedback)
    {
        feedback = _feedback;
        feedback = juce::jlimit(0.0f, 1.0f, feedback);
    }

    float process(float inVal)
    {
        float outval = linearInterpolation(); 
        buffer[writePos] = inVal + outval * feedback;
        writePos++;
        if (writePos >= size)
        {
            writePos  -= size;
        }
        readPos++;
        if (readPos >= size) readPos -= size;
        return outval;
    }

private:
    int size;
    float delayTime;
    float readPos = 0;
    int writePos = 1;
    std::vector<float> buffer;
    float feedback = 0.75;

    float linearInterpolation()
    { 
        int indexA = floor(readPos);
        int indexB = indexA + 1;
        indexB %= size;

        float valA = buffer[indexA];
        float valB = buffer[indexB];

        float frac = readPos - indexA;
        return (1 - frac) * valA + frac * valB;
    }
};
