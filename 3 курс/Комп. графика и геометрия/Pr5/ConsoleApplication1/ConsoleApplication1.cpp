#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <map>

// ===================== Параметры варианта 2 =====================
const float FOV = 40.0f;
const float NEAR_PLANE = 5.0f;
const float FAR_PLANE = 500.0f;

// ===================== Камера =====================
float cameraDistance = 80.0f;
float cameraYaw = 25.0f;
float cameraPitch = 18.0f;
glm::vec3 cameraTarget(0.0f, 0.0f, 0.0f);

float objectOffsetX = 0.0f;
float objectRotationY = 0.0f;

bool keys[1024] = { false };
int WinWidth = 1280, WinHeight = 720;

// ===================== Модель =====================
struct Model {
    std::vector<float> data;          // pos(3) + normal(3) + tex(2)
    std::vector<unsigned int> indices;
    GLuint VAO = 0, VBO = 0, EBO = 0;
    GLuint textureID = 0;
    size_t indexCount = 0;
    bool hasTexture = false;
};

// ===================== Загрузка текстуры =====================
GLuint loadTexture(const std::string& path)
{
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
    if (!data) {
        std::cerr << "Не удалось загрузить текстуру: " << path << std::endl;
        return 0;
    }

    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    return texture;
}

// ===================== Загрузчик OBJ =====================
bool loadOBJ(const std::string& path, Model& model)
{
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texcoords;

    std::vector<unsigned int> posIdx, texIdx, normIdx;

    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Не удалось открыть: " << path << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "v") {
            glm::vec3 v;
            iss >> v.x >> v.y >> v.z;
            positions.push_back(v);
        }
        else if (prefix == "vn") {
            glm::vec3 n;
            iss >> n.x >> n.y >> n.z;
            normals.push_back(n);
        }
        else if (prefix == "vt") {
            glm::vec2 t;
            iss >> t.x >> t.y;
            texcoords.push_back(t);
        }
        else if (prefix == "f") {
            std::string vert;
            std::vector<unsigned int> fp, ft, fn;

            while (iss >> vert) {
                std::replace(vert.begin(), vert.end(), '/', ' ');
                std::istringstream viss(vert);
                unsigned int p = 0, t = 0, n = 0;
                viss >> p >> t >> n;
                fp.push_back(p);
                ft.push_back(t);
                fn.push_back(n);
            }

            // Триангуляция
            for (size_t i = 1; i + 1 < fp.size(); ++i) {
                posIdx.push_back(fp[0]);  posIdx.push_back(fp[i]);  posIdx.push_back(fp[i + 1]);
                texIdx.push_back(ft[0]);  texIdx.push_back(ft[i]);  texIdx.push_back(ft[i + 1]);
                normIdx.push_back(fn[0]); normIdx.push_back(fn[i]); normIdx.push_back(fn[i + 1]);
            }
        }
    }
    file.close();

    // Собираем interleave-массив
    for (size_t i = 0; i < posIdx.size(); ++i) {
        unsigned int pi = posIdx[i] - 1;
        unsigned int ti = (texIdx[i] > 0) ? texIdx[i] - 1 : 0;
        unsigned int ni = (normIdx[i] > 0) ? normIdx[i] - 1 : 0;

        glm::vec3 pos = positions[pi];
        glm::vec3 norm = (ni < normals.size()) ? normals[ni] : glm::vec3(0, 1, 0);
        glm::vec2 tex = (ti < texcoords.size()) ? texcoords[ti] : glm::vec2(0, 0);

        model.data.push_back(pos.x);  model.data.push_back(pos.y);  model.data.push_back(pos.z);
        model.data.push_back(norm.x); model.data.push_back(norm.y); model.data.push_back(norm.z);
        model.data.push_back(tex.x);  model.data.push_back(tex.y);

        model.indices.push_back((unsigned int)i);
    }

    model.indexCount = model.indices.size();
    return true;
}

void setupModel(Model& model)
{
    glGenVertexArrays(1, &model.VAO);
    glGenBuffers(1, &model.VBO);
    glGenBuffers(1, &model.EBO);

    glBindVertexArray(model.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, model.VBO);
    glBufferData(GL_ARRAY_BUFFER, model.data.size() * sizeof(float), model.data.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model.indices.size() * sizeof(unsigned int),
        model.indices.data(), GL_STATIC_DRAW);

    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texcoord
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

// ===================== Шейдеры =====================
const char* vsSrc = R"(
#version 400
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal  = mat3(transpose(inverse(model))) * aNormal;
    TexCoord = aTexCoord;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)";

const char* fsSrc = R"(
#version 400
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform sampler2D texture1;
uniform bool useTexture;
uniform vec3 objectColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

out vec4 FragColor;

void main() {
    vec3 color = useTexture ? texture(texture1, TexCoord).rgb : objectColor;

    // Простое освещение
    vec3 ambient = 0.25 * color;
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * color;

    FragColor = vec4(ambient + diffuse * 0.9, 1.0);
}
)";

GLuint createProgram()
{
    auto compile = [](GLenum type, const char* src) {
        GLuint s = glCreateShader(type);
        glShaderSource(s, 1, &src, nullptr);
        glCompileShader(s);
        return s;
        };

    GLuint vs = compile(GL_VERTEX_SHADER, vsSrc);
    GLuint fs = compile(GL_FRAGMENT_SHADER, fsSrc);

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);

    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

// ===================== Планета =====================
struct Planet {
    Model model;
    float orbitRadius;
    float size;
    float orbitSpeed;
    float selfRotation;
    glm::vec3 color;          // запасной цвет, если нет текстуры
    std::string name;
};

// ===================== Callbacks =====================
void key_callback(GLFWwindow* window, int key, int, int action, int)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS)   keys[key] = true;
        if (action == GLFW_RELEASE) keys[key] = false;
    }
}

void framebuffer_size_callback(GLFWwindow*, int w, int h)
{
    WinWidth = w; WinHeight = h;
    glViewport(0, 0, w, h);
}

void processInput(float dt)
{
    float camSpeed = 45.0f * dt;
    float objSpeed = 15.0f * dt;
    float rotSpeed = 70.0f * dt;

    if (keys[GLFW_KEY_LEFT])  cameraYaw -= camSpeed;
    if (keys[GLFW_KEY_RIGHT]) cameraYaw += camSpeed;
    if (keys[GLFW_KEY_UP])    cameraPitch += camSpeed;
    if (keys[GLFW_KEY_DOWN])  cameraPitch -= camSpeed;

    if (cameraPitch > 89.0f)  cameraPitch = 89.0f;
    if (cameraPitch < -89.0f) cameraPitch = -89.0f;

    if (keys[GLFW_KEY_EQUAL] || keys[GLFW_KEY_KP_ADD])    cameraDistance -= 25.0f * dt;
    if (keys[GLFW_KEY_MINUS] || keys[GLFW_KEY_KP_SUBTRACT]) cameraDistance += 25.0f * dt;
    if (cameraDistance < 10.0f)  cameraDistance = 10.0f;
    if (cameraDistance > 300.0f) cameraDistance = 300.0f;

    if (keys[GLFW_KEY_A]) objectOffsetX -= objSpeed;
    if (keys[GLFW_KEY_D]) objectOffsetX += objSpeed;
    if (keys[GLFW_KEY_Q]) objectRotationY -= rotSpeed;
    if (keys[GLFW_KEY_E]) objectRotationY += rotSpeed;
}

// ===================== main =====================
int main()
{
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WinWidth, WinHeight,
        "Практика 5 | Вариант 2 | Солнечная система", nullptr, nullptr);
    if (!window) { glfwTerminate(); return -1; }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) return -1;

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.01f, 0.01f, 0.04f, 1.0f);

    GLuint shader = createProgram();

    // ========== Загрузка планет ==========
    struct PlanetInfo {
        const char* obj;
        const char* tex;
        float orbit;
        float size;
        float speed;
        float rot;
        glm::vec3 color;
    };

    // Относительные размеры и примерные орбиты (не в реальном масштабе)
    std::vector<PlanetInfo> infos = {
        {"models/sun.obj",     "textures/sun.jpg",     0.0f,  8.0f,  0.0f,  5.0f,  {1.0f, 0.85f, 0.3f}},
        {"models/mercury.obj", "textures/mercury.jpg", 14.0f, 0.6f,  4.1f,  8.0f,  {0.7f, 0.7f, 0.7f}},
        {"models/venus.obj",   "textures/venus.jpg",   18.0f, 0.95f, 1.6f, -3.0f,  {0.9f, 0.75f, 0.4f}},
        {"models/earth.obj",   "textures/earth.png",   24.0f, 1.0f,  1.0f, 30.0f,  {0.2f, 0.4f, 0.9f}},
        {"models/mars.obj",    "textures/mars.jpg",    30.0f, 0.7f,  0.53f,25.0f,  {0.9f, 0.4f, 0.2f}},
        {"models/jupiter.obj", "textures/jupiter.jpg", 42.0f, 3.5f,  0.08f,12.0f,  {0.85f,0.7f, 0.5f}},
        {"models/saturn.obj",  "textures/saturn.jpg",  55.0f, 3.0f,  0.03f,10.0f,  {0.9f, 0.85f,0.6f}},
        {"models/uranus.obj",  "textures/uranus.jpg",  68.0f, 1.8f,  0.01f, 8.0f,  {0.5f, 0.8f, 0.9f}},
        {"models/neptune.obj", "textures/neptune.jpg", 80.0f, 1.7f,  0.006f,7.0f,  {0.3f, 0.4f, 0.9f}},
    };

    std::vector<Planet> planets;

    for (auto& info : infos) {
        Planet p;
        if (!loadOBJ(info.obj, p.model)) {
            std::cerr << "Ошибка загрузки " << info.obj << std::endl;
            continue;
        }
        setupModel(p.model);

        p.model.textureID = loadTexture(info.tex);
        p.model.hasTexture = (p.model.textureID != 0);

        p.orbitRadius = info.orbit;
        p.size = info.size;
        p.orbitSpeed = info.speed;
        p.selfRotation = info.rot;
        p.color = info.color;
        p.name = info.obj;

        planets.push_back(p);
        std::cout << "Загружено: " << info.obj << std::endl;
    }

    // Кольцо Сатурна
    Model saturnRing;
    bool hasRing = loadOBJ("models/saturn_ring.obj", saturnRing);
    if (hasRing) {
        setupModel(saturnRing);
        saturnRing.textureID = loadTexture("textures/saturn_ring.png");
        saturnRing.hasTexture = (saturnRing.textureID != 0);
    }

    float lastTime = (float)glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        float currentTime = (float)glfwGetTime();
        float dt = currentTime - lastTime;
        lastTime = currentTime;

        processInput(dt);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shader);

        // ----- Матрицы -----
        glm::mat4 projection = glm::perspective(glm::radians(FOV),
            (float)WinWidth / WinHeight, NEAR_PLANE, FAR_PLANE);

        float yawRad = glm::radians(cameraYaw);
        float pitchRad = glm::radians(cameraPitch);

        glm::vec3 cameraPos;
        cameraPos.x = cameraTarget.x + cameraDistance * cos(pitchRad) * sin(yawRad);
        cameraPos.y = cameraTarget.y + cameraDistance * sin(pitchRad);
        cameraPos.z = cameraTarget.z + cameraDistance * cos(pitchRad) * cos(yawRad);

        glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, glm::vec3(0, 1, 0));

        glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniform3f(glGetUniformLocation(shader, "lightPos"), 0.0f, 0.0f, 0.0f); // свет от Солнца
        glUniform3fv(glGetUniformLocation(shader, "viewPos"), 1, glm::value_ptr(cameraPos));

        // ----- Рисуем планеты -----
        for (size_t i = 0; i < planets.size(); ++i)
        {
            Planet& p = planets[i];

            glm::mat4 model = glm::mat4(1.0f);

            // Общее смещение и вращение всей системы
            model = glm::translate(model, glm::vec3(objectOffsetX, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(objectRotationY), glm::vec3(0, 1, 0));

            // Орбита
            float angle = currentTime * p.orbitSpeed;
            model = glm::translate(model, glm::vec3(
                p.orbitRadius * cos(angle),
                0.0f,
                p.orbitRadius * sin(angle)
            ));

            // Собственное вращение
            model = glm::rotate(model, currentTime * p.selfRotation, glm::vec3(0, 1, 0));
            model = glm::scale(model, glm::vec3(p.size));

            glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniform3fv(glGetUniformLocation(shader, "objectColor"), 1, glm::value_ptr(p.color));
            glUniform1i(glGetUniformLocation(shader, "useTexture"), p.model.hasTexture ? 1 : 0);

            if (p.model.hasTexture) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, p.model.textureID);
                glUniform1i(glGetUniformLocation(shader, "texture1"), 0);
            }

            glBindVertexArray(p.model.VAO);
            glDrawElements(GL_TRIANGLES, (GLsizei)p.model.indexCount, GL_UNSIGNED_INT, 0);

            // Кольцо Сатурна (индекс 6)
            if (i == 6 && hasRing) {
                glUniform1i(glGetUniformLocation(shader, "useTexture"), saturnRing.hasTexture ? 1 : 0);
                if (saturnRing.hasTexture) {
                    glBindTexture(GL_TEXTURE_2D, saturnRing.textureID);
                }
                glBindVertexArray(saturnRing.VAO);
                glDrawElements(GL_TRIANGLES, (GLsizei)saturnRing.indexCount, GL_UNSIGNED_INT, 0);
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Очистка
    for (auto& p : planets) {
        glDeleteVertexArrays(1, &p.model.VAO);
        glDeleteBuffers(1, &p.model.VBO);
        glDeleteBuffers(1, &p.model.EBO);
        if (p.model.textureID) glDeleteTextures(1, &p.model.textureID);
    }
    glDeleteProgram(shader);
    glfwTerminate();
    return 0;
}