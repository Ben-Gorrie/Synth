#include "GameOfLife.h"
#include "Oscillators.h"
#include <vector>
#include <set>
#include <random>
#include <algorithm>

class ToneMatrix
{
public:
    ToneMatrix()
    {
        phasors.clear();    
        for (int i = 0; i < gameOfLife.getHeight(); i++)
        {
            TriOsc triOsc;
            phasors.push_back(triOsc);
        }
    }

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

    void setInitialState(std::atomic<float>* choiceParam, std::atomic<float>* randomNumberOfCellsParam, float sampleRate)
    {
        int choiceNumber = choiceParam->load();
        int randomNumberOfLiveCells = randomNumberOfCellsParam->load();

        gameOfLife.setInitialState(presetStates(choiceNumber, randomNumberOfLiveCells));
        for (int i = 0; i < gameOfLife.getHeight(); i++)
        {
            phasors[i].setSampleRate(sampleRate);
        }
    }

    GameOfLife getGame()
    {
        return gameOfLife;
    }

    void processColumn(int columnIndex)
    {
        std::vector<int> midiNotesToPlay = checkColumn(columnIndex);
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
        for (int i = 0; i < checkColumn(currentColumn).size(); i++)
        {
            sample += phasors[i].process();
        }

        // Normalise the sample and return it
        if (!checkColumn(currentColumn).empty())
        {
            return sample / checkColumn(currentColumn).size();
        }
        else {
            return 0;
        }
    }

    void incrementColumnAndWrap()
    {
        // If we are at the last column, update the board
        if ((currentColumn + 1) == gameOfLife.getWidth())
        {
            gameOfLife.update();
        }
        currentColumn = (currentColumn + 1) % gameOfLife.getWidth();
    }

private:
    GameOfLife gameOfLife;
    std::vector<TriOsc> phasors;
    int currentColumn = 0;


    std::vector<std::pair<int, int>> presetStates(int choiceInt, int randomNumberOfLiveCells)
    {
        std::vector<std::pair<int, int>> initialState;
        initialState.clear();

        if (choiceInt == 0)
        {
            // cube
            initialState.push_back({6, 3});
            initialState.push_back({7, 3});
            initialState.push_back({8, 3});
            initialState.push_back({6, 4});
            initialState.push_back({7, 4});
            initialState.push_back({8, 4});
            initialState.push_back({6, 5});
            initialState.push_back({7, 5});
            initialState.push_back({8, 5});

        } else if (choiceInt == 1)
        {
            // pentadecathlon
            initialState.push_back({3, 4});
            initialState.push_back({4, 4});
            initialState.push_back({5, 3});
            initialState.push_back({5, 5});
            initialState.push_back({6, 4});
            initialState.push_back({7, 4});
            initialState.push_back({8, 4});
            initialState.push_back({9, 3});
            initialState.push_back({9, 5});
            initialState.push_back({10, 4});
            initialState.push_back({11, 4});

        } else if (choiceInt == 2)
        {
            // tumbler
            initialState.push_back({4, 2});
            initialState.push_back({4, 3});
            initialState.push_back({5, 1});
            initialState.push_back({6, 2});
            initialState.push_back({6, 4});
            initialState.push_back({6, 5});
            initialState.push_back({7, 3});
            initialState.push_back({7, 5});
            initialState.push_back({9, 3});
            initialState.push_back({9, 5});
            initialState.push_back({10, 2});
            initialState.push_back({10, 4});
            initialState.push_back({10, 5});
            initialState.push_back({11, 1});
            initialState.push_back({12, 2});
            initialState.push_back({12, 3});

        } else if (choiceInt == 3)
        {
            //figure eight
            initialState.push_back({5, 4});
            initialState.push_back({5, 5});
            initialState.push_back({5, 6});
            initialState.push_back({6, 4});
            initialState.push_back({6, 5});
            initialState.push_back({6, 6});
            initialState.push_back({7, 4});
            initialState.push_back({7, 5});
            initialState.push_back({7, 6});
            initialState.push_back({8, 1});
            initialState.push_back({8, 2});
            initialState.push_back({8, 3});
            initialState.push_back({9, 1});
            initialState.push_back({9, 2});
            initialState.push_back({9, 3});
            initialState.push_back({10, 1});
            initialState.push_back({10, 2});
            initialState.push_back({10, 3});

        } else if (choiceInt == 4)
        {
            // Octagon 2
            initialState.push_back({4, 3});
            initialState.push_back({4, 4});
            initialState.push_back({5, 2});
            initialState.push_back({5, 5});
            initialState.push_back({6, 1});
            initialState.push_back({6, 6});
            initialState.push_back({7, 0});
            initialState.push_back({7, 7});
            initialState.push_back({8, 0});
            initialState.push_back({8, 7});
            initialState.push_back({9, 1});
            initialState.push_back({9, 6});
            initialState.push_back({10, 2});
            initialState.push_back({10, 5});
            initialState.push_back({11, 3});
            initialState.push_back({11, 4});

        } else if (choiceInt == 5)
        {
            std::random_device dev;
            std::mt19937 rng(dev());
            std::uniform_int_distribution<std::mt19937::result_type> dist_width(0, 15);
            std::uniform_int_distribution<std::mt19937::result_type> dist_height(0, 7);

            std::set<std::pair<int, int>> usedPositions;

            while (usedPositions.size() < randomNumberOfLiveCells) {
                int randomWidth = dist_width(rng);
                int randomHeight = dist_height(rng);
                usedPositions.insert({randomWidth, randomHeight}); // set automatically handles duplicates
            }

            initialState.assign(usedPositions.begin(), usedPositions.end());
        }

        return initialState;
    }

};
