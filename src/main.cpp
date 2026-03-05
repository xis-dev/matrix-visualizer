#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "includes/Shader.h"
#include "includes/Vertex.h"
#include "includes/Camera.h"


#include "glm/mat4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>


#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"






#include <iostream>
#include <cmath>
#include <vector>

#define CELLS 5

namespace screen
{
    constexpr int width = 1600;
    constexpr int height = 900;

    float lastXPos{ width / 2.0f };
    float lastYPos{ height / 2.0f };

    float xOffset{};
    float yOffset{};
    float scrollOffset{};
    bool shiftLock{ true };
    bool firstMouseInput{true};
}

void imguiInit(GLFWwindow* window);
void imguiUse();
void imguiRender();

void processInput(GLFWwindow* win, float dt);
void framebufferSizeCallback(GLFWwindow* window, const int width, const int height);
void cursorPosCallback(GLFWwindow* window, double xPos, double yPos);
void scrollCallback(GLFWwindow* window, double xOff, double yOff);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void render(GLFWwindow* window);
void init();


float deltaTime{};
float lastFrameTime{};

Camera cam{ glm::vec3(0.0f, 0.0f, -3.0f), glm::vec3(0.0f, 0.0f, 1.0f), 45.0f, (float)screen::width/screen::height, 10.0f};



glm::mat4 modelMat = glm::mat4{ 1.0f };
glm::mat4 imguiDisplayMat{};
bool freeMat{};

unsigned vao, vbo, ibo;
unsigned matrixUniforms;
GLuint length;


std::vector<float> verts{};
std::vector<unsigned> inds{};


glm::vec3 currentRotAxis{ 0.0f, 1.0f, 0.0f };
float currentAngle{ 0.0f };

glm::vec3 currentSheerAxis{ 1.0f, 0.0f, 0.0f };
float sheerIntensity;

glm::vec3 scale{ 1.0f };

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(screen::width, screen::height
        , "OpenGL Scene", nullptr, nullptr);
    if (!window)
    {
        std::cout << "Failed to initialize window.\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // ReSharper disable once CppCStyleCast
    if (!gladLoadGLLoader((GLADloadproc)(glfwGetProcAddress)))
    {
        std::cout << "Failed to initialize glad.\n";
        glfwTerminate();
        return -1;
    }


    glViewport(0, 0, screen::width, screen::height);


    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    imguiInit(window);

    Shader grid{ "assets/shaders/grid.vert", "assets/shaders/grid.frag" };


    grid.use();

    init();
    glDisable(GL_CULL_FACE);
    grid.setUniformBlock("Matrices", 0);

    glBindBuffer(GL_UNIFORM_BUFFER, matrixUniforms);


    while (!glfwWindowShouldClose(window))
    {
        const float currentTime = glfwGetTime();
        deltaTime = currentTime - lastFrameTime;
        lastFrameTime = currentTime;

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glEnable(GL_DEPTH_TEST);

        glClearColor(0.f, 0.f, 0.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glfwPollEvents();

        processInput(window, deltaTime);
        imguiUse();
       


        grid.use();
        grid.setUniformMat4("u_Model", modelMat);
        glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(cam.getProjectionMatrix()));
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(cam.getViewMatrix()));
        glDisable(GL_DEPTH_TEST);
        glBindVertexArray(vao);

        glDrawElements(GL_LINES, length, GL_UNSIGNED_INT, NULL);

        glBindVertexArray(0);

        if (!freeMat)
        {
            modelMat = glm::mat4{ 1.0f };

        }
        imguiRender();




        glfwSwapBuffers(window);
    }

    // shutdown imgui and terminate glfw
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}



void imguiInit(GLFWwindow* window)
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init();
}

void imguiUse()
{
    // setup imgui for a new frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    currentRotAxis = normalize(currentRotAxis);
    ImGui::Begin("Matrices");
    ImGui::DragFloat3("Rotation Axis", glm::value_ptr(currentRotAxis));
    ImGui::DragFloat("Rotation Angle", &currentAngle);

    ImGui::DragFloat3("Sheer Axis", glm::value_ptr(currentSheerAxis));
    ImGui::DragFloat("Sheer Intensity", &sheerIntensity);

    ImGui::DragFloat3("Scale", glm::value_ptr(scale));

    if (ImGui::Button("Free Matrices", ImVec2(100.0f, 50.0f)))
    {
        freeMat = !freeMat;
    }

    if (!freeMat)
    {
        ImGui::DragFloat3("Col 1", &imguiDisplayMat[0][0]);
        ImGui::DragFloat3("Col 2", &imguiDisplayMat[1][0]);
        ImGui::DragFloat3("Col 3", &imguiDisplayMat[2][0]);
        modelMat = glm::mat4{ 1.0f };
        modelMat = glm::rotate(modelMat, currentAngle, currentRotAxis);
        modelMat = glm::scale(modelMat, scale);
        imguiDisplayMat = modelMat;
    }
    else
    {

        ImGui::DragFloat3("Col 0", &modelMat[0][0], 0.1f);
        ImGui::DragFloat3("Col 1", &modelMat[1][0], 0.1f);
        ImGui::DragFloat3("Col 2", &modelMat[2][0], 0.1f);

    }


    ImGui::End();
}

void imguiRender()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void processInput(GLFWwindow* window, float dt)
{
    ImGuiIO& io = ImGui::GetIO();
    if (!io.WantCaptureMouse)
    {
        cam.processKeyboardInput(window, dt);
    }

    if (!io.WantCaptureKeyboard)
    {
        if (!screen::shiftLock) return;
        cam.processMouseInput(window, screen::xOffset, screen::yOffset, screen::scrollOffset);
    }
    screen::xOffset = 0.0f;
    screen::yOffset = 0.0f;
    screen::scrollOffset = 0.0f;
}

void cursorPosCallback(GLFWwindow* window, double xPos, double yPos)
{
    if (screen::firstMouseInput)
    {
        screen::lastXPos = xPos;
        screen::lastYPos = yPos;
        screen::firstMouseInput = false;
    }

    screen::xOffset = xPos - screen::lastXPos;
    screen::yOffset = screen::lastYPos - yPos;

    screen::lastXPos = xPos;
    screen::lastYPos = yPos;
}

void scrollCallback(GLFWwindow* window, double xOff, double yOff)
{
    screen::scrollOffset = yOff;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    using namespace screen;
    if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_PRESS)
    {
        shiftLock = !shiftLock;
        if (shiftLock)
        {
            firstMouseInput = true;
            glfwSetCursorPos(window, static_cast<float>(width) / 2.0f, static_cast<float>(height) / 2.0f);
        }
        glfwSetInputMode(window, GLFW_CURSOR, shiftLock ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }
}

void framebufferSizeCallback(GLFWwindow* window, const int width, const int height)
{
    glViewport(0, 0, width, height);
}

void init()
{
    // Toggling default gl settings
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    // Initialize frame buffer and it's attachments

    int p = CELLS + 1;


    // par to z
    for (int x = 0; x <= CELLS; ++x)
    {
        for (int y = 0; y <= CELLS; ++y)
        {
            for (int z = 0; z <= CELLS; ++z)
            {
                float nx = (float)(x) / CELLS;
                float ny = (float)(y) / CELLS;
                float nz = (float)(z) / CELLS;

                verts.push_back(nx * 2.0f - 1.0f);
                verts.push_back(ny * 2.0f - 1.0f);
                verts.push_back(nz * 2.0f - 1.0f);
            }
            
        }
    }

    for (int x = 0; x <= CELLS; ++x)
    {
        for (int y = 0; y <= CELLS; ++y)
        {
            for (int z = 0; z <= CELLS; ++z)
            {
                int base = x * (p * p) + y * p + z;

                if (x < CELLS)
                {
                    inds.push_back(base);
                    inds.push_back(base + p * p);
                }

                if (y < CELLS)
                {
                    inds.push_back(base);
                    inds.push_back(base + p);
                }

                if (z < CELLS)
                {
                    inds.push_back(base);
                    inds.push_back(base + 1);
                }
            }
        }
    }

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, inds.size() * sizeof(unsigned), inds.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    length = (GLuint)inds.size();

    glGenBuffers(1, &matrixUniforms);
    glBindBuffer(GL_UNIFORM_BUFFER, matrixUniforms);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(cam.getProjectionMatrix()));
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(cam.getViewMatrix()));
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, matrixUniforms);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);


}

void render(GLFWwindow* window)
{
    
}