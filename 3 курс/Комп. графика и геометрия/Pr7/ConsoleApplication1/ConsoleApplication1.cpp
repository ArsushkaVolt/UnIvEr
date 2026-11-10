#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <cmath>
#include <string>

// ============================================================
//  Практическая работа № 7, вариант 2
//  Фильтры: MAG = NEAREST, MIN = LINEAR
//  Объект: параллелограмм
//  Управление: тайлинг + повторения + вращение вокруг Z
// ============================================================

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"          // скачай stb_image.h и положи рядом

// -------------------- Глобальные переменные --------------------
int   WinWidth = 1280;
int   WinHeight = 720;

float texRepeat = 1.0f;         // количество повторений текстуры
bool  tileMode = true;         // включён ли тайлинг (GL_REPEAT)
float rotZ = 0.0f;         // угол вращения вокруг Z

GLuint textureID = 0;

// -------------------- Шейдеры --------------------
const char* vertexShaderSource = R"(
#version 400
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float uRepeat;

out vec2 TexCoord;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    TexCoord = aTexCoord * uRepeat;
}
)";

const char* fragmentShaderSource = R"(
#version 400
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D uTexture;

void main() {
    FragColor = texture(uTexture, TexCoord);
}
)";

// -------------------- Вспомогательные функции --------------------
GLuint CompileShader(GLenum type, const char* src) {
    GLuint sh = glCreateShader(type);
    glShaderSource(sh, 1, &src, nullptr);
    glCompileShader(sh);
    GLint ok;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(sh, 1024, nullptr, log);
        std::cerr << "Shader error:\n" << log << std::endl;
    }
    return sh;
}

GLuint CreateProgram() {
    GLuint vs = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

// Создание параллелограмма (вариант 2)
void CreateParallelogram(std::vector<float>& vertices, std::vector<unsigned int>& indices) {
    // Вершины: позиция (x,y,z) + текстурные координаты (s,t)
    // Наклонный четырёхугольник
    float verts[] = {
        // позиции                // texcoords
        -1.5f, -1.0f, 0.0f,       0.0f, 0.0f,   // 0 левый низ
         1.0f, -1.0f, 0.0f,       1.0f, 0.0f,   // 1 правый низ
         1.5f,  1.0f, 0.0f,       1.0f, 1.0f,   // 2 правый верх
        -1.0f,  1.0f, 0.0f,       0.0f, 1.0f    // 3 левый верх
    };

    unsigned int inds[] = {
        0, 1, 2,
        0, 2, 3
    };

    vertices.assign(verts, verts + 20);
    indices.assign(inds, inds + 6);
}

// Загрузка текстуры (или процедурная, если файла нет)
GLuint LoadTexture(const char* path) {
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    // === Фильтры по варианту 2 ===
    // Увеличивающий фильтр — простая (NEAREST)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // Уменьшающий фильтр — билинейная (LINEAR)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // Тайлинг (будет меняться клавишами)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    int w, h, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &w, &h, &channels, 0);

    if (data) {
        GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
        std::cout << "Текстура загружена: " << path << " (" << w << "x" << h << ")\n";
    }
    else {
        // Процедурная текстура 64×64 (шахматка)
        std::cout << "Файл текстуры не найден → создаём процедурную\n";
        const int size = 64;
        std::vector<unsigned char> proc(size * size * 3);
        for (int y = 0; y < size; ++y) {
            for (int x = 0; x < size; ++x) {
                bool black = ((x / 8) + (y / 8)) % 2 == 0;
                int i = (y * size + x) * 3;
                proc[i] = black ? 40 : 220;
                proc[i + 1] = black ? 40 : 180;
                proc[i + 2] = black ? 80 : 60;
            }
        }
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size, size, 0, GL_RGB, GL_UNSIGNED_BYTE, proc.data());
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    return tex;
}

void ApplyWrapMode() {
    glBindTexture(GL_TEXTURE_2D, textureID);
    GLint mode = tileMode ? GL_REPEAT : GL_CLAMP_TO_EDGE;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// -------------------- Обработка клавиш --------------------
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;

    switch (key) {
    case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window, true);
        break;

        // Вкл/выкл тайлинг
    case GLFW_KEY_T:
        tileMode = !tileMode;
        ApplyWrapMode();
        std::cout << "Тайлинг: " << (tileMode ? "ВКЛ (REPEAT)" : "ВЫКЛ (CLAMP)") << std::endl;
        break;

        // Увеличить количество повторений
    case GLFW_KEY_EQUAL:      // клавиша =
    case GLFW_KEY_KP_ADD:
        texRepeat += 0.5f;
        if (texRepeat > 10.0f) texRepeat = 10.0f;
        std::cout << "Повторений текстуры: " << texRepeat << std::endl;
        break;

        // Уменьшить количество повторений
    case GLFW_KEY_MINUS:
    case GLFW_KEY_KP_SUBTRACT:
        texRepeat -= 0.5f;
        if (texRepeat < 0.5f) texRepeat = 0.5f;
        std::cout << "Повторений текстуры: " << texRepeat << std::endl;
        break;

        // Вращение вокруг Z
    case GLFW_KEY_LEFT:
        rotZ += 5.0f;
        break;
    case GLFW_KEY_RIGHT:
        rotZ -= 5.0f;
        break;
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    WinWidth = width;
    WinHeight = height;
    glViewport(0, 0, width, height);
}

// ============================================================
int main() {
    if (!glfwInit()) {
        std::cerr << "Ошибка GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(WinWidth, WinHeight,
        "ПР №7, вариант 2 — Текстуры (параллелограмм)", nullptr, nullptr);
    if (!window) {
        std::cerr << "Не удалось создать окно\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSwapInterval(1);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Ошибка GLEW\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.12f, 0.12f, 0.15f, 1.0f);

    GLuint program = CreateProgram();

    // ---- Геометрия параллелограмма ----
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    CreateParallelogram(vertices, indices);

    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // позиция
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texcoord
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // ---- Текстура ----
    textureID = LoadTexture("texture.jpg");   // положи любой jpg/png рядом
    ApplyWrapMode();

    // ---- Матрицы ----
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WinWidth / WinHeight, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

    std::cout << "\n=== Управление ===\n"
        << "T          — вкл/выкл тайлинг (REPEAT / CLAMP)\n"
        << "+ / -      — изменить количество повторений текстуры\n"
        << "← / →      — вращение объекта вокруг оси Z\n"
        << "Esc        — выход\n\n";

    // ---- Главный цикл ----
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(program);

        // Матрицы
        glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform1f(glGetUniformLocation(program, "uRepeat"), texRepeat);

        // Модель (вращение вокруг Z)
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(rotZ), glm::vec3(0, 0, 1));
        glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, glm::value_ptr(model));

        // Текстура
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glUniform1i(glGetUniformLocation(program, "uTexture"), 0);

        // Рисуем
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteTextures(1, &textureID);
    glDeleteProgram(program);
    glfwTerminate();
    return 0;
}