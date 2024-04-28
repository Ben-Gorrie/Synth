#include "GameOfLife.h"
#include "Oscillators.h"
#include <set>
#include <random>
#include <algorithm>

/**
 *  Class to simulate a musical tone matrix using the Game of Life automaton rules.
 *  It manages a set of oscillators that generate tones based on the cellular states of the game of life.
 */
class ToneMatrix
{
public:
    /**
     *  Constructor to initialize the tone matrix.
     *  The number of oscillators matches the height of the Game of Life grid.
     */
    ToneMatrix()
    {
        phasors.clear();
        
        // Populate the vector with triangular oscillators.
        for (int i = 0; i < gameOfLife.getHeight(); i++)
        {
            TriOsc triOsc;
            phasors.push_back(triOsc);
        }
    }

    /**
     *  Checks a column in the Game of Life grid to determine which notes should be played.
     *  @param columnIndex The index of the column to check.
     *  @return Vector of MIDI notes corresponding to the indexes of all live cells in the column.
     */
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
    
    /**
     *  Sets the initial state of the tone matrix based on user parameters and the sample rate.
     *  @param choiceParam Pointer to the user choice parameter, affecting initial configuration.
     *  See the function presetStates() for information about which initial states are possible.
     *  Keep in mind that choosing the random option will lead to uncertain behaviour with the game of life as it is non-deterministic.
     *  It is entirely possible that a randomly set game of life ends up empty after a few updates, which is not very interesting.
     *  @param randomNumberOfCellsParam Pointer to the random number of cells parameter.
     *  If choiceParam is set to "random" (represented internally as 5), then this parameter determines how many initial cells are alive. 
     *  However, their configuration will be random.      *  @param sampleRate The sample rate for audio processing.
     */
    void setInitialState(std::atomic<float>* choiceParam, std::atomic<float>* randomNumberOfCellsParam, float sampleRate)
    {
        int choiceNumber = choiceParam->load();
        int randomNumberOfLiveCells = randomNumberOfCellsParam->load();

        // Determine which preset state should be used and set it to the game of life grid.
        std::vector<std::pair<int, int>> presetState = presetStates(choiceNumber, randomNumberOfLiveCells);
        gameOfLife.setInitialState(presetState);

        // Set the sample rate for each oscillator
        for (int i = 0; i < gameOfLife.getHeight(); i++)
        {
            phasors[i].setSampleRate(sampleRate);
        }
    }
    
    /**
     *  Returns the game of life object.
     *  @return Game of life object.
     */
    GameOfLife getGame()
    {
        return gameOfLife;
    }

    /**
     *  Processes a specified column by setting the frequency of the oscillators based on MIDI notes derived from the Game of Life state.
     *  @param columnIndex The column index to process for frequency setting.
     */
    void setFrequencies(int columnIndex)
    {
        std::vector<int> midiNotesToPlay = checkColumn(columnIndex);
        if (!midiNotesToPlay.empty())
        {
            for (int i = 0; i < midiNotesToPlay.size(); i++)
            {
                float frequencyToSet = juce::MidiMessage::getMidiNoteInHertz(midiNotesToPlay[i]);
                phasors[i].setFrequency(frequencyToSet);
            }     
        }
    }

    /**
     *  Processes the current game of life state to produce a combined audio output from all oscillators. Note this will only consider one column at a time.
     *  In order to change this, incrementColumnAndWrap() should be called.
     *  This function should be called repeatedly in a real-time audio processing loop.
     *  @return The normalized mixed audio output from all oscillators for the current frame.
     */
    float process()
    {
        // Set the frequencies of the oscillators
        setFrequencies(currentColumn);

        // Check which midi notes should played
        std::vector<int> midiNotesToPlay = checkColumn(currentColumn);

        // Store the output of all of the oscillators 
        float sample = 0;

        // Increment the sample with each oscillator that needs to play
        for (int i = 0; i < midiNotesToPlay.size(); i++)
        {
            sample += phasors[i].process();
        }

        // Normalise the sample and return it
        if (!midiNotesToPlay.empty())
        {
            return sample / midiNotesToPlay.size();
        }
        else {
            return 0;
        }
    }

    /**
     *  Increment the index of the column we should currently be considering. 
     *  If we are at the last column, the board should be updated according to the rules of the game of life and return to the first (zeroth) column.
     * */
    void incrementColumnAndWrap()
    {
        int boardWidth = gameOfLife.getWidth();
        // If we are at the last column, update the board
        if ((currentColumn + 1) == boardWidth)
        {
            gameOfLife.update();
        }

        // Increment the column, making sure to wrap the count when at the end.
        currentColumn = (currentColumn + 1) % boardWidth;
    }

   
    /**
     *  This function allows the game of life to automatically change parameters, even with them being exposed to the host.
     *  If the current column we are considering for the game of life is empty, the parameter should decrease.
     *  Conversely, if the column has cells that are alive, the parameter should increase. The more cells are alive, the more pronounced this increase should be.
     *  There is some random element added so that the amount by which parameters increase and decrease is not the same each time.
     *  This function is the reason why the parameters that the game of life should be allowed to control cannot simply be std::atomic<float>*
     *  as we need functions to modify the value of the parameter, determine range and ID.
     *  @param parameter Parameter that the tone matrix should modify.
     * */
    void changeSliderParamAccordingToColumn(juce::RangedAudioParameter* parameter)
    {
        // Get the maximum and minimum allowed values for the parameter.
        float minVal = parameter->getNormalisableRange().start;
        float maxVal = parameter->getNormalisableRange().end;

        // Normalised maximum and minimum values any parameter can take (as setValueNotifyingHost() takes values between 0 and 1)
        float fixedMaxVal = 1.0f;
        float fixedMinVal = 0.0f;

        // If the parameter we wish to modify has to do with reverb, clamp the minimum and maximum values to 0.05 and 0.75 respectively.
        // This is to prevent reverb from getting maxed out randomly (which is very loud) or it getting set too low (which makes the audio too quiet).
        // I could have done something similar for the low-pass filter, but I decided against letting the game of life change its parameters.
        if (parameter->getParameterID().contains("reverb"))
        {
            fixedMaxVal = 0.75f;
            fixedMinVal = 0.05f;
        }

        // Normalize the current parameter value between 0 and 1.
        float currentValue = parameter->getValue();
        float normalizedCurrentValue = (currentValue - minVal) / (maxVal - minVal); 

        // Determine by how much the normalised parameter should increase or decrease (if the parameter increases, further processing is done to this increment).
        float incrementProp = 0.01f + (random.nextFloat() / 10);

        std::vector<int> midiNotesToPlay = checkColumn(currentColumn);
        if (!midiNotesToPlay.empty())
        {
            // Scale the increment by the number of life cells in the current column.
            float increment = midiNotesToPlay.size() * incrementProp;

            // Apply increment and clamp the value between 0 and 1 (or 0.05 and 0.75 if reverb)
            float newNormalizedValue = std::min(normalizedCurrentValue + increment, fixedMaxVal);
            parameter->setValueNotifyingHost(newNormalizedValue);
        } else 
        {
            float decrement = incrementProp;

            // Apply decrement and clamp the value between 0 and 1 (or 0.05 and 0.75 if reverb)
            float newNormalizedValue = std::max(normalizedCurrentValue - decrement, fixedMinVal);
            parameter->setValueNotifyingHost(newNormalizedValue);
        }
    }

private:
    // Object processing the game of life
    GameOfLife gameOfLife;

    // Triangular oscillators to play when a live cell is found
    std::vector<TriOsc> phasors;

    // The current column of the gmae of life the tone matrix is considering
    int currentColumn = 0;

    // Random number generator to change the parameter increments
    juce::Random random;


    /**
     *  Determines which preset state should be used on the game of life.
     *  @param choiceInt Number representing which choice to choose. 
     *  This number is a controllable parameter, as this function is only ever used internally after the value from the parameter has been extracted (hence the int representation).
     *  @param randomNumberOfLiveCells Similarly to above, this is a controllable parameter. 
     *  If the choice parameter is "random", this determines how many random cells should start off alive in the game of life.
     *  @return A vector containing pairs of integers. The first integer is the index of the column, and the second the index of the row. The vector holds the positions of all initially alive cells.
     * */
    std::vector<std::pair<int, int>> presetStates(int choiceInt, int randomNumberOfLiveCells)
    {
        std::vector<std::pair<int, int>> initialState;
        initialState.clear();

        if (choiceInt == 0)
        {
            // Cube
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
            // Pentadecathlon
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
            // Tumbler
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
            //Figure eight
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
            // Random

            // Keep track of all currently cused positions
            std::set<std::pair<int, int>> usedPositions;

            // While there are fewer live cells than wanted, add some more
            while (usedPositions.size() < randomNumberOfLiveCells) {
                int randomWidth = random.nextInt(16); 
                int randomHeight =  random.nextInt(8);
                usedPositions.insert({randomWidth, randomHeight}); // set automatically handles duplicates
            }

            initialState.assign(usedPositions.begin(), usedPositions.end());
        }

        return initialState;
    }
};
