#pragma once
#ifndef _LEDCUBEEDITOR_MAIN_H_
#define _LEDCUBEEDITOR_MAIN_H_

#include <cstdint>
#include <vector>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

constexpr int CUBE_SIZE = 8;

struct Frame
{
    uint8_t voxels[CUBE_SIZE][CUBE_SIZE][CUBE_SIZE] = {};
};

void setupRenderer();
void destroyRenderer();
void mainLoop(std::vector<Frame> &frames);
void drawCube3D(const uint8_t cube[8][8][8], GLuint shaderProgram, GLuint cubeVAO,
    const glm::mat4 &view, const glm::mat4 &projection);
void exportCBIN(std::vector<Frame> &frames, int delay, bool loop);
void importCBIN(std::vector<Frame> &frames, int &delay, bool &loop);

#endif