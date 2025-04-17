#include <vector>
#include "main.h"
std::vector<Frame> frames;

int main()
{
    frames.emplace_back(); // one empty frame
    setupRenderer();
    mainLoop(frames);
    destroyRenderer();
    return 0;
}