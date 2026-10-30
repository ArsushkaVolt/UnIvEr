#include <iostream>
#include <vector>
#include <cmath>
#define _USE_MATH_DEFINES
#include <math.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// Глобальные переменные
int WinWidth = 800;
int WinHeight = 600;
int currentTask = 1;
GLuint programGouraud = 0;
GLuint programFlat = 0;

// -----------------------------------------------------------------------------
// ШЕЙДЕРЫ GLSL
// -----------------------------------------------------------------------------
const char* vsSourceSmooth = R"(
#version 400
in vec3 vertex_position;
in vec3 vertex_colour;
out vec3 colour;
void main() {
    colour = vertex_colour;
    gl_Position = vec4(vertex_position, 1.0);
}
)";
const char* fsSourceSmooth = R"(
#version 400
in vec3 colour;
out vec4 frag_colour;
void main() {
    frag_colour = vec4(colour, 1.0);
}
)";
const char* vsSourceFlat = R"(
#version 400
in vec3 vertex_position;
in vec3 vertex_colour;
flat out vec3 colour;
void main() {
    colour = vertex_colour;
    gl_Position = vec4(vertex_position, 1.0);
}
)";
const char* fsSourceFlat = R"(
#version 400
flat in vec3 colour;
out vec4 frag_colour;
void main() {
    frag_colour = vec4(colour, 1.0);
}
)";

// -----------------------------------------------------------------------------
// ФУНКЦИИ ГЕНЕРАЦИИ ГЕОМЕТРИИ (ВАРИАНТ 2)
// -----------------------------------------------------------------------------
void generateNGon(int n, float radius, std::vector<float>& pos, std::vector<float>& col) {
    pos.clear(); col.clear();
    for (int i = 0; i < n; ++i) {
        float angle = 2.0f * (float)M_PI * i / n;
        pos.push_back(radius * cos(angle));
        pos.push_back(radius * sin(angle));
        pos.push_back(0.0f);
        col.push_back((float)i / n);
        col.push_back(1.0f - (float)i / n);
        col.push_back(0.5f + 0.5f * sin(angle));
    }
}

void generateNGonFan(int n, float radius, std::vector<float>& pos, std::vector<float>& col) {
    pos.clear(); col.clear();
    pos.push_back(0.0f); pos.push_back(0.0f); pos.push_back(0.0f);
    col.push_back(1.0f); col.push_back(1.0f); col.push_back(1.0f);
    float sectorColors[7][3] = {
        {1.0f, 0.2f, 0.2f},
        {0.2f, 1.0f, 0.2f},
        {0.2f, 0.2f, 1.0f},
        {1.0f, 1.0f, 0.2f},
        {1.0f, 0.2f, 1.0f},
        {0.2f, 1.0f, 1.0f},
        {1.0f, 0.5f, 0.0f}
    };
    for (int i = 0; i <= n; ++i) {
        int idx = i % n;
        float angle = 2.0f * (float)M_PI * idx / n;
        pos.push_back(radius * cos(angle));
        pos.push_back(radius * sin(angle));
        pos.push_back(0.0f);
        col.push_back(sectorColors[idx][0]);
        col.push_back(sectorColors[idx][1]);
        col.push_back(sectorColors[idx][2]);
    }
}

void generateFigure1(std::vector<float>& pos, std::vector<float>& col) {
    pos = {
       -0.8f,  0.5f, 0.0f,
       -0.6f, -0.4f, 0.0f,
       -0.4f,  0.1f, 0.0f,
        0.0f,  0.1f, 0.0f,
       -0.1f,  0.6f, 0.0f,
        0.6f,  0.6f, 0.0f,
        0.0f, -0.5f, 0.0f
    };
    col = {
        1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 0.0f,  1.0f, 0.0f, 1.0f,  0.0f, 1.0f, 1.0f,
        1.0f, 0.5f, 0.0f
    };
}

// ===== ИСПРАВЛЕННАЯ ФИГУРА 2 (по картинке) =====
void generateFigure2(std::vector<float>& pos, std::vector<float>& col) {
    // Контур точно как на изображении
    pos = {
        -0.90f,  0.70f, 0.0f,   // 0 верхний левый острый
         0.90f,  0.70f, 0.0f,   // 1 верхний правый острый
         0.28f,  0.18f, 0.0f,   // 2 начало ступеньки
         0.28f, -0.18f, 0.0f,   // 3 внутренний верх ступеньки
         0.72f, -0.18f, 0.0f,   // 4 внешний верх ступеньки
         0.72f, -0.70f, 0.0f,   // 5 нижний правый ступеньки
        -0.90f, -0.70f, 0.0f,   // 6 нижний левый
        -0.28f, -0.02f, 0.0f    // 7 левый зубец
    };
    col = {
        1.0f, 0.2f, 0.2f,  0.2f, 1.0f, 0.2f,  0.2f, 0.2f, 1.0f,
        1.0f, 1.0f, 0.2f,  0.2f, 1.0f, 1.0f,  1.0f, 0.2f, 1.0f,
        0.5f, 0.5f, 0.5f,  1.0f, 0.5f, 0.0f
    };
}

void generateFig2Triangles(std::vector<float>& pos, std::vector<float>& col) {
    // 6 треугольников, полностью закрывающих фигуру без выхода наружу
    pos = {
        // 1. верх
        -0.90f,  0.70f, 0.0f,   0.90f,  0.70f, 0.0f,   0.28f,  0.18f, 0.0f,
        // 2. верх-лево
        -0.90f,  0.70f, 0.0f,   0.28f,  0.18f, 0.0f,  -0.28f, -0.02f, 0.0f,
        // 3. середина
         0.28f,  0.18f, 0.0f,   0.28f, -0.18f, 0.0f,  -0.28f, -0.02f, 0.0f,
         // 4. низ-лево
         -0.28f, -0.02f, 0.0f,   0.28f, -0.18f, 0.0f,  -0.90f, -0.70f, 0.0f,
         // 5. ступенька
          0.28f, -0.18f, 0.0f,   0.72f, -0.18f, 0.0f,   0.72f, -0.70f, 0.0f,
          // 6. низ
           0.28f, -0.18f, 0.0f,   0.72f, -0.70f, 0.0f,  -0.90f, -0.70f, 0.0f
    };
    col = {
        1.0f, 0.1f, 0.1f,  1.0f, 0.1f, 0.1f,  1.0f, 0.1f, 0.1f,
        0.1f, 0.8f, 0.1f,  0.1f, 0.8f, 0.1f,  0.1f, 0.8f, 0.1f,
        0.1f, 0.4f, 1.0f,  0.1f, 0.4f, 1.0f,  0.1f, 0.4f, 1.0f,
        1.0f, 0.9f, 0.1f,  1.0f, 0.9f, 0.1f,  1.0f, 0.9f, 0.1f,
        0.9f, 0.1f, 0.9f,  0.9f, 0.1f, 0.9f,  0.9f, 0.1f, 0.9f,
        0.1f, 0.9f, 0.9f,  0.1f, 0.9f, 0.9f,  0.1f, 0.9f, 0.9f
    };
}

void generateFig2Strip(std::vector<float>& pos, std::vector<float>& col) {
    // Порядок для TRIANGLE_STRIP
    pos = {
        -0.90f,  0.70f, 0.0f,   // A
         0.90f,  0.70f, 0.0f,   // B
        -0.28f, -0.02f, 0.0f,   // H
         0.28f,  0.18f, 0.0f,   // C
        -0.90f, -0.70f, 0.0f,   // G
         0.28f, -0.18f, 0.0f,   // D
         0.72f, -0.70f, 0.0f,   // F
         0.72f, -0.18f, 0.0f    // E
    };
    col = {
        1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 0.0f,  1.0f, 0.0f, 1.0f,  0.0f, 1.0f, 1.0f,
        1.0f, 0.5f, 0.0f,  0.5f, 0.0f, 1.0f
    };
}

void generateFig2Fan(std::vector<float>& pos, std::vector<float>& col) {
    // Центр + обход контура
    pos = {
         0.00f,  0.00f, 0.0f,   // центр
        -0.90f,  0.70f, 0.0f,   // A
         0.90f,  0.70f, 0.0f,   // B
         0.28f,  0.18f, 0.0f,   // C
         0.28f, -0.18f, 0.0f,   // D
         0.72f, -0.18f, 0.0f,   // E
         0.72f, -0.70f, 0.0f,   // F
        -0.90f, -0.70f, 0.0f,   // G
        -0.28f, -0.02f, 0.0f    // H
    };
    col = {
        1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f,  1.0f, 1.0f, 0.0f,
        1.0f, 0.0f, 1.0f,  0.0f, 1.0f, 1.0f,
        1.0f, 0.5f, 0.0f,  0.6f, 0.3f, 0.9f
    };
}

// Задание 7 и 8: Фигура 3
void generateFigure3Triangles(std::vector<float>& pos, std::vector<float>& col) {
    pos = {
        // Тр. 1 (Красный — верхний левый)
       -0.80f, -0.15f, 0.0f,  -0.30f,  0.90f, 0.0f,  -0.05f,  0.50f, 0.0f,
       // Тр. 2 (Зелёный — верхний правый)
      -0.30f,  0.90f, 0.0f,   0.85f,  0.80f, 0.0f,  -0.05f,  0.50f, 0.0f,
      // Тр. 3 (Синий — правый)
      0.85f,  0.80f, 0.0f,   0.95f, -0.25f, 0.0f,  -0.05f,  0.50f, 0.0f,
      // Тр. 4 (Жёлтый — нижний правый)
      0.95f, -0.25f, 0.0f,   0.35f, -0.85f, 0.0f,  -0.05f,  0.50f, 0.0f,
      // Тр. 5 (Пурпурный — средний левый)
     -0.80f, -0.15f, 0.0f,  -0.05f,  0.50f, 0.0f,  -0.35f, -0.30f, 0.0f,
     // Тр. 6 (Голубой)
    -0.80f, -0.15f, 0.0f,  -0.45f, -0.65f, 0.0f,   0.02f, -0.35f, 0.0f
    };
    col = {
        1.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f,
        0.0f, 0.8f, 0.0f,   0.0f, 0.8f, 0.0f,   0.0f, 0.8f, 0.0f,
        0.0f, 0.3f, 1.0f,   0.0f, 0.3f, 1.0f,   0.0f, 0.3f, 1.0f,
        1.0f, 0.9f, 0.0f,   1.0f, 0.9f, 0.0f,   1.0f, 0.9f, 0.0f,
        0.9f, 0.0f, 0.9f,   0.9f, 0.0f, 0.9f,   0.9f, 0.0f, 0.9f,
        0.0f, 0.9f, 0.9f,   0.0f, 0.9f, 0.9f,   0.0f, 0.9f, 0.9f
    };
}

// -----------------------------------------------------------------------------
// ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ OPENGL
// -----------------------------------------------------------------------------
GLuint createShaderProgram(const char* vsText, const char* fsText) {
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vsText, NULL);
    glCompileShader(vs);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fsText, NULL);
    glCompileShader(fs);
    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glBindAttribLocation(program, 0, "vertex_position");
    glBindAttribLocation(program, 1, "vertex_colour");
    glLinkProgram(program);
    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}

GLuint createVAO(const std::vector<float>& positions, const std::vector<float>& colors) {
    GLuint vao = 0, vboPos = 0, vboCol = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vboPos);
    glBindBuffer(GL_ARRAY_BUFFER, vboPos);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(float), positions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);
    glGenBuffers(1, &vboCol);
    glBindBuffer(GL_ARRAY_BUFFER, vboCol);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(float), colors.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
    return vao;
}

void glfw_window_size_callback(GLFWwindow* window, int width, int height) {
    WinWidth = width;
    WinHeight = height;
    glViewport(0, 0, WinWidth, WinHeight);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(window, 1);
        }
        if (key >= GLFW_KEY_1 && key <= GLFW_KEY_9) {
            currentTask = key - GLFW_KEY_0;
            std::cout << "Переключено на режим Задания № " << currentTask << std::endl;
        }
        if (key == GLFW_KEY_0) {
            currentTask = 10;
            std::cout << "Переключено на Задание № 7б (Flat Shading)" << std::endl;
        }
        if (key == GLFW_KEY_MINUS) {
            currentTask = 11;
            std::cout << "Переключено на Задание № 8 (Прозрачная каркасная сетка glPolygonMode)" << std::endl;
        }
    }
}

// -----------------------------------------------------------------------------
// MAIN
// -----------------------------------------------------------------------------
int main() {
    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(WinWidth, WinHeight, "Prakticheskaya rabota 2 - Variant 2", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glewInit();
    glfwSetWindowSizeCallback(window, glfw_window_size_callback);
    glfwSetKeyCallback(window, key_callback);

    programGouraud = createShaderProgram(vsSourceSmooth, fsSourceSmooth);
    programFlat = createShaderProgram(vsSourceFlat, fsSourceFlat);

    std::vector<float> ngonPos, ngonCol; generateNGon(7, 0.6f, ngonPos, ngonCol); GLuint vaoNGon = createVAO(ngonPos, ngonCol);
    std::vector<float> ngonFanPos, ngonFanCol; generateNGonFan(7, 0.6f, ngonFanPos, ngonFanCol); GLuint vaoNGonFan = createVAO(ngonFanPos, ngonFanCol);
    std::vector<float> fig1Pos, fig1Col; generateFigure1(fig1Pos, fig1Col);       GLuint vaoFig1 = createVAO(fig1Pos, fig1Col);
    std::vector<float> fig2Pos, fig2Col; generateFigure2(fig2Pos, fig2Col);       GLuint vaoFig2 = createVAO(fig2Pos, fig2Col);
    std::vector<float> f5aPos, f5aCol; generateFig2Triangles(f5aPos, f5aCol); GLuint vaoF5a = createVAO(f5aPos, f5aCol);
    std::vector<float> f5bPos, f5bCol; generateFig2Strip(f5bPos, f5bCol);       GLuint vaoF5b = createVAO(f5bPos, f5bCol);
    std::vector<float> f5cPos, f5cCol; generateFig2Fan(f5cPos, f5cCol);         GLuint vaoF5c = createVAO(f5cPos, f5cCol);
    std::vector<float> fig3Pos, fig3Col; generateFigure3Triangles(fig3Pos, fig3Col); GLuint vaoFig3 = createVAO(fig3Pos, fig3Col);

    glClearColor(0.12f, 0.12f, 0.12f, 1.0f);

    std::cout << "===== ИНСТРУКЦИЯ ПО ПЕРЕКЛЮЧЕНИЮ ЗАДАНИЙ =====\n"
        << " Нажмите [1]: Задание 1 (Точки 7-угольника)\n"
        << " Нажмите [2]: Задание 2 (Контур 7-угольника)\n"
        << " Нажмите [3]: Задание 3 (Фигура 1 - Ломаная линия)\n"
        << " Нажмите [4]: Задание 4 (Фигура 2 - Контур Сигма)\n"
        << " Нажмите [5]: Задание 5а (Фигура 2 - GL_TRIANGLES)\n"
        << " Нажмите [6]: Задание 5б (Фигура 2 - GL_TRIANGLE_STRIP)\n"
        << " Нажмите [7]: Задание 5в (Фигура 2 - GL_TRIANGLE_FAN)\n"
        << " Нажмите [8]: Задание 6 (Закрашенный 7-угольник веером GL_TRIANGLE_FAN)\n"
        << " Нажмите [9]: Задание 7а (Фигура 3 - Плавное тонирование / Smooth)\n"
        << " Нажмите [0]: Задание 7б (Фигура 3 - Плоское тонирование / Flat)\n"
        << " Нажмите [-]: Задание 8 (Фигура 3 - Прозрачная каркасная сетка glPolygonMode)\n"
        << " ESC: Выход из программы\n"
        << "===============================================" << std::endl;

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        switch (currentTask) {
        case 1:
            glUseProgram(programGouraud);
            glEnable(GL_POINT_SMOOTH);
            glEnable(GL_PROGRAM_POINT_SIZE);
            glPointSize(14.0f);
            glBindVertexArray(vaoNGon);
            glDrawArrays(GL_POINTS, 0, (GLsizei)(ngonPos.size() / 3));
            break;
        case 2:
            glUseProgram(programGouraud);
            glLineWidth(3.0f);
            glEnable(GL_LINE_SMOOTH);
            glBindVertexArray(vaoNGon);
            glDrawArrays(GL_LINE_LOOP, 0, (GLsizei)(ngonPos.size() / 3));
            break;
        case 3:
            glUseProgram(programGouraud);
            glLineWidth(3.0f);
            glBindVertexArray(vaoFig1);
            glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)(fig1Pos.size() / 3));
            break;
        case 4:
            glUseProgram(programGouraud);
            glLineWidth(3.0f);
            glBindVertexArray(vaoFig2);
            glDrawArrays(GL_LINE_LOOP, 0, (GLsizei)(fig2Pos.size() / 3));
            break;
        case 5:
            glUseProgram(programGouraud);
            glBindVertexArray(vaoF5a);
            glDrawArrays(GL_TRIANGLES, 0, (GLsizei)(f5aPos.size() / 3));
            break;
        case 6:
            glUseProgram(programFlat);
            glBindVertexArray(vaoF5b);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, (GLsizei)(f5bPos.size() / 3));
            break;
        case 7:
            glUseProgram(programFlat);
            glBindVertexArray(vaoF5c);
            glDrawArrays(GL_TRIANGLE_FAN, 0, (GLsizei)(f5cPos.size() / 3));
            break;
        case 8:
            glUseProgram(programFlat);
            glBindVertexArray(vaoNGonFan);
            glDrawArrays(GL_TRIANGLE_FAN, 0, (GLsizei)(ngonFanPos.size() / 3));
            break;
        case 9:
            glUseProgram(programGouraud);
            glBindVertexArray(vaoFig3);
            glDrawArrays(GL_TRIANGLES, 0, (GLsizei)(fig3Pos.size() / 3));
            break;
        case 10:
            glUseProgram(programFlat);
            glBindVertexArray(vaoFig3);
            glDrawArrays(GL_TRIANGLES, 0, (GLsizei)(fig3Pos.size() / 3));
            break;
        case 11:
            glUseProgram(programGouraud);
            glBindVertexArray(vaoFig3);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDrawArrays(GL_TRIANGLES, 0, (GLsizei)(fig3Pos.size() / 3));
            break;
        }
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    glfwTerminate();
    return 0;
}