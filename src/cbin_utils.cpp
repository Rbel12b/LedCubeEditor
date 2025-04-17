#include <cstdint>
#include <fstream>
#include <tinyfiledialogs.h>
#include <iostream>
#include "main.h"

// Export frames to .cbin
void exportCBIN(std::vector<Frame> &frames, int delay, bool loop)
{
    const char *filter_patterns[] = {"*.cbin"};
    const char *file = tinyfd_saveFileDialog(
        "Choose a file",
        "Cube.cbin",
        1,
        filter_patterns,
        "cbin files");

    if (file)
    {
        std::cout << "You selected: " << file << std::endl;
    }
    else
    {
        std::cout << "No file selected." << std::endl;
        return;
    }
    std::ofstream out(file, std::ios::binary);
    uint32_t numFrames = frames.size();
    out.write(reinterpret_cast<const char *>(&numFrames), 4);
    out.write(reinterpret_cast<const char *>(&delay), 4);
    out.put(loop ? 1 : 0);
    for (const auto &frame : frames)
    {
        for (int z = 0; z < CUBE_SIZE; z++)
        {
            for (int x = 0; x < CUBE_SIZE; ++x)
            {
                uint8_t byte = 0;
                for (int y = 0; y < CUBE_SIZE; ++y)
                {
                    byte |= frame.voxels[CUBE_SIZE - x - 1][CUBE_SIZE - 1 - y][z] ? (1 << y) : 0;
                }
                out.put(byte);
            }
        }
    }
    out.close();
}

void importCBIN(std::vector<Frame> &frames, int &delay, bool &loop)
{
    const char *filter_patterns[] = {"*.cbin"};
    const char *file = tinyfd_openFileDialog(
        "Choose a file",
        "Cube.cbin",
        1,
        filter_patterns,
        "cbin files",
        0);

    if (file)
    {
        std::cout << "You selected: " << file << std::endl;
    }
    else
    {
        std::cout << "No file selected." << std::endl;
        return;
    }
    std::ifstream in(file, std::ios::binary);
    uint32_t numFrames;
    in.read(reinterpret_cast<char *>(&numFrames), 4);
    in.read(reinterpret_cast<char *>(&delay), 4);
    in.read(reinterpret_cast<char *>(&loop),1);
    frames.clear();
    frames.resize(numFrames);
    for (int i = 0; i < numFrames; ++i)
    {
        frames[i] = Frame();
    }
    for (auto &frame : frames)
    {
        for (int z = 0; z < CUBE_SIZE; z++)
        {
            for (int x = 0; x < CUBE_SIZE; ++x)
            {
                uint8_t byte = 0;
                in.read(reinterpret_cast<char *>(&byte), 1);
                for (int y = 0; y < CUBE_SIZE; ++y)
                {
                    frame.voxels[CUBE_SIZE - x - 1][CUBE_SIZE - 1 - y][z] = (byte & (1 << y)) ? 1 : 0;
                }
            }
        }
    }
    in.close();
}