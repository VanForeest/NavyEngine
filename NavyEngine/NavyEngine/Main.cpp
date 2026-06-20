#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "Utilities.h"
#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "DirectLight.h"
#include "PointLight.h"

#include <iostream>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <cstdlib>
#include <string>
#include <windows.h>
#include <mmsystem.h>

#pragma comment(lib, "winmm.lib")

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void InitWindow();
void ConfigureWorkingDirectory();
void StartBackgroundMusic();
void StopBackgroundMusic();
void StartOceanWaves();
void StopOceanWaves();
void StartForestAmbience();
void StopForestAmbience();
void StopAllAudio();
void UpdateAudioLoops();
bool PlayLoopingAudio(const std::string& alias, const std::string& relativePath, const std::string& displayName);
bool IsAudioStopped(const std::string& alias);
void PrintMciError(MCIERROR errorCode, const std::string& displayName);

unsigned int LoadTexture(const char* path);
void DrawUIScreen(Shader& shader, unsigned int texture, glm::vec2 center, glm::vec2 size, int screenWidth, int screenHeight, float alpha = 1.0f);
void DrawUIScreenRegion(Shader& shader, unsigned int texture, glm::vec2 center, glm::vec2 size, glm::vec2 uvMin, glm::vec2 uvMax, int screenWidth, int screenHeight, float alpha = 1.0f);
bool IsMouseInside(glm::vec2 center, glm::vec2 size, double mouseX, double mouseY);

enum class AppScreen {
    Menu,
    Credits,
    Gameplay
};

// settings
GLFWwindow* window;
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f));
float lastX = 1280.0f / 2.0;
float lastY = 720.0 / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;
AppScreen currentScreen = AppScreen::Menu;
bool backgroundMusicActive = false;
bool oceanWavesActive = false;
bool forestAmbienceActive = false;

int main()
{
    ConfigureWorkingDirectory();
    InitWindow();
    StartBackgroundMusic();

    camera.MoveSpeed = 15.0f;

    // build and compile shaders
    // -------------------------
    Shader pbrShader("PBR.vs", "PBR.fs");
    Shader TexturedpbrShader("WindPBR.vs", "TexturedPBR.fs");
    Shader backgroundShader("SkyBox.vs", "SkyBox.fs");
    Shader OceanShader("Ocean.vs", "PBR.fs");
    Shader uiShader("UI.vs", "UI.fs");

    pbrShader.use();
    pbrShader.setInt("irradianceMap", 0);
    pbrShader.setInt("prefilterMap", 1);
    pbrShader.setInt("brdfLUT", 2);
    pbrShader.setVec3("albedo", 0.01, 0.05, 0.1);
    pbrShader.setFloat("roughness", 0.03);
    pbrShader.setFloat("metallic", 0.0);
    pbrShader.setFloat("ao", 1.0f);
    pbrShader.setVec3("FF0", glm::vec3(0.02f));

    backgroundShader.use();
    backgroundShader.setInt("SkyBoxMap", 0);

    TexturedpbrShader.use();
    TexturedpbrShader.setInt("texture_BaseColor1", 0);
    TexturedpbrShader.setInt("texture_Normal1", 1);
    TexturedpbrShader.setInt("texture_ORM1", 2);
    TexturedpbrShader.setInt("irradianceMap", 3);
    TexturedpbrShader.setInt("prefilterMap", 4);
    TexturedpbrShader.setInt("brdfLUT", 5);

    OceanShader.use();
    OceanShader.setInt("irradianceMap", 0);
    OceanShader.setInt("prefilterMap", 1);
    OceanShader.setInt("brdfLUT", 2);
    OceanShader.setVec3("albedo", 0.02, 0.08, 0.12);
    OceanShader.setFloat("roughness", 0.03);
    OceanShader.setFloat("metallic", 0.0);
    OceanShader.setFloat("ao", 1.0f);
    OceanShader.setVec3("FF0", glm::vec3(0.02f));

    OceanShader.setFloat("Amplitude", 1.0);
    OceanShader.setFloat("wavelenght", 4.0);
    OceanShader.setFloat("speed", 2.0);

    uiShader.use();
    uiShader.setInt("screenTexture", 0);

    unsigned int menuTexture = LoadTexture("UI/menu_bg_day.png");
    unsigned int creditsTexture = LoadTexture("UI/credits_info.jpeg");
    unsigned int logoTexture = LoadTexture("UI/logo.png");
    unsigned int enterIdleTexture = LoadTexture("UI/enter_N.png");
    unsigned int enterHoverTexture = LoadTexture("UI/enter_Hover.png");
    unsigned int creditsIdleTexture = LoadTexture("UI/credit_N.png");
    unsigned int creditsHoverTexture = LoadTexture("UI/credit_Hover.png");

    //3D models
    Model masterreyModel("Modelos3D/masterrey.obj");
    Model OceanPlane("Modelos3D/OceanPlane3.obj");
    Model IslandModel("Modelos3D/Island_Final.glb");

    //Lights
    // El vector Direction original
    DirectLight Dirlight(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 3.0f));

    PointLight light1(glm::vec3(100.0f), glm::vec3(-10.0f, 10.0f, 10.0f), 10.0f);
    PointLight light2(glm::vec3(150.0f), glm::vec3(10.0f, 10.0f, 10.0f), 15.0f);
    PointLight light3(glm::vec3(200.0f), glm::vec3(-10.0f, -10.0f, 10.0f), 20.0f);
    PointLight light4(glm::vec3(250.0f), glm::vec3(10.0f, -10.0f, 10.0f), 25.0f);

    std::vector<PointLight> Lights{ light1, light2, light3, light4 };

    //Get IBL maps from HDRi file
    const std::string HDRfile = "HDR/LowSky.hdr";
    GLuint envCubemap, irradianceMap, prefilterMap, brdfLUTTexture;
    GenIBLmapsFromHDR(HDRfile, envCubemap, irradianceMap, prefilterMap, brdfLUTTexture);

    //Valores estaticos
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 150.0f);
    pbrShader.use();
    pbrShader.setMat4("projection", projection);
    backgroundShader.use();
    backgroundShader.setMat4("projection", projection);
    TexturedpbrShader.use();
    TexturedpbrShader.setMat4("projection", projection);
    OceanShader.use();
    OceanShader.setMat4("projection", projection);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // Regresamos al viewport a su resolucion original
    int scrWidth, scrHeight;
    glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
    glViewport(0, 0, scrWidth, scrHeight);

    bool wasMousePressed = false;

    //RENDER LOOP
    while (!glfwWindowShouldClose(window)) {

        // Logica del tiempo per-frame 
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        UpdateAudioLoops();

        //input
        processInput(window);
        glfwGetFramebufferSize(window, &scrWidth, &scrHeight);

        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        float menuYOffset = scrHeight * 0.23f;
        glm::vec2 enterCenter(scrWidth * 0.5f, scrHeight * 0.405f + menuYOffset);
        glm::vec2 creditsCenter(scrWidth * 0.5f, scrHeight * 0.585f + menuYOffset);
        float buttonWidth = std::min(scrWidth * 0.34f, 430.0f);
        glm::vec2 buttonSize(buttonWidth, buttonWidth * 0.32f);
        glm::vec2 buttonHitSize(buttonSize.x, buttonSize.y * 0.86f);
        bool enterHovered = IsMouseInside(enterCenter, buttonHitSize, mouseX, mouseY);
        bool creditsHovered = IsMouseInside(creditsCenter, buttonHitSize, mouseX, mouseY);
        bool mousePressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
        bool mouseClicked = mousePressed && !wasMousePressed;

        if (currentScreen == AppScreen::Menu && mouseClicked) {
            if (enterHovered) {
                currentScreen = AppScreen::Gameplay;
                firstMouse = true;
                StartOceanWaves();
                StartForestAmbience();
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
            else if (creditsHovered) {
                currentScreen = AppScreen::Credits;
            }
        }
        else if (currentScreen == AppScreen::Credits && (mouseClicked || glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)) {
            currentScreen = AppScreen::Menu;
        }
        wasMousePressed = mousePressed;

        // 2. Render de la escena de manera normal
        glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
        glViewport(0, 0, scrWidth, scrHeight);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (currentScreen != AppScreen::Gameplay) {
            unsigned int backgroundTexture = currentScreen == AppScreen::Credits ? creditsTexture : menuTexture;
            float aspect = scrWidth / static_cast<float>(scrHeight);

            glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glm::vec2 backgroundSize = aspect >= 1.0f
                ? glm::vec2(scrWidth, scrWidth)
                : glm::vec2(scrHeight, scrHeight);
            glm::vec2 backgroundCenter(scrWidth * 0.5f, scrHeight * 0.5f);
            if (currentScreen == AppScreen::Menu)
                backgroundCenter.y += menuYOffset;

            DrawUIScreen(uiShader, backgroundTexture, backgroundCenter, backgroundSize, scrWidth, scrHeight);

            if (currentScreen == AppScreen::Menu) {
                float logoSize = std::min(scrHeight * 0.39f, 280.0f);
                glm::vec2 enterUvMin = enterHovered ? glm::vec2(0.108f, 0.173f) : glm::vec2(0.158f, 0.337f);
                glm::vec2 enterUvMax = enterHovered ? glm::vec2(0.892f, 0.921f) : glm::vec2(0.846f, 0.850f);
                glm::vec2 creditsUvMin = creditsHovered ? glm::vec2(0.108f, 0.097f) : glm::vec2(0.159f, 0.114f);
                glm::vec2 creditsUvMax = creditsHovered ? glm::vec2(0.889f, 0.817f) : glm::vec2(0.847f, 0.735f);

                DrawUIScreen(uiShader, logoTexture, glm::vec2(scrWidth * 0.5f, scrHeight * 0.22f), glm::vec2(logoSize, logoSize), scrWidth, scrHeight);
                DrawUIScreenRegion(uiShader, enterHovered ? enterHoverTexture : enterIdleTexture, enterCenter, buttonSize, enterUvMin, enterUvMax, scrWidth, scrHeight);
                DrawUIScreenRegion(uiShader, creditsHovered ? creditsHoverTexture : creditsIdleTexture, creditsCenter, buttonSize, creditsUvMin, creditsUvMax, scrWidth, scrHeight);
            }

            glDisable(GL_BLEND);
            glEnable(GL_DEPTH_TEST);
            glfwSwapBuffers(window);
            glfwPollEvents();
            continue;
        }

        // 1. Calculate camera and view matrix
        glm::mat4 view = camera.GetViewMatrix();
        glm::vec3 camPos = camera.Position;

        // 2. Render 3D Environment (Always rendering behind the UI)
        glm::mat4 model = glm::mat4(1.0f);

        // SHADER DEL PBR CON TEXTURA
        TexturedpbrShader.use();
        TexturedpbrShader.setMat4("view", view);
        TexturedpbrShader.setVec3("camPos", camPos);
        TexturedpbrShader.setFloat("time", currentFrame);

        Dirlight.SendDataToShader(TexturedpbrShader, "dirlight");

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);

        // Render masterrey
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, -12.0f));
        model = glm::scale(model, glm::vec3(0.5f));
        TexturedpbrShader.setMat4("model", model);
        TexturedpbrShader.setMat3("NormalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
        masterreyModel.Draw(TexturedpbrShader);

        // Render Island
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 60.0f));
        model = glm::scale(model, glm::vec3(1.0f));
        TexturedpbrShader.setMat4("model", model);
        TexturedpbrShader.setMat3("NormalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
        IslandModel.Draw(TexturedpbrShader);

        // USAMOS EL OCEAN SHADER
        OceanShader.use();
        OceanShader.setMat4("view", view);
        OceanShader.setVec3("camPos", camPos);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);

        // Render Ocean Plane
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        OceanShader.setFloat("time", currentFrame);
        OceanShader.setMat4("model", model);
        OceanShader.setMat3("NormalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
        OceanPlane.Draw(OceanShader);

        // Render skybox
        backgroundShader.use();
        backgroundShader.setMat4("view", view);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
        renderCube();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    StopAllAudio();
    return 0;
}

void processInput(GLFWwindow* window)
{
    if (currentScreen == AppScreen::Credits)
        return;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (currentScreen != AppScreen::Gameplay)
        return;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.HandleKeyboard(FRONT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.HandleKeyboard(BACK, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.HandleKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.HandleKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.HandleKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.HandleKeyboard(DOWN, deltaTime);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    if (currentScreen != AppScreen::Gameplay)
        return;

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.HandleMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (currentScreen != AppScreen::Gameplay)
        return;

    camera.HandleMouseScroll(static_cast<float>(yoffset));
}

void InitWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "NavyEngine", NULL, NULL);
    if (window == NULL) {
        std::cout << "Error al crear GLFW window" << std::endl;
        glfwTerminate();
        std::exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    if (glewInit() != GLEW_OK) {
        std::cout << "Error al iniciar Glew" << std::endl;
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

void ConfigureWorkingDirectory()
{
    namespace fs = std::filesystem;

    auto hasProjectAssets = [](const fs::path& path) {
        return fs::exists(path / "PBR.vs") &&
            fs::exists(path / "UI" / "menu_bg_day.png") &&
            fs::exists(path / "Modelos3D" / "masterrey.obj") &&
            fs::exists(path / "HDR" / "LowSky.hdr");
    };

    fs::path current = fs::current_path();
    std::vector<fs::path> candidates;

    candidates.push_back(current);
    candidates.push_back(current / "NavyEngine");

    fs::path probe = current;
    for (int i = 0; i < 6 && !probe.empty(); ++i) {
        candidates.push_back(probe / "NavyEngine");
        candidates.push_back(probe / "NavyEngine" / "NavyEngine");
        candidates.push_back(probe / "NavyEngine-feature-tree-animations" / "NavyEngine" / "NavyEngine");
        probe = probe.parent_path();
    }

    for (const fs::path& candidate : candidates) {
        if (hasProjectAssets(candidate)) {
            fs::current_path(candidate);
            std::cout << "Directorio de recursos: " << candidate.string() << std::endl;
            return;
        }
    }

    std::cout << "No se encontro la carpeta de recursos. Directorio actual: " << current.string() << std::endl;
}

void StartBackgroundMusic()
{
    backgroundMusicActive = PlayLoopingAudio("backgroundMusic", "Audio/ocean_theme.wav", "musica de fondo");
}

void StopBackgroundMusic()
{
    backgroundMusicActive = false;
    mciSendStringA("stop backgroundMusic", NULL, 0, NULL);
    mciSendStringA("close backgroundMusic", NULL, 0, NULL);
}

void StartOceanWaves()
{
    oceanWavesActive = PlayLoopingAudio("oceanWaves", "Audio/ocean_waves.wav", "sonido del mar");
}

void StopOceanWaves()
{
    oceanWavesActive = false;
    mciSendStringA("stop oceanWaves", NULL, 0, NULL);
    mciSendStringA("close oceanWaves", NULL, 0, NULL);
}

void StartForestAmbience()
{
    forestAmbienceActive = PlayLoopingAudio("forestAmbience", "Audio/forest_wind_birds.wav", "ambiente de viento y aves");
}

void StopForestAmbience()
{
    forestAmbienceActive = false;
    mciSendStringA("stop forestAmbience", NULL, 0, NULL);
    mciSendStringA("close forestAmbience", NULL, 0, NULL);
}

void StopAllAudio()
{
    StopForestAmbience();
    StopOceanWaves();
    StopBackgroundMusic();
}

void UpdateAudioLoops()
{
    if (backgroundMusicActive && IsAudioStopped("backgroundMusic"))
        mciSendStringA("play backgroundMusic from 0", NULL, 0, NULL);

    if (oceanWavesActive && IsAudioStopped("oceanWaves"))
        mciSendStringA("play oceanWaves from 0", NULL, 0, NULL);

    if (forestAmbienceActive && IsAudioStopped("forestAmbience"))
        mciSendStringA("play forestAmbience from 0", NULL, 0, NULL);
}

bool PlayLoopingAudio(const std::string& alias, const std::string& relativePath, const std::string& displayName)
{
    namespace fs = std::filesystem;

    if (!fs::exists(relativePath)) {
        std::cout << "No se encontro " << displayName << ": " << relativePath << std::endl;
        return false;
    }

    const std::string closeCommand = "close " + alias;
    mciSendStringA(closeCommand.c_str(), NULL, 0, NULL);

    const std::string absolutePath = fs::absolute(relativePath).string();
    const std::string openCommand = "open \"" + absolutePath + "\" alias " + alias;
    MCIERROR errorCode = mciSendStringA(openCommand.c_str(), NULL, 0, NULL);
    if (errorCode != 0) {
        PrintMciError(errorCode, displayName);
        return false;
    }

    const std::string playCommand = "play " + alias + " from 0";
    errorCode = mciSendStringA(playCommand.c_str(), NULL, 0, NULL);
    if (errorCode != 0) {
        PrintMciError(errorCode, displayName);
        mciSendStringA(closeCommand.c_str(), NULL, 0, NULL);
        return false;
    }

    return true;
}

bool IsAudioStopped(const std::string& alias)
{
    char status[32] = {};
    const std::string statusCommand = "status " + alias + " mode";
    MCIERROR errorCode = mciSendStringA(statusCommand.c_str(), status, sizeof(status), NULL);
    return errorCode == 0 && std::string(status) == "stopped";
}

void PrintMciError(MCIERROR errorCode, const std::string& displayName)
{
    char errorText[256] = {};
    if (mciGetErrorStringA(errorCode, errorText, sizeof(errorText))) {
        std::cout << "No se pudo reproducir " << displayName << ": " << errorText << std::endl;
    }
    else {
        std::cout << "No se pudo reproducir " << displayName << ". Codigo MCI: " << errorCode << std::endl;
    }
}

unsigned int LoadTexture(const char* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format = GL_RGB;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else {
        std::cout << "No se pudo cargar la textura: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

void DrawUIScreen(Shader& shader, unsigned int texture, glm::vec2 center, glm::vec2 size, int screenWidth, int screenHeight, float alpha)
{
    DrawUIScreenRegion(shader, texture, center, size, glm::vec2(0.0f), glm::vec2(1.0f), screenWidth, screenHeight, alpha);
}

void DrawUIScreenRegion(Shader& shader, unsigned int texture, glm::vec2 center, glm::vec2 size, glm::vec2 uvMin, glm::vec2 uvMax, int screenWidth, int screenHeight, float alpha)
{
    glm::vec2 ndcCenter(
        (center.x / static_cast<float>(screenWidth)) * 2.0f - 1.0f,
        1.0f - (center.y / static_cast<float>(screenHeight)) * 2.0f
    );
    glm::vec2 ndcScale(
        size.x / static_cast<float>(screenWidth),
        size.y / static_cast<float>(screenHeight)
    );

    shader.use();
    shader.setVec2("offset", ndcCenter);
    shader.setVec2("scale", ndcScale);
    shader.setVec2("uvMin", uvMin);
    shader.setVec2("uvMax", uvMax);
    shader.setFloat("alpha", alpha);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    renderQuad();
}

bool IsMouseInside(glm::vec2 center, glm::vec2 size, double mouseX, double mouseY)
{
    return mouseX >= center.x - size.x * 0.5f &&
        mouseX <= center.x + size.x * 0.5f &&
        mouseY >= center.y - size.y * 0.5f &&
        mouseY <= center.y + size.y * 0.5f;
}


