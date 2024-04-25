#include <iostream>
#include "ToneMatrixTest.h"
#include "Oscillators.h"
using namespace std;

int main()
{
    ToneMatrix toneMatrix;
    toneMatrix.setInitialState(0, 40, 44100.0f);

    toneMatrix.getGame().visualize();
    for (int j = 0; j < 5; j++)
    {
        for (int i = 0; i < toneMatrix.getGame().getWidth(); i++)
        {
            cout << toneMatrix.process() << "\n";
            toneMatrix.incrementColumnAndWrap();
        }
        toneMatrix.getGame().visualize();
    }
    return 0;
}
