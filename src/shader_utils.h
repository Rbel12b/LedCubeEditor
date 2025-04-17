// shader_utils.h
#pragma once
#ifndef _SHADER_UTILS_H_
#define _SHADER_UTILS_H_

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glad/glad.h>

GLuint LoadShaderProgram(const char* vertexPath, const char* fragmentPath);
#endif