#include <iostream>
#include <vector>
#include <set>
#include "GameOfLife.h"
#include <random>
#include <algorithm> 
using namespace std;





int main()
{
    GameOfLife gameofLife;

    // Initialise the game of life state for each voice
    std::vector<std::pair<int, int>> initialState;
    initialState.clear();

    int preset;
    cin >> preset;

    if (preset == 1)
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
    } else if (preset == 2)
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
    } else if (preset == 3)
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
    } else if (preset == 4)
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

    } else if (preset == 5)
    {
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
    } else if (preset == 6)
    {

        int numberOfCells;
        std::cout << "How many cells should be filled?\n";
        std::cin >> numberOfCells;

        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<std::mt19937::result_type> dist_width(0, 15);
        std::uniform_int_distribution<std::mt19937::result_type> dist_height(0, 7);

        std::set<std::pair<int, int>> usedPositions;

        while (usedPositions.size() < numberOfCells) {
            int randomWidth = dist_width(rng);
            int randomHeight = dist_height(rng);
            usedPositions.insert({randomWidth, randomHeight}); // set automatically handles duplicates
        }

        initialState.assign(usedPositions.begin(), usedPositions.end());
    }

    gameofLife.setInitialState(initialState);
    gameofLife.visualize();

    int signal = 3;

    while (signal != 0)
    {
        gameofLife.update();
        gameofLife.visualize();
        cin >> signal;
    }


    return 0;
}
