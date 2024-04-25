#include <vector>
class GameOfLife 
{
public:
    GameOfLife(int width = 16, int height = 8) : width(width), height(height) {
        grid.resize(width * height);
    }

    void update() {
        std::vector<bool> newGrid(width * height);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int aliveNeighbors = countAliveNeighbors(x, y);
                bool isAlive = grid[y * width + x];
                bool newState = (isAlive && (aliveNeighbors == 2 || aliveNeighbors == 3)) || (!isAlive && aliveNeighbors == 3);
                newGrid[y * width + x] = newState;
            }
        }
        grid.swap(newGrid);
    }

    // Set initial state
    void setInitialState(const std::vector<std::pair<int, int>>& liveCells) {
        for (const auto& cell : liveCells) {
            if (cell.first >= 0 && cell.first < width && cell.second >= 0 && cell.second < height) {
                grid[cell.second * width + cell.first] = true;
            }
        }
    }
    
    // Debugging function
    void visualize() {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                std::cout << (grid[y * width + x] ? '#' : '.');
            }
            std::cout << '\n';
        }
        std::cout << std::string(width, '@') << '\n'; // Separator line for clarity
    }


    std::vector<bool> getGrid()
    {
        return grid;
    }

    int getWidth()
    {
        return width;
    }

    int getHeight()
    {
        return height;
    }   

private:
    int width, height;
    std::vector<bool> grid;

    int countAliveNeighbors(int x, int y) {
        int count = 0;
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                if (dx == 0 && dy == 0) continue;
                int nx = x + dx, ny = y + dy;
                if (nx >= 0 && nx < width && ny >= 0 && ny < height)
                    count += grid[ny * width + nx] ? 1 : 0;
            }
        }
        return count;
    }
};
