#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

SDL_Window* displayWindow;
bool gameIsRunning = true;

ShaderProgram program1, program2;
std::vector<ShaderProgram> programVec;

glm::mat4 viewMatrix, projectionMatrix;

glm::mat4 modelMatrix1, modelMatrix2;
std::vector<glm::mat4> modelMatrixVec;

float ufo_x = 0;
float enemy_rotate = 0;

GLuint playerTextureID1;
GLuint playerTextureID2;
std::vector<GLuint> playerTextureIDVec;

GLuint LoadTexture(const char* filePath) {
    int w, h, n;
    unsigned char* image = stbi_load(filePath, &w, &h, &n, STBI_rgb_alpha);
    if (image == NULL) {
        std::cout << "Unable to load image. Make sure the path is correct\n";
        assert(false);
    }
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    stbi_image_free(image);
    return textureID;
}

void Initialize() {
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Scene!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(0, 0, 640, 480);

    program1.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");
    program2.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");

    viewMatrix = glm::mat4(1.0f);

    modelMatrix1 = glm::mat4(1.0f);
    modelMatrix2 = glm::mat4(1.0f);
    modelMatrixVec.push_back(modelMatrix1);
    modelMatrixVec.push_back(modelMatrix2);

    projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    program1.SetProjectionMatrix(projectionMatrix);
    program1.SetViewMatrix(viewMatrix);

    program2.SetProjectionMatrix(projectionMatrix);
    program2.SetViewMatrix(viewMatrix);

    programVec.push_back(program1);
    programVec.push_back(program2);

    glUseProgram(programVec[0].programID);
    glUseProgram(programVec[1].programID);

    glClearColor(1.0f, 0.6f, 0.6f, 1.0f);

    glEnable(GL_BLEND);
    // Good setting for transparency
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    playerTextureID1 = LoadTexture("ufoYellow.png");
    playerTextureID2 = LoadTexture("enemyBlack3.png");

    playerTextureIDVec.push_back(playerTextureID1);
    playerTextureIDVec.push_back(playerTextureID2);
}

void ProcessInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
            gameIsRunning = false;
        }
    }
}

float lastTicks = 0.0f;

void Update() {
    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float deltaTime = ticks - lastTicks;
    lastTicks = ticks;

    ufo_x += 1.0f * deltaTime;
    enemy_rotate += 90.0f * deltaTime;

    modelMatrixVec[0] = glm::mat4(1.0f);
    modelMatrixVec[0] = glm::translate(modelMatrixVec[0], glm::vec3(ufo_x, 2.0f, 0.0f));

    modelMatrixVec[1] = glm::mat4(1.0f);
    modelMatrixVec[1] = glm::rotate(modelMatrixVec[1], glm::radians(enemy_rotate), glm::vec3(0.0f, 0.0f, 1.0f));
}

void Render() {
    glClear(GL_COLOR_BUFFER_BIT);
    for (int i = 0; i < programVec.size(); i++) {
        programVec[i].SetModelMatrix(modelMatrixVec[i]);

        float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
        float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

        glVertexAttribPointer(programVec[i].positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(programVec[i].positionAttribute);
        glVertexAttribPointer(programVec[i].texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(programVec[i].texCoordAttribute);

        glBindTexture(GL_TEXTURE_2D, playerTextureIDVec[i]);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glDisableVertexAttribArray(programVec[i].positionAttribute);
        glDisableVertexAttribArray(programVec[i].texCoordAttribute);
    }
    SDL_GL_SwapWindow(displayWindow);
}

void Shutdown() {
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    Initialize();

    while (gameIsRunning) {
        ProcessInput();
        Update();
        Render();
    }

    Shutdown();
    return 0;
}