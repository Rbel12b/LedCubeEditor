#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "shader_utils.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <chrono>
#include <thread>
#include <algorithm>
#include "main.h"

glm::mat4 projection, view;
glm::vec3 cameraPos, up, target;
auto prevCameraPos = cameraPos;
GLuint voxelShader;
GLuint cubeVAO, cubeVBO;
float cameraDistance = 4.5f;

GLFWwindow *window;
int display_w, display_h;

const int target_fps = 60;
const int frame_time_ms = 1000 / target_fps;

float cubeVertices[] = {
    // positions
    -0.1f, -0.1f, -0.1f, // Back face
    0.1f, -0.1f, -0.1f,
    0.1f, 0.1f, -0.1f,
    0.1f, 0.1f, -0.1f,
    -0.1f, 0.1f, -0.1f,
    -0.1f, -0.1f, -0.1f,

    -0.1f, -0.1f, 0.1f, // Front face
    0.1f, -0.1f, 0.1f,
    0.1f, 0.1f, 0.1f,
    0.1f, 0.1f, 0.1f,
    -0.1f, 0.1f, 0.1f,
    -0.1f, -0.1f, 0.1f,

    -0.1f, 0.1f, 0.1f, // Left face
    -0.1f, 0.1f, -0.1f,
    -0.1f, -0.1f, -0.1f,
    -0.1f, -0.1f, -0.1f,
    -0.1f, -0.1f, 0.1f,
    -0.1f, 0.1f, 0.1f,

    0.1f, 0.1f, 0.1f, // Right face
    0.1f, 0.1f, -0.1f,
    0.1f, -0.1f, -0.1f,
    0.1f, -0.1f, -0.1f,
    0.1f, -0.1f, 0.1f,
    0.1f, 0.1f, 0.1f,

    -0.1f, -0.1f, -0.1f, // Bottom face
    0.1f, -0.1f, -0.1f,
    0.1f, -0.1f, 0.1f,
    0.1f, -0.1f, 0.1f,
    -0.1f, -0.1f, 0.1f,
    -0.1f, -0.1f, -0.1f,

    -0.1f, 0.1f, -0.1f, // Top face
    0.1f, 0.1f, -0.1f,
    0.1f, 0.1f, 0.1f,
    0.1f, 0.1f, 0.1f,
    -0.1f, 0.1f, 0.1f,
    -0.1f, 0.1f, -0.1f};

void setupRenderer()
{
    glfwInit();
    window = glfwCreateWindow(800, 600, "LED Cube Editor", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glfwSetScrollCallback(window, [](GLFWwindow *, double xoffset, double yoffset)
                          {
                              ImGuiIO &io = ImGui::GetIO();
                              io.MouseWheelH += (float)xoffset; // horizontal scroll
                              io.MouseWheel += (float)yoffset;  // vertical scroll
                          });

    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);

    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);

    voxelShader = LoadShaderProgram("shaders/vertex.glsl", "shaders/fragment.glsl");

    cameraPos = glm::vec3(-20.0f, 4.5f, cameraDistance);
    target = glm::vec3(4.5f, 4.5f, 4.5f); // center of 8x8x8 cube
    up = glm::vec3(0.0f, 0.0f, 1.0f);

    view = glm::lookAt(cameraPos, target, up);

    glfwGetFramebufferSize(window, &display_w, &display_h);

    float fov = glm::radians(45.0f); // Field of view
    float aspect = (float)display_w / (float)display_h;
    float near = 0.1f;
    float far = 100.0f;

    projection = glm::perspective(fov, aspect, near, far);
}

void destroyRenderer()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteProgram(voxelShader);
    glfwTerminate();
}

bool rightDragging = false;
bool wheelDragging = false;

int currentFrame = 0;
int delay = 100;
bool loop = true;
int editLayer = 0; // Z layer
bool showMatrixEditor = true;


void mainLoop(std::vector <Frame> &frames)
{
    while (!glfwWindowShouldClose(window))
    {
        auto start = std::chrono::high_resolution_clock::now();
        glfwPollEvents();

        auto &IO = ImGui::GetIO();

        if (!IO.WantCaptureMouse)
        {
            if (IO.MouseDown[ImGuiMouseButton_Right])
            {
                if (rightDragging)
                {
                    ImVec2 mouseDelta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
                    ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);

                    float sensitivity = 0.005f;
                    float yaw = -mouseDelta.x * sensitivity;
                    float pitch = -mouseDelta.y * sensitivity;

                    // 1. Get offset from target to camera
                    glm::vec3 offset = cameraPos - target;

                    // 2. Calculate camera right and up axes
                    glm::vec3 direction = glm::normalize(offset);
                    glm::vec3 right = glm::normalize(glm::cross(up, direction));

                    // 3. Apply pitch (rotate around right axis)
                    glm::mat4 pitchMat = glm::rotate(glm::mat4(1.0f), pitch, right);
                    offset = glm::vec3(pitchMat * glm::vec4(offset, 1.0f));

                    // 4. Apply yaw (rotate around global up)
                    glm::mat4 yawMat = glm::rotate(glm::mat4(1.0f), yaw, up);
                    offset = glm::vec3(yawMat * glm::vec4(offset, 1.0f));

                    // 5. New camera position
                    cameraPos = target + offset;

                    // 6. Update view matrix
                    view = glm::lookAt(cameraPos, target, up);
                    glUniformMatrix4fv(glGetUniformLocation(voxelShader, "view"), 1, GL_FALSE, glm::value_ptr(view));
                }
                else
                {
                    rightDragging = true;
                }
            }
            else if (IO.MouseReleased[ImGuiMouseButton_Right])
            {
                rightDragging = false;
            }

            if (IO.MouseWheel != 0)
            {
                cameraDistance = glm::length(cameraPos - target);
                cameraDistance -= IO.MouseWheel * 1.0f; // Change zoom speed here
                cameraDistance = std::clamp(cameraDistance, 5.0f, 100.0f);
                cameraPos = glm::normalize(cameraPos - target) * cameraDistance + target; // Recalculate camera position
                view = glm::lookAt(cameraPos, target, up);
                // Update the view matrix
                glUniformMatrix4fv(glGetUniformLocation(voxelShader, "view"), 1, GL_FALSE, glm::value_ptr(view));
            }

            if (IO.MouseDown[ImGuiMouseButton_Middle])
            {
                if (wheelDragging)
                {
                    ImVec2 mouseDelta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Middle);
                    ImGui::ResetMouseDragDelta(ImGuiMouseButton_Middle);

                    float sensitivity = 0.01f;

                    glm::vec3 forward = glm::normalize(target - cameraPos);
                    glm::vec3 right = glm::normalize(glm::cross(forward, up));
                    glm::vec3 camUp = glm::normalize(glm::cross(right, forward));

                    glm::vec3 panRight = right * -mouseDelta.x * sensitivity;
                    glm::vec3 panUp = camUp * mouseDelta.y * sensitivity;

                    glm::vec3 panMovement = panRight + panUp;

                    cameraPos += panMovement;
                    target += panMovement;

                    view = glm::lookAt(cameraPos, target, up);
                    glUniformMatrix4fv(glGetUniformLocation(voxelShader, "view"), 1, GL_FALSE, glm::value_ptr(view));
                }
                else
                {
                    prevCameraPos = cameraPos;
                    wheelDragging = true;
                }
            }
            else if (IO.MouseReleased[ImGuiMouseButton_Middle])
            {
                wheelDragging = false;
            }
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // UI
        ImGui::Begin("LED Cube Controls");
        if (ImGui::Button("Add Frame"))
            frames.emplace_back();
        if (ImGui::Button("Export .cbin"))
            exportCBIN(frames, delay, loop);
        if (ImGui::Button("Import .cbin"))
            importCBIN(frames, delay, loop);
        ImGui::SliderInt("Current Frame", &currentFrame, 0, frames.size() - 1);
        ImGui::InputInt("Delay (ms)", &delay);
        ImGui::Checkbox("Loop", &loop);
        ImGui::End();
        ImGui::Begin("Matrix Editor");

        ImGui::SliderInt("Z Layer", &editLayer, 0, CUBE_SIZE - 1);
        if (ImGui::Button("Clear Layer"))
        {
            for (int x = 0; x < CUBE_SIZE; ++x)
                for (int y = 0; y < CUBE_SIZE; ++y)
                    frames[currentFrame].voxels[x][y][editLayer] = 0;
        }

        // Render 8x8 grid
        for (int x = 0; x < CUBE_SIZE; ++x)
        {
            for (int y = 0; y < CUBE_SIZE; ++y)
            {
                uint8_t &cell = frames[currentFrame].voxels[CUBE_SIZE - x - 1][CUBE_SIZE - 1 - y][editLayer];
                ImGui::PushID(y * CUBE_SIZE + x);
                ImGui::PushStyleColor(ImGuiCol_Button, cell ? ImVec4(0.2f, 0.8f, 1.0f, 1.0f) : ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
                if (ImGui::Button(" ", ImVec2(30, 30)))
                {
                    cell = !cell;
                }
                ImGui::PopStyleColor();
                ImGui::PopID();
                ImGui::SameLine();
            }
            ImGui::NewLine();
        }

        ImGui::End();

        ImGui::Render();
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        drawCube3D(frames[currentFrame].voxels, voxelShader, cubeVAO, view, projection);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        if (elapsed < frame_time_ms) {
            std::this_thread::sleep_for(std::chrono::milliseconds(frame_time_ms - elapsed));
        }
    }
}

void drawCube3D(const uint8_t cube[8][8][8], GLuint shaderProgram, GLuint cubeVAO,
                const glm::mat4 &view, const glm::mat4 &projection)
{
    float spacing = 1.0f; // Leave gaps between voxels

    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(cubeVAO);

    for (int z = 0; z < 8; ++z)
    {
        for (int y = 0; y < 8; ++y)
        {
            for (int x = 0; x < 8; ++x)
            {
                glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z) * spacing);
                glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
                if (cube[z][y][x])
                {
                    glUniform3f(glGetUniformLocation(shaderProgram, "color"), 0.2f, 0.8f, 1.0f); // Cyan
                }
                else
                {
                    glUniform3f(glGetUniformLocation(shaderProgram, "color"), 0.1f, 0.1f, 0.1f); // Dark gray
                }
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
        }
    }

    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, -1.0f, -1.0f) * spacing);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniform3f(glGetUniformLocation(shaderProgram, "color"), 1.0f, 0.0f, 0.0f); // Red
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glBindVertexArray(0);
    glUseProgram(0);
}