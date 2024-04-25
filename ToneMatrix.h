#include "GameOfLife.h"
#include "Oscillators.h"
#include <vector>
#include <set>
#include <random>
#include <algorithm>

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
        phasors.clear();
        for (int i = 0; i < gameOfLife.getHeight(); i++)
        {
            TriOsc triOsc;
            triOsc.setSampleRate(sampleRate);
            phasors.push_back(triOsc);
        }
    }

    GameOfLife getGame()
    {
        return gameOfLife;
    }

    void processColumn(int columnIndex)
    {
        std::vector<int> midiNotesToPlay = checkColumn(columnIndex);
        
        for (auto& osc : phasors)
        {
            osc.setFrequency(0);
        }

        if (!midiNotesToPlay.empty())
        {
            for (int i = 0; i < midiNotesToPlay.size(); i++)
            {
                phasors[i].setFrequency(juce::MidiMessage::getMidiNoteInHertz(midiNotesToPlay[i]));
            }     
        }
    }

    float process()
    {
        processColumn(currentColumn);
        // Store the output of all of the oscillators 
        float sample = 0;

        // Increment the sample with each oscillator
        for (auto& osc : phasors)
        {
            sample += osc.process(); 
            juce::Logger::writeToLog(std::to_string(sample));
            
        }

        // Normalise the sample and return it
        if (!checkColumn(currentColumn).empty())
        {
            //juce::Logger::writeToLog("Sample produced by game of life");
            //juce::Logger::writeToLog("Number of notes to play");
            //juce::Logger::writeToLog(std::to_string(checkColumn(currentColumn).size()));
            //juce::Logger::writeToLog(std::to_string(sample / checkColumn(currentColumn).size()));
            return sample / checkColumn(currentColumn).size();
        }
        else {
            return 0;
        }
    }

    void incrementColumnAndWrap()
    {
        // If we are at the last column, update the board
        //juce::Logger::writeToLog("Incrementing column");
        if ((currentColumn + 1) == gameOfLife.getWidth())
        {


            //juce::Logger::writeToLog("Column is at end");

            //juce::Logger::writeToLog(std::to_string(currentColumn));
            //juce::Logger::writeToLog("Updating board");
            gameOfLife.update();
        }
        currentColumn = (currentColumn + 1) % gameOfLife.getWidth();
    }

private:
    GameOfLife gameOfLife;
    std::vector<TriOsc> phasors;
    int currentColumn = 0;
};
