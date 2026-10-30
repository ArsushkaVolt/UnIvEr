#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <iostream>
#include <cmath>
#include <string>

#ifndef PI
#define PI 3.14159265358979323846f
#endif

// ===================== Генерация сферы =====================
void genSphere(
    std::vector<GLuint>& outIndexes,
    std::vector<glm::vec3>& outVertices,
    std::vector<glm::vec3>& outNormals,
    float radius,
    int sectorCount,
    int stackCount)
{
    outIndexes.clear();
    outVertices.clear();
    outNormals.clear();

    float sectorStep = 2.0f * PI / sectorCount;
    float stackStep = PI / stackCount;

    for (int i = 0; i <= stackCount; ++i) {
        float stackAngle = PI / 2 - i * stackStep;
        float xy = radius * cosf(stackAngle);
        float z = radius * sinf(stackAngle);

        for (int j = 0; j <= sectorCount; ++j) {
            float sectorAngle = j * sectorStep;
            float x = xy * cosf(sectorAngle);
            float y = xy * sinf(sectorAngle);

            outVertices.emplace_back(x, y, z);
            outNormals.emplace_back(x / radius, y / radius, z / radius);
        }
    }

    for (int i = 0; i < stackCount; ++i) {
        int k1 = i * (sectorCount + 1);
        int k2 = k1 + sectorCount + 1;

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            if (i != 0) {
                outIndexes.push_back(k1);
                outIndexes.push_back(k2);
                outIndexes.push_back(k1 + 1);
            }
            if (i != (stackCount - 1)) {
                outIndexes.push_back(k1 + 1);
                outIndexes.push_back(k2);
                outIndexes.push_back(k2 + 1);
            }
        }
    }
}

// ===================== Шейдеры =====================
const char* vertexShaderSource = R"(
#version 400
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal  = mat3(transpose(inverse(model))) * aNormal;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 400
in vec3 FragPos;
in vec3 Normal;

uniform vec3 objectColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

out vec4 FragColor;

void main() {
    // Простое освещение (чтобы объекты выглядели объёмными)
    vec3 ambient = 0.25 * objectColor;
    
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * objectColor;
    
    vec3 result = ambient + diffuse;
    FragColor = vec4(result, 1.0);
}
)";

// ===================== Глобальные переменные =====================
int WinWidth = 1280;
int WinHeight = 720;

// Параметры камеры (вариант 2)
float cameraDistance = 25.0f;
float cameraYaw = 0.0f;
float cameraPitch = 20.0f;
glm::vec3 cameraTarget(0.0f, 0.0f, 0.0f);

// Управление объектом
float objectOffsetX = 0.0f;
float objectRotationY = 0.0f;

// Клавиши
bool keys[1024] = { false };

// ===================== Callbacks =====================
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    WinWidth = width;
    WinHeight = height;
    glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS)   keys[key] = true;
        if (action == GLFW_RELEASE) keys[key] = false;
    }
}

void processInput(float deltaTime) {
    float camSpeed = 40.0f * deltaTime;
    float objSpeed = 8.0f * deltaTime;
    float rotSpeed = 90.0f * deltaTime;

    // Камера: объезд
    if (keys[GLFW_KEY_LEFT])  cameraYaw -= camSpeed;
    if (keys[GLFW_KEY_RIGHT]) cameraYaw += camSpeed;
    if (keys[GLFW_KEY_UP])    cameraPitch += camSpeed;
    if (keys[GLFW_KEY_DOWN])  cameraPitch -= camSpeed;

    // Ограничение pitch
    if (cameraPitch > 89.0f)  cameraPitch = 89.0f;
    if (cameraPitch < -89.0f) cameraPitch = -89.0f;

    // Приближение / удаление
    if (keys[GLFW_KEY_EQUAL] || keys[GLFW_KEY_KP_ADD])    cameraDistance -= 15.0f * deltaTime;
    if (keys[GLFW_KEY_MINUS] || keys[GLFW_KEY_KP_SUBTRACT]) cameraDistance += 15.0f * deltaTime;
    if (cameraDistance < 8.0f)  cameraDistance = 8.0f;
    if (cameraDistance > 60.0f) cameraDistance = 60.0f;

    // Объект: движение влево/вправо
    if (keys[GLFW_KEY_A]) objectOffsetX -= objSpeed;
    if (keys[GLFW_KEY_D]) objectOffsetX += objSpeed;

    // Вращение объекта вокруг своей оси
    if (keys[GLFW_KEY_Q]) objectRotationY -= rotSpeed;
    if (keys[GLFW_KEY_E]) objectRotationY += rotSpeed;
}

// ===================== Создание шейдерной программы =====================
GLuint createShaderProgram() {
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexShaderSource, NULL);
    glCompileShader(vs);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentShaderSource, NULL);
    glCompileShader(fs);

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}

// ===================== Основная программа =====================
int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WinWidth, WinHeight,
        "Практика 5, вариант 2 — Солнечная система", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.02f, 0.02f, 0.08f, 1.0f); // почти чёрный космос

    GLuint shaderProgram = createShaderProgram();

    // ---- Генерация сферы (общая геометрия) ----
    std::vector<GLuint> indices;
    std::vector<glm::vec3> vertices, normals;
    genSphere(indices, vertices, normals, 1.0f, 48, 24); // unit sphere

    GLuint VAO, VBO, NBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &NBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, NBO);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);

    // ---- Планеты солнечной системы ----
    struct Planet {
        float orbitRadius;
        float size;
        glm::vec3 color;
        float orbitSpeed;
        float selfRotationSpeed;
    };

    std::vector<Planet> planets = {
        // Солнце
        { 0.0f,  2.8f, glm::vec3(1.0f, 0.85f, 0.2f), 0.0f, 10.0f },
        // Меркурий
        { 5.5f,  0.35f, glm::vec3(0.7f, 0.7f, 0.7f), 4.1f,  5.0f },
        // Венера
        { 7.5f,  0.55f, glm::vec3(0.9f, 0.7f, 0.3f), 1.6f,  -2.0f },
        // Земля
        { 10.0f, 0.6f,  glm::vec3(0.2f, 0.4f, 0.9f), 1.0f,  30.0f },
        // Марс
        { 12.5f, 0.45f, glm::vec3(0.9f, 0.3f, 0.15f), 0.53f, 28.0f },
        // Юпитер
        { 17.0f, 1.4f,  glm::vec3(0.85f, 0.7f, 0.5f), 0.084f, 15.0f },
        // Сатурн
        { 21.0f, 1.2f,  glm::vec3(0.9f, 0.85f, 0.6f), 0.034f, 12.0f }
    };

    float lastTime = (float)glfwGetTime();

    // Основной цикл
    while (!glfwWindowShouldClose(window)) {
        float currentTime = (float)glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        processInput(deltaTime);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // ===== Матрица проекции (вариант 2) =====
        glm::mat4 projection = glm::perspective(
            glm::radians(40.0f),                    // FOV 40°
            (float)WinWidth / (float)WinHeight,
            5.0f,                                   // near = 5
            10.0f * 6.0f                            // far = 10 → реально нужно больше, 
            // чтобы видеть всю систему
        );
        

        // ===== Матрица вида (камера) =====
        float yawRad = glm::radians(cameraYaw);
        float pitchRad = glm::radians(cameraPitch);

        glm::vec3 cameraPos;
        cameraPos.x = cameraTarget.x + cameraDistance * cos(pitchRad) * sin(yawRad);
        cameraPos.y = cameraTarget.y + cameraDistance * sin(pitchRad);
        cameraPos.z = cameraTarget.z + cameraDistance * cos(pitchRad) * cos(yawRad);

        glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, glm::vec3(0.0f, 1.0f, 0.0f));

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

        glUniform3f(glGetUniformLocation(shaderProgram, "lightPos"), 0.0f, 0.0f, 0.0f); // свет из центра (Солнце)
        glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, glm::value_ptr(cameraPos));

        glBindVertexArray(VAO);

        // Рисуем все объекты
        for (size_t i = 0; i < planets.size(); ++i) {
            Planet& p = planets[i];

            glm::mat4 model = glm::mat4(1.0f);

            // Общее смещение всей системы (задание 5)
            model = glm::translate(model, glm::vec3(objectOffsetX, 0.0f, 0.0f));

            // Общее вращение всей системы
            model = glm::rotate(model, glm::radians(objectRotationY), glm::vec3(0.0f, 1.0f, 0.0f));

            // Орбитальное движение
            float angle = currentTime * p.orbitSpeed;
            model = glm::translate(model, glm::vec3(
                p.orbitRadius * cos(angle),
                0.0f,
                p.orbitRadius * sin(angle)
            ));

            // Собственное вращение
            model = glm::rotate(model, currentTime * p.selfRotationSpeed, glm::vec3(0.0f, 1.0f, 0.0f));

            // Размер
            model = glm::scale(model, glm::vec3(p.size));

            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1, glm::value_ptr(p.color));

            glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &NBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}