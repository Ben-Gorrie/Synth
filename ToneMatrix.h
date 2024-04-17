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
        sinOscs.clear();
        for (int i = 0; i < gameOfLife.getHeight(); i++)
        {
            SinOsc sinOsc;
            sinOsc.setSampleRate(sampleRate);
            sinOscs.push_back(sinOsc);
        }
    }

    GameOfLife getGame()
    {
        return gameOfLife;
    }

    float process()
    {
        std::vector<int> midiNotesToPlay = checkColumn(4);
        if (midiNotesToPlay.empty())
        {
            for (auto& osc : sinOscs)
            {
                osc.setFrequency(0);
            }

        }
        else 
        {
            for (int i = 0; i < midiNotesToPlay.size(); i++)
            {
                sinOscs[i].setFrequency(juce::MidiMessage::getMidiNoteInHertz(midiNotesToPlay[i]));
            }     
        }
       
    }


private:
    GameOfLife gameOfLife;
    std::vector<SinOsc> sinOscs;
};
