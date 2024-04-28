#include <vector>

/**
 *  Class to simulate Conway's Game of Life on a grid.
 * */ 
class GameOfLife 
{
public:
    /**
     *  Constructor to initialize the grid with specified dimensions.
     *  The grid cells are initialized as dead (false).
     *  Note that if used as a tone matrix (as is done in the context of the synth), the number of cells must not exceed the total number of midi notes (8*16 = 128).
     * */
    GameOfLife(int width = 16, int height = 8) : width(width), height(height) {
        grid.resize(width * height);
    }

    /**
     *  Update the grid to the next generation based on the Game of Life rules.
     * */
    void update() {
        std::vector<bool> newGrid(width * height);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int aliveNeighbors = countAliveNeighbors(x, y);
                bool isAlive = grid[y * width + x];
                // A cell survives if it has 2 or 3 neighbors, or it comes to life with exactly 3 neighbors.
                bool newState = (isAlive && (aliveNeighbors == 2 || aliveNeighbors == 3)) || (!isAlive && aliveNeighbors == 3);
                newGrid[y * width + x] = newState;
            }
        }
        // Swap the old grid with the new updated grid.
        grid.swap(newGrid);
    }

    /**
     *  Set the initial live cells on the grid based on input coordinates.
     * */
    void setInitialState(const std::vector<std::pair<int, int>>& liveCells) {
        for (const auto& cell : liveCells) {
            if (cell.first >= 0 && cell.first < width && cell.second >= 0 && cell.second < height) {
                grid[cell.second * width + cell.first] = true;
            }
        }
    }

    /**
     *  Visualize the current state of the grid to the console output. Used for debugging.
     * */
    void visualize() {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                std::cout << (grid[y * width + x] ? '#' : '.');
            }
            std::cout << '\n';
        }
        std::cout << std::string(width, '@') << '\n'; // Separator line for clarity
    }
    
    /**
     *  Get the current state of the grid.
     * */
    std::vector<bool> getGrid() {
        return grid;
    }

    /**
     *  Get the width of the grid.
     * */
    int getWidth() {
        return width;
    }

    /**
     *  Get the height of the grid.
     * */
    int getHeight() {
        return height;
    }

private:
    /**
     *  Helper function to count alive neighbors of a cell. 
     *  @param x Row index of cell to check
     *  @param Column index of cell to check
     *  @return Number of alive neighbouring cells
     * */
    int countAliveNeighbors(int x, int y) {
        int count = 0;
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                if (dx == 0 && dy == 0) continue;  // Skip the current cell itself.
                int nx = x + dx, ny = y + dy;
                if (nx >= 0 && nx < width && ny >= 0 && ny < height)
                    count += grid[ny * width + nx] ? 1 : 0;
            }
        }
        return count;
    }

    int width, height;  // Dimensions of the grid.
    std::vector<bool> grid;  // Grid to store the state of each cell. Note that although this is one dimensional, if the width is known, we can essentially convert this to a 2d grid.
};
