// shader_utils.cpp
#include "shader_utils.h"

GLuint LoadShaderProgram(const char* vertexPath, const char* fragmentPath) {
    std::string vCode, fCode;
    std::ifstream vShaderFile(vertexPath), fShaderFile(fragmentPath);

    if (!vShaderFile || !fShaderFile) {
        std::cerr << "Error loading shader files.\n";
        return 0;
    }

    std::stringstream vStream, fStream;
    vStream << vShaderFile.rdbuf();
    fStream << fShaderFile.rdbuf();
    vCode = vStream.str();
    fCode = fStream.str();

    const char* vSource = vCode.c_str();
    const char* fSource = fCode.c_str();

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vSource, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fSource, NULL);
    glCompileShader(fragmentShader);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}