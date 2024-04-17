#include "GameOfLife.h"

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

private:
    GameOfLife gameOfLife;
};
