#include "GameOfLife.h"
#include "Oscillators.h"

class ToneMatrix
{
public:
    std::vector<int> checkColumn(int columnIndex)
    {
        std::vector<int> midiNotesToPlay;
        std::vector<bool> grid = gameOfLife.getGrid();
        int gridHeight = gameOfLife.getHeight();
        int gridWidth = gameOfLife.getWidth();
        for (int rowIndex = 0; rowIndex < gridHeight; rowIndex++)
        {
            int index = columnIndex + rowIndex * gridWidth;
            if (grid[index])
            {
                midiNotesToPlay.push_back(index);
            }
        }

        return midiNotesToPlay;
    }

    void setInitialState(const std::vector<std::pair<int, int>>& liveCells, int sampleRate)
    {
        gameOfLife.setInitialState(liveCells);
        notesOscs.clear();
        for (int i = 0; i < gameOfLife.getHeight(); i++)
        {
            SinOsc sinOsc;
            sinOsc.setSampleRate(sampleRate);
            notesOscs.push_back(sinOsc);
        }
    }

    GameOfLife getGame()
    {
        return gameOfLife;
    }

    void processColumn(int columnIndex)
    {
        std::vector<int> midiNotesToPlay = checkColumn(columnIndex);
        if (midiNotesToPlay.empty())
        {
            for (auto& osc : notesOscs)
            {
                osc.setFrequency(0);
            }
        }
        else 
        {
            for (auto& osc : notesOscs)
            {
                osc.setFrequency(0);
            }

            for (int i = 0; i < midiNotesToPlay.size(); i++)
            {
                notesOscs[i].setFrequency(juce::MidiMessage::getMidiNoteInHertz(midiNotesToPlay[i]));
            }     
        }
    }

    void process(juce::AudioSampleBuffer& outputBuffer, juce::MidiBuffer& midiMessages, int startSample, int numSamples)
    {
        for (const auto metadata : midiMessages)
        {
            const auto msg = metadata.getMessage();    
            if (msg.isNoteOn())
            {
                // iterate through the necessary number of samples (from startSample up to startSample + numSamples)
                for (int sampleIndex = startSample; sampleIndex < (startSample + numSamples); sampleIndex++)
                {
                    // your sample-by-sample DSP code here!
                    float outputSample = notesOscs[0].process(); 
                    
                    // for each channel, write the currentSample float to the output
                    for (int chan = 0; chan<outputBuffer.getNumChannels(); chan++)
                    {
                        // The output sample is scaled by 0.2 so that it is not too loud by default
                        outputBuffer.addSample(chan, sampleIndex, outputSample * 0.2);
                    }
                }
            }
        }
    }

private:
    GameOfLife gameOfLife;
    std::vector<SinOsc> notesOscs;
    int currentColumn = 0;
    bool playing = false;
};
