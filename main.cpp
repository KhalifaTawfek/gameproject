#include "Graphics/window.h"
#include "Camera/camera.h"
#include "Shaders/shader.h"
#include "Model Loading/mesh.h"
#include "Model Loading/texture.h"
#include "Model Loading/meshLoaderObj.h"
#include <vector>
#include <iostream>
#include <ctime>
#include <algorithm>
#include <string>
#include <deque>
#include <cmath>
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_opengl3.h"

#include <C:\Users\a7lat\Downloads\TESTGROUND\GameEngine2025\GameEngine\Dependencies\glm/glm.hpp>
#include <C:\Users\a7lat\Downloads\TESTGROUND\GameEngine2025\GameEngine\Dependencies\glm/gtc/matrix_transform.hpp>
#include <C:\Users\a7lat\Downloads\TESTGROUND\GameEngine2025\GameEngine\Dependencies\glm/gtc/type_ptr.hpp>

GLuint loadCubemap(std::vector<std::string> faces);

void processKeyboardInput();
void processMouseInput();
void updateGameLogic();

float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool firstMouse = true;
float lastX = 400.0f, lastY = 400.0f;

Window window("Sura's Journey", 1600, 1600);

Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));

glm::vec3 lightColor(0.4f, 0.4f, 0.7f);
glm::vec3 lightPos(-180.0f, 100.0f, -200.0f);

struct Fireball {
    glm::vec3 position;
    glm::vec3 direction;
    float lifeTime;
};

std::vector<Fireball> fireballs;

glm::vec3 bossPos(650.0f, -4.0f, 0.0f);
bool bossActivated = false;
float bossFlyHeight = 30.0f;
glm::vec3 iceBossPos(400.0f, -4.0f, 400.0f);
bool iceBossActivated = false;
bool iceBossDead = false;

int iceBossHP = 5;
int iceBossMaxHP = 5;

bool portalActive = false;
glm::vec3 portalPos(0.0f);
glm::vec3 iceIslandSpawn(400.0f, -4.0f, 400.0f);


float bossMoveTimer = 0.0f;
bool bossMoveLeft = true;

float bossShootTimer = 0.0f;
int bossHP = 5;
int bossMaxHP = 5;
bool bossDead = false;

struct Bullet {
    glm::vec3 position{ 0.0f };
    glm::vec3 direction{ 0.0f };
    float lifeTime{ 0.0f };
};

std::vector<Bullet> bullets;

struct Zombie {
    glm::vec3 pos;
    int type;
    int hp = 2;
    bool isDead = false;
};
float iceBossMoveTimer = 0.0f;
bool iceBossMoveLeft = true;

struct GameMessage {
    std::string text;
    float timer;
};
std::deque<GameMessage> messageLog;

void AddMessage(std::string text) {
    messageLog.push_back({ text, 3.0f });
    if (messageLog.size() > 5) messageLog.pop_front();
}

int playerHP = 100;
int maxPlayerHP = 100;
bool hasGun = false;
bool hasKey = false;
bool keyPickedUp = false;
bool eKeyPressed = false;
bool fKeyPressed = false;
bool fiveKeyPressed = false;
bool isRiding = false;
bool freeCam = false;
bool isInsideHouse = false;

bool showInventory = false;
int currentItem = 0;
float mapHalfSize = 300.0f;

glm::vec3 horsePos(0.0f, -4.0f, -120.0f);
glm::vec3 playerWorldPos(0.0f, 0.0f, 0.0f);
glm::vec3 housePos(-80.0f, -3.0f, -60.0f);
glm::vec3 safePos(-80.0f, -3.0f, -60.0f);
glm::vec3 keyPos(0.0f, 0.0f, 0.0f);
glm::vec3 hospitalPos(0.0f, -4.0f, 0.0f);
glm::vec3 stablePos(0.0f, -4.0f, -120.0f);
glm::vec3 volcanoPos(-400.0f, -4.0f, -400.0f);
glm::vec3 lavaMapPos(600.0f, -4.0f, 0.0f);

float volcanoScale = 0.1;

std::vector<glm::vec3> treePositions;
std::vector<glm::vec3> lavaPositions; 
std::vector<Zombie> zombies;
std::string interactionText = "";
bool showInteraction = false;
float interactionFlashTimer = 0.0f;


float damageFlashTimer = 0.0f;
glm::mat4 BuildFPSItemModel(
    const glm::vec3& camPos,
    const glm::vec3& camFront,
    const glm::vec3& camRight,
    const glm::vec3& camUp,
    const glm::vec3& offset,
    const glm::vec3& scale
) {
    glm::vec3 pos =
        camPos +
        camFront * offset.z +
        camRight * offset.x +
        camUp * offset.y;

    glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);

    glm::mat4 rot = glm::mat4(1.0f);
    rot[0] = glm::vec4(camRight, 0.0f);
    rot[1] = glm::vec4(camUp, 0.0f);
    rot[2] = glm::vec4(-camFront, 0.0f);

    model *= rot;
    model = glm::scale(model, scale);

    return model;
}

enum Mission {
    FIND_KEY,
    OPEN_SAFE,
    FREE_HORSE,
    CROSS_LAVA,
    FIGHT_BOSS
};
Mission currentMission = FIND_KEY;


glm::vec2 WorldToScreen(glm::vec3 pos, glm::mat4 view, glm::mat4 proj, float width, float height) {
    glm::vec4 clipSpace = proj * view * glm::vec4(pos, 1.0f);
    if (clipSpace.w <= 0.0f) return glm::vec2(-1.0f);
    glm::vec3 ndc = glm::vec3(clipSpace) / clipSpace.w;
    return glm::vec2(((ndc.x + 1.0f) / 2.0f) * width, ((1.0f - ndc.y) / 2.0f) * height);
}


float getTerrainHeight(float x, float z) {
    float groundY = -4.0f; 

    float distToVolcano = glm::distance(glm::vec2(x, z), glm::vec2(volcanoPos.x, volcanoPos.z));


    float baseRadius = 12.0f;
    float rimRadius = 3.0f;
    float peakHeight = 5.0f;
    float holeDepth = -12.0f;

    if (distToVolcano < baseRadius) {
        if (distToVolcano > rimRadius) {
            float t = (baseRadius - distToVolcano) / (baseRadius - rimRadius);
            return groundY + t * peakHeight;
        }
        else {
            float t = distToVolcano / rimRadius;
            return holeDepth + t * ((groundY + peakHeight) - holeDepth);
        }
    }

    return groundY;
}

bool isOutsideMap(glm::vec3 pos) {
    if (pos.x < -mapHalfSize || pos.x > mapHalfSize ||
        pos.z < -mapHalfSize || pos.z > mapHalfSize)
        return true;

    return false;
}
int main()
{
    srand(static_cast<unsigned int>(time(0)));
    float keyX = -140.0f - static_cast<float>(rand() % 20);
    float keyZ = -110.0f - static_cast<float>(rand() % 30);
    keyPos = glm::vec3(keyX, -1.0f, keyZ);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    glfwSetInputMode(window.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window.getWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 330");

    Shader shader("Shaders/vertex_shader.glsl", "Shaders/fragment_shader.glsl");
    Shader sunShader("Shaders/sun_vertex_shader.glsl", "Shaders/sun_fragment_shader.glsl");
    Shader skyboxShader("Shaders/skybox_vertex.glsl", "Shaders/skybox_fragment.glsl");

    float skyboxVertices[] = {
        -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f
    };

    GLuint skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    std::vector<std::string> faces{
        "Resources/Textures/night1.bmp", "Resources/Textures/night2.bmp",
        "Resources/Textures/night3.bmp", "Resources/Textures/night4.bmp",
        "Resources/Textures/night5.bmp", "Resources/Textures/night6.bmp"
    };
    GLuint cubemapTexture = loadCubemap(faces);

    GLuint texWood = loadBMP("Resources/Textures/woodplanks.bmp");
    GLuint texWood2 = loadBMP("Resources/Textures/woodplanks.bmp");
    GLuint texOrange = loadBMP("Resources/Textures/orange.bmp");
    GLuint texLava = loadBMP("Resources/Textures/lava.bmp");
    GLuint texMan = loadBMP("Resources/Textures/man.bmp");
    GLuint texKey = loadBMP("Resources/Textures/key.bmp");
    GLuint texZ1 = loadBMP("Resources/Textures/zombieskin1.bmp");
    GLuint texZ2 = loadBMP("Resources/Textures/zombieskin2.bmp");
    GLuint texZ3 = loadBMP("Resources/Textures/zombieskin3.bmp");
    GLuint texFlash = loadBMP("Resources/Textures/flash.bmp");
    GLuint texPortal = loadBMP("Resources/Textures/portal.bmp");
    GLuint texIce = loadBMP("Resources/Textures/volcanoice.bmp");

    auto makeTex = [](GLuint id) {
        std::vector<Texture> v; Texture t = {}; t.id = id; t.type = "texture_diffuse"; v.push_back(t); return v;
        };

    std::vector<Texture> woodTextures = makeTex(texWood);
    std::vector<Texture> stableTextures = makeTex(texWood2);
    std::vector<Texture> orangeTextures = makeTex(texOrange);
    std::vector<Texture> lavaTextures = makeTex(texLava);
    std::vector<Texture> manTextures = makeTex(texMan);
    std::vector<Texture> keyTextures = makeTex(texKey);
    std::vector<Texture> z1Textures = makeTex(texZ1);
    std::vector<Texture> z2Textures = makeTex(texZ2);
    std::vector<Texture> z3Textures = makeTex(texZ3);
    std::vector<Texture> flashTextures = makeTex(texFlash);
    std::vector<Texture> portalTextures = makeTex(texPortal);
    std::vector<Texture> iceTextures = makeTex(texIce);
    std::vector<Texture> iceBossTextures = makeTex(texIce);


    MeshLoaderObj loader;
    Mesh sun = loader.loadObj("Resources/Models/sphere.obj");
    Mesh plane = loader.loadObj("Resources/Models/maptree.obj", orangeTextures);

    Mesh lavamap = loader.loadObj("Resources/Models/lavamap.obj", lavaTextures);
    Mesh lava = loader.loadObj("Resources/Models/lava.obj", orangeTextures);

    Mesh hospital = loader.loadObj("Resources/Models/hospital_room.obj", woodTextures);
    Mesh stable = loader.loadObj("Resources/Models/stable.obj", stableTextures);
    Mesh horse = loader.loadObj("Resources/Models/horse.obj", orangeTextures);
    Mesh volcano = loader.loadObj("Resources/Models/volcano.obj", lavaTextures);
    Mesh treeModel = loader.loadObj("Resources/Models/tree.obj", orangeTextures);
    Mesh house = loader.loadObj("Resources/Models/house.obj", woodTextures);
    Mesh safe = loader.loadObj("Resources/Models/safe.obj", keyTextures);
    Mesh keyModel = loader.loadObj("Resources/Models/key.obj", keyTextures);
    Mesh zombie1 = loader.loadObj("Resources/Models/zombie1.obj", z1Textures);
    Mesh zombie2 = loader.loadObj("Resources/Models/zombie2.obj", z2Textures);
    Mesh zombie3 = loader.loadObj("Resources/Models/zombie3.obj", z3Textures);
    Mesh playerBody = loader.loadObj("Resources/Models/man.obj", manTextures);
    Mesh playerHand = loader.loadObj("Resources/Models/gun.obj", flashTextures);
    Mesh flashlightModel = loader.loadObj("Resources/Models/flash.obj", flashTextures);
    Mesh bulletModel = loader.loadObj("Resources/Models/bullet.obj", keyTextures);
    Mesh bossModel = loader.loadObj("Resources/Models/boss.obj", lavaTextures);
    Mesh iceBossModel = loader.loadObj("Resources/Models/boss.obj", iceBossTextures);
    Mesh fireballModel = loader.loadObj("Resources/Models/fireball.obj", orangeTextures);
    Mesh portalModel = loader.loadObj("Resources/Models/portal.obj", portalTextures);
    Mesh iceIsland = loader.loadObj("Resources/Models/volcano.obj", iceTextures);


    treePositions.push_back(glm::vec3(-145.0f, -4.0f, -120.0f));
    treePositions.push_back(glm::vec3(-155.0f, -4.0f, -128.0f));
    treePositions.push_back(glm::vec3(-140.0f, -4.0f, -115.0f));
    treePositions.push_back(glm::vec3(-160.0f, -4.0f, -110.0f));
    treePositions.push_back(glm::vec3(-150.0f, -4.0f, -140.0f));
    treePositions.push_back(glm::vec3(-120.0f, -4.0f, -100.0f));
    treePositions.push_back(glm::vec3(-110.0f, -4.0f, -90.0f));
    treePositions.push_back(glm::vec3(-100.0f, -4.0f, -130.0f));
    treePositions.push_back(glm::vec3(-80.0f, -4.0f, -120.0f));
    treePositions.push_back(glm::vec3(-50.0f, -4.0f, 50.0f));
    treePositions.push_back(glm::vec3(-40.0f, -4.0f, 60.0f));
    treePositions.push_back(glm::vec3(40.0f, -4.0f, 40.0f));
    treePositions.push_back(glm::vec3(50.0f, -4.0f, 30.0f));
    treePositions.push_back(glm::vec3(20.0f, -4.0f, -80.0f));
    treePositions.push_back(glm::vec3(30.0f, -4.0f, -90.0f));


    float tileWidth = 200.0f; 
    int gridRange = 3; 

    for (int x = -gridRange; x <= gridRange; x++) {
        for (int z = -gridRange; z <= gridRange; z++) {
            
            if (x == 0 && z == 0) continue;


            lavaPositions.push_back(glm::vec3(x * tileWidth, -25.0f, z * tileWidth));
        }
    }

    zombies.push_back({ glm::vec3(-10.0f, -4.0f, -110.0f), 1 });
    zombies.push_back({ glm::vec3(10.0f, -4.0f, -130.0f), 2 });
    zombies.push_back({ glm::vec3(0.0f, -4.0f, -140.0f), 3 });
    zombies.push_back({ glm::vec3(-15.0f, -4.0f, -125.0f), 1 });
    zombies.push_back({ glm::vec3(15.0f, -4.0f, -115.0f), 2 });

    while (!window.isPressed(GLFW_KEY_ESCAPE) && glfwWindowShouldClose(window.getWindow()) == 0)
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        window.clear();
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (!messageLog.empty()) {
            messageLog.front().timer -= deltaTime;
            if (messageLog.front().timer <= 0) messageLog.pop_front();
        }



        processKeyboardInput();
        processMouseInput();
        
        updateGameLogic();

        glm::mat4 ProjectionMatrix = glm::perspective(90.0f, (float)window.getWidth() / (float)window.getHeight(), 0.1f, 10000.0f);
        glm::mat4 ViewMatrix = glm::lookAt(camera.getCameraPosition(), camera.getCameraPosition() + camera.getCameraViewDirection(), camera.getCameraUp());

        if (showInventory) {
            ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_FirstUseEver);
            ImGui::Begin("Inventory");
            ImGui::Text("Click to Hold Item:");
            ImGui::Separator();
            if (ImGui::Button("Empty Hands")) currentItem = 0;
            if (ImGui::Button("Flashlight")) currentItem = 1;
            if (hasGun) { if (ImGui::Button("Gun")) currentItem = 2; }
            else { ImGui::TextDisabled("Gun (Not Found)"); }
            if (hasKey) { if (ImGui::Button("Key")) currentItem = 3; }
            else { ImGui::TextDisabled("Key (Not Found)"); }
            ImGui::End();
        }

        ImGui::SetNextWindowPos(ImVec2(10, 10));
        ImGui::SetNextWindowSize(ImVec2(320, 180));
        ImGui::Begin("HUD", NULL,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoBackground);
        if (bossActivated && !bossDead) {
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Fire Dragon");

            float bossHpRatio = (float)bossHP / (float)bossMaxHP;

            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, IM_COL32(255, 80, 80, 255));
            ImGui::ProgressBar(bossHpRatio, ImVec2(180, 20), "");
            ImGui::PopStyleColor();
        }



        std::string missionText;
        switch (currentMission) {
        case FIND_KEY: missionText = "Mission: Find the key in the forest"; break;
        case OPEN_SAFE: missionText = "Mission: Open the safe in the house"; break;
        case FREE_HORSE: missionText = "Mission: Rescue the horse"; break;
        case CROSS_LAVA: missionText = "Mission: Cross the lava with the horse"; break;
        case FIGHT_BOSS: missionText = "Mission: Fight the Fire Dragon"; break;
        }

        ImGui::TextColored(ImVec4(0.6f, 0.9f, 1.0f, 1.0f), missionText.c_str());
        ImGui::Separator();


        ImGui::Text("Player HP: %d / %d", playerHP, maxPlayerHP);
        ImGui::ProgressBar(
            (float)playerHP / (float)maxPlayerHP,
            ImVec2(180, 20),
            ""
        );


        if (!messageLog.empty()) {
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(1, 1, 0, 1), messageLog.front().text.c_str());
        }

        interactionFlashTimer += deltaTime;
        bool interactionFlashOn = fmod(interactionFlashTimer, 1.0f) < 0.5f;

        if (showInteraction && interactionFlashOn) {
            ImGui::Spacing();
            ImGui::TextColored(
                ImVec4(1.0f, 1.0f, 0.3f, 1.0f),
                interactionText.c_str()
            );
        }

        ImGui::End();


        if (!keyPickedUp) {
            glm::vec2 keyScreen = WorldToScreen(keyPos + glm::vec3(0, 2.0f, 0), ViewMatrix, ProjectionMatrix, window.getWidth(), window.getHeight());
            if (keyScreen.x != -1.0f) {
                ImGui::SetNextWindowPos(ImVec2(keyScreen.x, keyScreen.y));
                ImGui::Begin("KeyLabel", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_AlwaysAutoResize);
                ImGui::TextColored(ImVec4(1, 1, 0, 1), "KEY");
                ImGui::End();
            }
        }

        glm::vec2 safeScreen = WorldToScreen(safePos + glm::vec3(0, 10.0f, 0), ViewMatrix, ProjectionMatrix, window.getWidth(), window.getHeight());
        if (safeScreen.x != -1.0f) {
            ImGui::SetNextWindowPos(ImVec2(safeScreen.x, safeScreen.y));
            ImGui::Begin("SafeLabel", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::TextColored(ImVec4(0, 1, 1, 1), "SAFEBOX");
            ImGui::End();
        }

        for (const auto& z : zombies) {
            if (z.isDead) continue;
            glm::vec3 headPos = z.pos + glm::vec3(0.0f, 8.0f, 0.0f);
            glm::vec2 screenPos = WorldToScreen(headPos, ViewMatrix, ProjectionMatrix, (float)window.getWidth(), (float)window.getHeight());
            if (screenPos.x != -1.0f && screenPos.x > 0 && screenPos.x < window.getWidth() && screenPos.y > 0 && screenPos.y < window.getHeight()) {
                ImGui::SetNextWindowPos(ImVec2(screenPos.x - 30, screenPos.y));
                ImGui::Begin(std::string("HP" + std::to_string((long long)&z)).c_str(), NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_AlwaysAutoResize);
                if (z.hp == 2) ImGui::PushStyleColor(ImGuiCol_PlotHistogram, IM_COL32(0, 255, 0, 255));
                else ImGui::PushStyleColor(ImGuiCol_PlotHistogram, IM_COL32(255, 0, 0, 255));
                ImGui::ProgressBar((float)z.hp / 2.0f, ImVec2(60, 10), "");
                ImGui::PopStyleColor();
                ImGui::End();
            }
        }

        if (!showInventory) {
            ImDrawList* drawList = ImGui::GetForegroundDrawList();
            ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            drawList->AddLine(ImVec2(center.x - 10, center.y), ImVec2(center.x + 10, center.y), IM_COL32(0, 255, 0, 255), 2.0f);
            drawList->AddLine(ImVec2(center.x, center.y - 10), ImVec2(center.x, center.y + 10), IM_COL32(0, 255, 0, 255), 2.0f);
        }

        glm::vec3 currentCamPos = camera.getCameraPosition();

        if (!freeCam) {
            float playerHeight = isRiding ? 25.0f : 22.0f;
            float terrainY = getTerrainHeight(currentCamPos.x, currentCamPos.z);
            camera.setCameraPosition(glm::vec3(currentCamPos.x, terrainY + playerHeight, currentCamPos.z));
            playerWorldPos = glm::vec3(currentCamPos.x, terrainY, currentCamPos.z);
        }

        for (auto& z : zombies) {
            if (z.isDead) continue;
            float dist = glm::distance(playerWorldPos, z.pos);
            if (dist < 60.0f && dist > 1.5f) {
                glm::vec3 dir = glm::normalize(playerWorldPos - z.pos);
                dir.y = 0;
                float speed = (z.type == 2) ? 9.5f : 5.5f;
                z.pos += dir * (speed * deltaTime);
                z.pos.y = -4.0f;
            }
        }

        sunShader.use();

        glUniformMatrix4fv(glGetUniformLocation(sunShader.getId(), "MVP"), 1, GL_FALSE, &((ProjectionMatrix * ViewMatrix * glm::translate(glm::mat4(1.0f), lightPos))[0][0]));
        sun.draw(sunShader);

        for (auto& b : bullets) {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), b.position);
            model = glm::scale(model, glm::vec3(3.0f));
            glUniformMatrix4fv(glGetUniformLocation(sunShader.getId(), "MVP"), 1, GL_FALSE, &((ProjectionMatrix * ViewMatrix * model)[0][0]));
            bulletModel.draw(sunShader);
        }


        shader.use();
        GLuint MVP_ID = glGetUniformLocation(shader.getId(), "MVP");
        GLuint MODEL_ID = glGetUniformLocation(shader.getId(), "model");
        glUniform3f(glGetUniformLocation(shader.getId(), "lightColor"), lightColor.x, lightColor.y, lightColor.z);
        glUniform3f(glGetUniformLocation(shader.getId(), "lightPos"), lightPos.x, lightPos.y, lightPos.z);
        glUniform3f(glGetUniformLocation(shader.getId(), "viewPos"), camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

        bool flashOn = (currentItem == 1);
        glUniform1i(glGetUniformLocation(shader.getId(), "flashOn"), flashOn ? 1 : 0);
        glUniform3f(glGetUniformLocation(shader.getId(), "flashPos"), camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);
        glUniform3f(glGetUniformLocation(shader.getId(), "flashDir"), camera.getCameraViewDirection().x, camera.getCameraViewDirection().y, camera.getCameraViewDirection().z);
        glUniform1f(glGetUniformLocation(shader.getId(), "flashCutOff"), glm::cos(glm::radians(12.5f)));

        auto DrawMesh = [&](Mesh& m, glm::vec3 pos, glm::vec3 scale, bool useTex) {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
            model = glm::scale(model, scale);
            glm::mat4 mvp = ProjectionMatrix * ViewMatrix * model;
            glUniformMatrix4fv(MVP_ID, 1, GL_FALSE, &mvp[0][0]);
            glUniformMatrix4fv(MODEL_ID, 1, GL_FALSE, &model[0][0]);
            glUniform1i(glGetUniformLocation(shader.getId(), "useTexture"), useTex);
            m.draw(shader);
            };
        for (const auto& f : fireballs) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texOrange);
            glUniform1i(glGetUniformLocation(shader.getId(), "texture_diffuse1"), 0);

            DrawMesh(fireballModel, f.position, glm::vec3(1.5f), true);
        }
        if (iceBossActivated && !iceBossDead) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texIce);
            glUniform1i(glGetUniformLocation(shader.getId(), "texture_diffuse1"), 0);

            DrawMesh(iceBossModel, iceBossPos, glm::vec3(3.0f), true);
        }

        if (portalActive) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texPortal);
            glUniform1i(glGetUniformLocation(shader.getId(), "texture_diffuse1"), 0);

            DrawMesh(portalModel, portalPos, glm::vec3(6.0f), true);
        }
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texIce);
        glUniform1i(glGetUniformLocation(shader.getId(), "texture_diffuse1"), 0);

        DrawMesh(iceIsland, glm::vec3(400.0f, -4.0f, 400.0f), glm::vec3(0.1f), true);



        glUniform3f(glGetUniformLocation(shader.getId(), "objectColor"), 0.25f, 0.7f, 0.25f);
        DrawMesh(plane, glm::vec3(0, -20, 0), glm::vec3(1.0f), false);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texLava);
        glUniform1i(glGetUniformLocation(shader.getId(), "texture_diffuse1"), 0);
        DrawMesh(lavamap, lavaMapPos, glm::vec3(1.0f), true);
        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, texOrange); glUniform1i(glGetUniformLocation(shader.getId(), "texture_diffuse1"), 0);
        for (const auto& pos : lavaPositions) {
            DrawMesh(lavamap, pos, glm::vec3(1.0f), true);
            DrawMesh(lava, pos, glm::vec3(1.0f), true);
        }

        glUniform3f(glGetUniformLocation(shader.getId(), "objectColor"), 1.0f, 1.0f, 1.0f);
        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, texWood); glUniform1i(glGetUniformLocation(shader.getId(), "texture_diffuse1"), 0);
        DrawMesh(hospital, hospitalPos, glm::vec3(7.0f), true);
        DrawMesh(house, housePos, glm::vec3(12.0f), true);

        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, texWood2); glUniform1i(glGetUniformLocation(shader.getId(), "texture_diffuse1"), 0);
        DrawMesh(stable, stablePos, glm::vec3(5.0f), true);

        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, texKey); glUniform1i(glGetUniformLocation(shader.getId(), "texture_diffuse1"), 0);
        DrawMesh(safe, safePos, glm::vec3(12.5f), true);

        if (!keyPickedUp) {
            glUniform3f(glGetUniformLocation(shader.getId(), "objectColor"), 1.0f, 1.0f, 1.0f);
            DrawMesh(keyModel, keyPos, glm::vec3(1.5f), true);
        }

        glm::vec3 drawHorsePos = isRiding ? glm::vec3(playerWorldPos.x, -4.0f, playerWorldPos.z) : horsePos;
        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, texOrange); glUniform1i(glGetUniformLocation(shader.getId(), "texture_diffuse1"), 0);
        DrawMesh(horse, drawHorsePos, glm::vec3(0.5f), true);

        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, texLava); glUniform1i(glGetUniformLocation(shader.getId(), "texture_diffuse1"), 0);
        DrawMesh(volcano, volcanoPos, glm::vec3(volcanoScale), true);
        if (bossActivated && !bossDead) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texLava);
            glUniform1i(glGetUniformLocation(shader.getId(), "texture_diffuse1"), 0);

            DrawMesh(bossModel, bossPos, glm::vec3(3.0f), true);
        }


        glUniform3f(glGetUniformLocation(shader.getId(), "objectColor"), 0.0f, 0.6f, 0.0f);
        glUniform1i(glGetUniformLocation(shader.getId(), "useTexture"), 0);
        for (const auto& pos : treePositions) DrawMesh(treeModel, pos, glm::vec3(6.0f), false);

        glUniform3f(glGetUniformLocation(shader.getId(), "objectColor"), 1.0f, 1.0f, 1.0f);
        glUniform1i(glGetUniformLocation(shader.getId(), "useTexture"), 1);
        for (const auto& z : zombies) {
            if (z.isDead) continue;
            glm::vec3 scale = (z.type == 2) ? glm::vec3(7.0f) : ((z.type == 3) ? glm::vec3(0.1f) : glm::vec3(1.0f));
            glActiveTexture(GL_TEXTURE0);
            if (z.type == 1) { glBindTexture(GL_TEXTURE_2D, texZ1); glUniform1i(glGetUniformLocation(shader.getId(), "texture_diffuse1"), 0); DrawMesh(zombie1, z.pos, scale, true); }
            else if (z.type == 2) { glBindTexture(GL_TEXTURE_2D, texZ2); glUniform1i(glGetUniformLocation(shader.getId(), "texture_diffuse1"), 0); DrawMesh(zombie2, z.pos, scale, true); }
            else { glBindTexture(GL_TEXTURE_2D, texZ3); glUniform1i(glGetUniformLocation(shader.getId(), "texture_diffuse1"), 0); DrawMesh(zombie3, z.pos, scale, true); }
        }

        float playerY = isRiding ? -1.5f : -4.0f;
        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, texMan); glUniform1i(glGetUniformLocation(shader.getId(), "texture_diffuse1"), 0);
        DrawMesh(playerBody, playerWorldPos + glm::vec3(0, playerY - (-4.0f), 0), glm::vec3(0.1f), true);

        glm::vec3 camFront = camera.getCameraViewDirection();
        glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 camRight = glm::normalize(glm::cross(camFront, worldUp));
        glm::vec3 camUp = glm::normalize(glm::cross(camRight, camFront));

        if (currentItem == 1) {
            glm::mat4 flashModel = BuildFPSItemModel(
                camera.getCameraPosition(),
                camFront,
                camRight,
                camUp,
                glm::vec3(0.35f, -0.35f, 1.2f),
                glm::vec3(0.15f)
            );

            glm::mat4 mvp = ProjectionMatrix * ViewMatrix * flashModel;
            glUniformMatrix4fv(glGetUniformLocation(shader.getId(), "model"), 1, GL_FALSE, &flashModel[0][0]);
            glUniformMatrix4fv(glGetUniformLocation(shader.getId(), "MVP"), 1, GL_FALSE, &mvp[0][0]);
            glUniform1i(glGetUniformLocation(shader.getId(), "useTexture"), 1);

            flashlightModel.draw(shader);

        }
        else if (currentItem == 2 && hasGun) {

            glm::mat4 gunModel = BuildFPSItemModel(
                camera.getCameraPosition(),
                camFront,
                camRight,
                camUp,
                glm::vec3(0.45f, -0.45f, 1.3f),
                glm::vec3(3.5f)
            );

            gunModel = glm::rotate(gunModel, glm::radians(180.0f), glm::vec3(0, 1, 0));

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texFlash);
            glUniform1i(glGetUniformLocation(shader.getId(), "texture_diffuse1"), 0);
            glUniform1i(glGetUniformLocation(shader.getId(), "useTexture"), 1);

            glm::mat4 mvp = ProjectionMatrix * ViewMatrix * gunModel;
            glUniformMatrix4fv(glGetUniformLocation(shader.getId(), "model"), 1, GL_FALSE, &gunModel[0][0]);
            glUniformMatrix4fv(glGetUniformLocation(shader.getId(), "MVP"), 1, GL_FALSE, &mvp[0][0]);

            playerHand.draw(shader);
        }


        else if (currentItem == 3 && hasKey) {
            glm::vec3 renderPos = camera.getCameraPosition() + (camFront * 1.5f) + (camRight * 0.4f) - (camUp * 0.4f);
            glUniform3f(glGetUniformLocation(shader.getId(), "objectColor"), 1.0f, 1.0f, 1.0f);
            glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, texKey); glUniform1i(glGetUniformLocation(shader.getId(), "texture_diffuse1"), 0);
            DrawMesh(keyModel, renderPos, glm::vec3(0.3f), true);
        }

        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        glm::mat4 view = glm::mat4(glm::mat3(ViewMatrix));
        glUniformMatrix4fv(glGetUniformLocation(skyboxShader.getId(), "view"), 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(skyboxShader.getId(), "projection"), 1, GL_FALSE, &ProjectionMatrix[0][0]);
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glUniform1i(glGetUniformLocation(skyboxShader.getId(), "skybox"), 0);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);
        if (damageFlashTimer > 0.0f) {
            ImGui::GetForegroundDrawList()->AddRectFilled(
                ImVec2(0, 0),
                ImGui::GetIO().DisplaySize,
                IM_COL32(255, 0, 0, 80)
            );
        }
        if (iceBossActivated && !iceBossDead) {
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "Ice Dragon");

            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, IM_COL32(100, 180, 255, 255));
            ImGui::ProgressBar(
                (float)iceBossHP / (float)iceBossMaxHP,
                ImVec2(180, 20),
                ""
            );
            ImGui::PopStyleColor();
        }


        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        window.update();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    return 0;
}

void processKeyboardInput() {
    float cameraSpeed = (isRiding ? 80.0f : 30.0f) * deltaTime;
    if (freeCam) cameraSpeed = 100.0f * deltaTime;

    glm::vec3 nextPos = playerWorldPos;
    bool moving = false;


    if (window.isPressed(GLFW_KEY_W)) {
        nextPos += camera.getCameraViewDirection() * cameraSpeed;
        moving = true;
    }
    if (window.isPressed(GLFW_KEY_S)) {
        nextPos -= camera.getCameraViewDirection() * cameraSpeed;
        moving = true;
    }
    if (window.isPressed(GLFW_KEY_A)) {
        nextPos -= glm::normalize(glm::cross(camera.getCameraViewDirection(), camera.getCameraUp())) * cameraSpeed;
        moving = true;
    }
    if (window.isPressed(GLFW_KEY_D)) {
        nextPos += glm::normalize(glm::cross(camera.getCameraViewDirection(), camera.getCameraUp())) * cameraSpeed;
        moving = true;
    }

    if (!freeCam && !isInsideHouse) {
        if (glm::distance(nextPos, housePos) < 25.0f ||
            glm::distance(nextPos, hospitalPos) < 15.0f ||
            glm::distance(nextPos, stablePos) < 10.0f) {
            moving = false;
        }

        for (const auto& tPos : treePositions) {
            if (glm::distance(nextPos, tPos) < 5.0f) {
                moving = false;
            }
        }
    }

    if (moving) {
        if (window.isPressed(GLFW_KEY_W)) camera.keyboardMoveFront(cameraSpeed);
        if (window.isPressed(GLFW_KEY_S)) camera.keyboardMoveBack(cameraSpeed);
        if (window.isPressed(GLFW_KEY_A)) camera.keyboardMoveLeft(cameraSpeed);
        if (window.isPressed(GLFW_KEY_D)) camera.keyboardMoveRight(cameraSpeed);
    }

    if (freeCam) {
        if (window.isPressed(GLFW_KEY_R)) camera.keyboardMoveUp(cameraSpeed);
        if (window.isPressed(GLFW_KEY_G)) camera.keyboardMoveDown(cameraSpeed);
    }

    static bool iLast = false; bool iCurr = window.isPressed(GLFW_KEY_I);
    if (iCurr && !iLast) {
        showInventory = !showInventory;
        if (showInventory) glfwSetInputMode(window.getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        else glfwSetInputMode(window.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    iLast = iCurr;

    static bool fiveLast = false; bool fiveCurr = window.isPressed(GLFW_KEY_5);
    if (fiveCurr && !fiveLast) { freeCam = !freeCam; if (!freeCam) { float h = isRiding ? 22.0f : 20.0f; camera.setCameraPosition(glm::vec3(playerWorldPos.x, h, playerWorldPos.z)); } }
    fiveLast = fiveCurr;

    static bool fLast = false; bool fCurr = window.isPressed(GLFW_KEY_F);
    if (fCurr && !fLast) {
        if (glm::distance(playerWorldPos, housePos) < 30.0f || glm::distance(playerWorldPos, hospitalPos) < 20.0f) {
            isInsideHouse = !isInsideHouse;
            AddMessage(isInsideHouse ? "Entered Building" : "Exited Building");
        }
    }
    fLast = fCurr;

    static bool eLast = false; bool eCurr = window.isPressed(GLFW_KEY_E);
    if (eCurr && !eLast) eKeyPressed = true;
    eLast = eCurr;
}

void processMouseInput() {
    if (showInventory) return;

    static bool mouseLast = false;
    bool mouseCurr = glfwGetMouseButton(window.getWindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

    if (mouseCurr && !mouseLast && currentItem == 2 && hasGun) {
        Bullet b;
        b.position = camera.getCameraPosition();
        b.direction = camera.getCameraViewDirection();
        b.lifeTime = 3.0f;
        bullets.push_back(b);
        std::cout << ">> PEW!" << std::endl;
    }
    mouseLast = mouseCurr;

    double xpos, ypos; glfwGetCursorPos(window.getWindow(), &xpos, &ypos);
    if (firstMouse) { lastX = (float)xpos; lastY = (float)ypos; firstMouse = false; }

    float xoffset = lastX - (float)xpos;
    float yoffset = lastY - (float)ypos;
    lastX = (float)xpos; lastY = (float)ypos;

    camera.rotateOy(xoffset * 0.05f);
    camera.rotateOx(yoffset * 0.05f);
}

void updateGameLogic() {
    if (iceBossActivated && !iceBossDead) {
        iceBossPos.y = -4.0f + bossFlyHeight;

        iceBossMoveTimer += deltaTime;

        if (iceBossMoveLeft) {
            iceBossPos.x -= 10.0f * deltaTime;
            if (iceBossMoveTimer > 3.0f) {
                iceBossMoveLeft = false;
                iceBossMoveTimer = 0.0f;
            }
        }
        else {
            iceBossPos.x += 10.0f * deltaTime;
            if (iceBossMoveTimer > 6.0f) {
                iceBossMoveLeft = true;
                iceBossMoveTimer = 0.0f;
            }
        }

    }

    if (!iceBossActivated && !iceBossDead &&
        glm::distance(playerWorldPos, iceBossPos) < 120.0f) {

        iceBossActivated = true;
        AddMessage("The Ice Dragon awakens!");
    }


    if (!bossActivated && glm::distance(playerWorldPos, bossPos) < 120.0f) {
        bossActivated = true;
        AddMessage("The Fire Dragon awakens!");
    }
    if (bossActivated && !bossDead) {

        bossPos.y = -4.0f + bossFlyHeight;

        bossMoveTimer += deltaTime;

        if (bossMoveLeft) {
            bossPos.x -= 10.0f * deltaTime;
            if (bossMoveTimer > 3.0f) {
                bossMoveLeft = false;
                bossMoveTimer = 0.0f;
            }
        }
        else {
            bossPos.x += 10.0f * deltaTime;
            if (bossMoveTimer > 6.0f) {
                bossMoveLeft = true;
                bossMoveTimer = 0.0f;
            }
        }
    }

    if (bossActivated && !bossDead) {
        bossShootTimer += deltaTime;

        if (bossShootTimer >= 2.0f) {
            Fireball f;
            f.position = bossPos;
            f.direction = glm::normalize(playerWorldPos - bossPos);
            f.lifeTime = 5.0f;

            fireballs.push_back(f);
            bossShootTimer = 0.0f;
        }
    }

    static float zombieAttackCooldown = 0.0f;
    zombieAttackCooldown -= deltaTime;
    if (keyPickedUp && currentMission == FIND_KEY)
        currentMission = OPEN_SAFE;

    if (hasGun && currentMission == OPEN_SAFE)
        currentMission = FREE_HORSE;

    if (isRiding && currentMission == FREE_HORSE)
        currentMission = CROSS_LAVA;

    showInteraction = false;
    interactionText = "";
    if (damageFlashTimer > 0.0f)
        damageFlashTimer -= deltaTime;
    for (size_t i = 0; i < bullets.size(); i++) {
        bullets[i].position += bullets[i].direction * (100.0f * deltaTime);
        bullets[i].lifeTime -= deltaTime;

        for (auto& z : zombies) {
            if (z.isDead) continue;

            float hitDist = glm::distance(bullets[i].position, z.pos);

            if (hitDist < 5.0f) {  
                z.hp -= 1;
                AddMessage("Zombie hit!");

                if (z.hp <= 0) {
                    z.isDead = true;
                    AddMessage("Zombie killed!");
                }

                bullets[i].lifeTime = 0.0f; 
                break;
            }
        }
        if (bossActivated && !iceBossDead) {
            float bossHitDist = glm::distance(bullets[i].position, bossPos);
            if (iceBossActivated && !iceBossDead) {
                float hitDist = glm::distance(bullets[i].position, iceBossPos);

                if (hitDist < 6.0f) {
                    iceBossHP -= 1;
                    bullets[i].lifeTime = 0.0f;
                    AddMessage("Ice Boss hit!");

                    if (iceBossHP <= 0) {
                        iceBossDead = true;
                        AddMessage("Ice Dragon defeated!");
                    }
                }
            }


            if (bossHitDist < 6.0f) {
                bossHP -= 1;
                bullets[i].lifeTime = 0.0f;
                AddMessage("Boss hit!");
                
                if (bossHP <= 0) {
                    bossDead = true;
                    portalActive = true;

                    portalPos = bossPos;
                    portalPos.y = -4.0f; 

                    AddMessage("A portal opens...");
                }

            }
        }

    }


    bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [](const Bullet& b) { return b.lifeTime <= 0.0f; }), bullets.end());
    
    if (!keyPickedUp && glm::distance(playerWorldPos, keyPos) < 5.0f) {
        showInteraction = true;
        interactionText = "Press E to pick up the key";
    }
    if (glm::distance(playerWorldPos, safePos) < 15.0f) {
        showInteraction = true;

        if (!hasKey)
            interactionText = "The safe is locked";
        else if (currentItem != 3)
            interactionText = "Hold the key to open the safe";
        else
            interactionText = "Press E to open the safe";
    }
    if (glm::distance(playerWorldPos, horsePos) < 6.0f) {
        showInteraction = true;
        interactionText = isRiding
            ? "Press E to hop off the horse"
            : "Press E to ride the horse";
    }

    if (!hasKey && !keyPickedUp && glm::distance(playerWorldPos, keyPos) < 5.0f && eKeyPressed) {
        hasKey = true;
        keyPickedUp = true;
        AddMessage("Key Obtained!");
    }
    static float lavaTimer = 0.0f;

    if (isOutsideMap(playerWorldPos)) {
        if (!isRiding) {
            lavaTimer += deltaTime;

            if (lavaTimer > 0.5f) {   
                playerHP -= 15;
                AddMessage("You are burning in lava!");
                lavaTimer = 0.0f;
            }
        }
    }
    else {
        lavaTimer = 0.0f; 
    }


    if (hasKey && !hasGun && glm::distance(playerWorldPos, safePos) < 15.0f && eKeyPressed) {
        if (currentItem == 3) {
            hasGun = true;
            AddMessage("Safe Opened! Gun Obtained");
        }
        else {
            AddMessage("Need to HOLD Key!");
        }
    }
    else if (!hasKey && !hasGun && glm::distance(playerWorldPos, safePos) < 15.0f && eKeyPressed) {
        AddMessage("Safe Locked!");
    }
    for (size_t i = 0; i < fireballs.size(); i++) {
        fireballs[i].position += fireballs[i].direction * (40.0f * deltaTime);
        fireballs[i].lifeTime -= deltaTime;

        if (glm::distance(fireballs[i].position, playerWorldPos) < 4.0f) {
            playerHP -= 5;
            AddMessage("Fireball hit!");
            damageFlashTimer = 0.2f;
            fireballs[i].lifeTime = 0.0f;
        }
    }

    fireballs.erase(
        std::remove_if(fireballs.begin(), fireballs.end(),
            [](const Fireball& f) { return f.lifeTime <= 0.0f; }),
        fireballs.end()
    );


    if (!isRiding && glm::distance(playerWorldPos, horsePos) < 6.0f && eKeyPressed) {
        isRiding = true;
        AddMessage("Horse Mounted!");
    }
    else if (isRiding && eKeyPressed) {
        isRiding = false;
        horsePos = glm::vec3(playerWorldPos.x, -4.0f, playerWorldPos.z);
        AddMessage("Horse Freed!");
    }
    if (playerHP < 0) playerHP = 0;
    if (portalActive && glm::distance(playerWorldPos, portalPos) < 8.0f) {
        showInteraction = true;
        interactionText = "Press E to enter the portal";

        if (eKeyPressed) {
            camera.setCameraPosition(glm::vec3(
                iceIslandSpawn.x,
                iceIslandSpawn.y + 22.0f,
                iceIslandSpawn.z
            ));

            playerWorldPos = iceIslandSpawn;

            AddMessage("You entered the Frozen Island");
        }
    }

    eKeyPressed = false;
}