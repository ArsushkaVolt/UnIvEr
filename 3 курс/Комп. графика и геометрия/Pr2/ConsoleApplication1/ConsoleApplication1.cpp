#include <iostream>
#include <vector>
#include <cmath>

#define _USE_MATH_DEFINES
#include <math.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>


// Глобальные переменные
// 
int WinWidth = 800;   // Ширина окна
int WinHeight = 600;  // Высота окна
int currentTask = 1;  // Номер активного задания (1-8)

GLuint programGouraud = 0; // Идентификатор шейдера с плавным тонированием
GLuint programFlat = 0;    // Идентификатор шейдера с плоским (Flat) тонированием


// Шейдеры GLSL


// Вершинный шейдер (Smooth / Gouraud)[cite: 29]
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

// Фрагментный шейдер (Smooth / Gouraud)[cite: 30]
const char* fsSourceSmooth = R"(
#version 400
in vec3 colour;
out vec4 frag_colour;
void main() {
    frag_colour = vec4(colour, 1.0);
}
)";

// Вершинный шейдер (Flat shading)
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

// Фрагментный шейдер (Flat shading)[cite: 37]
const char* fsSourceFlat = R"(
#version 400
flat in vec3 colour;
out vec4 frag_colour;
void main() {
    frag_colour = vec4(colour, 1.0);
}
)";


// Вспомогательные функции OpenGL


// Компиляция и сборка шейдерной программы[cite: 13, 14, 30]
GLuint createShaderProgram(const char* vsText, const char* fsText) {
    GLuint vs = glCreateShader(GL_VERTEX_SHADER); //[cite: 13]
    glShaderSource(vs, 1, &vsText, NULL); //[cite: 13]
    glCompileShader(vs); //[cite: 13]

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER); //[cite: 14]
    glShaderSource(fs, 1, &fsText, NULL); //[cite: 14]
    glCompileShader(fs); //[cite: 14]

    GLuint program = glCreateProgram(); //[cite: 14]
    glAttachShader(program, vs); //[cite: 14]
    glAttachShader(program, fs); //[cite: 14]
    glBindAttribLocation(program, 0, "vertex_position"); //[cite: 30]
    glBindAttribLocation(program, 1, "vertex_colour"); //[cite: 30]
    glLinkProgram(program); //[cite: 14]

    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}

// Создание VAO и VBO для координат и цветов[cite: 11, 12, 28, 29]
GLuint createVAO(const std::vector<float>& positions, const std::vector<float>& colors) {
    GLuint vao = 0, vboPos = 0, vboCol = 0;
    glGenVertexArrays(1, &vao); //
    glBindVertexArray(vao); //[cite: 12]

    // VBO Координат[cite: 11]
    glGenBuffers(1, &vboPos); //[cite: 11]
    glBindBuffer(GL_ARRAY_BUFFER, vboPos); //[cite: 11]
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(float), positions.data(), GL_STATIC_DRAW); //[cite: 11]
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL); //[cite: 12, 29]
    glEnableVertexAttribArray(0); //[cite: 12, 29]

    // VBO Цветов[cite: 28]
    glGenBuffers(1, &vboCol); //[cite: 28]
    glBindBuffer(GL_ARRAY_BUFFER, vboCol); //[cite: 28]
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(float), colors.data(), GL_STATIC_DRAW); //[cite: 28]
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL); //[cite: 29]
    glEnableVertexAttribArray(1); //[cite: 29]

    glBindVertexArray(0);
    return vao;
}


// Обработчики событий GLFW
// 

void glfw_window_size_callback(GLFWwindow* window, int width, int height) {
    WinWidth = width;
    WinHeight = height;
    glViewport(0, 0, WinWidth, WinHeight); //[cite: 17]
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(window, 1); //[cite: 17, 18]
        }
        if (key >= GLFW_KEY_1 && key <= GLFW_KEY_8) {
            currentTask = key - GLFW_KEY_0;
            std::cout << "Переключение на Задание № " << currentTask << std::endl;
        }
    }
}

// 
// Генерация геометрии
// 

// Задание 1, 2, 6: Генерация N-угольника (N = 7 для Варианта 2)[cite: 39, 40]
void generateNGon(int n, float radius, std::vector<float>& pos, std::vector<float>& col) {
    pos.clear(); col.clear();
    for (int i = 0; i < n; ++i) {
        float angle = 2.0f * M_PI * i / n;
        pos.push_back(radius * cos(angle));
        pos.push_back(radius * sin(angle));
        pos.push_back(0.0f);

        col.push_back((float)i / n);
        col.push_back(1.0f - (float)i / n);
        col.push_back(0.5f + 0.5f * sin(angle));
    }
}

// Задание 3: Фигура 1 (Разомкнутая ломаная)[cite: 39, 40]
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
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 0.0f,
        1.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 1.0f,
        1.0f, 0.5f, 0.0f
    };
}

// Задание 4: Фигура 2 (Замкнутая ломаная "Сигма")[cite: 39, 40]
void generateFigure2(std::vector<float>& pos, std::vector<float>& col) {
    pos = {
       -0.7f,  0.6f, 0.0f,
        0.5f,  0.6f, 0.0f,
        0.1f,  0.3f, 0.0f,
        0.1f, -0.2f, 0.0f,
        0.5f, -0.2f, 0.0f,
        0.5f, -0.6f, 0.0f,
       -0.5f, -0.6f, 0.0f,
       -0.1f, -0.1f, 0.0f
    };
    col = {
        1.0f, 0.2f, 0.2f,
        0.2f, 1.0f, 0.2f,
        0.2f, 0.2f, 1.0f,
        1.0f, 1.0f, 0.2f,
        0.2f, 1.0f, 1.0f,
        1.0f, 0.2f, 1.0f,
        0.5f, 0.5f, 0.5f,
        1.0f, 0.5f, 0.0f
    };
}

//   (Задания 5, 7, 8)
void generateFigure3Triangles(std::vector<float>& pos, std::vector<float>& col) {
    // Точные координаты 8 вершин внешнего и внутреннего контура фигуры:
    // V0(-0.8, -0.15) — левый внешний угол
    // V1(-0.25, 0.85) — верхний левый угол
    // V2(0.75,  0.78) — верхний правый угол
    // V3(0.88, -0.15) — правый угол
    // V4(0.30, -0.78) — правый нижний выем
    // V5(-0.06, 0.48) — центральный верхний выем
    // V6(0.08, -0.48) — центральный нижний выем
    // V7(-0.45, -0.30) — внутренний левый изгиб
    // V8(-0.30, -0.78) — левый нижний угол

    pos = {
        // Треугольник 1: (V0, V1, V5)
       -0.80f, -0.15f, 0.0f,  -0.25f,  0.85f, 0.0f,  -0.06f,  0.48f, 0.0f,
       // Треугольник 2: (V1, V2, V5)
      -0.25f,  0.85f, 0.0f,   0.75f,  0.78f, 0.0f,  -0.06f,  0.48f, 0.0f,
      // Треугольник 3: (V2, V3, V5)
      0.75f,  0.78f, 0.0f,   0.88f, -0.15f, 0.0f,  -0.06f,  0.48f, 0.0f,
      // Треугольник 4: (V3, V4, V5)
      0.88f, -0.15f, 0.0f,   0.30f, -0.78f, 0.0f,  -0.06f,  0.48f, 0.0f,
      // Треугольник 5: (V0, V5, V7)
     -0.80f, -0.15f, 0.0f,  -0.06f,  0.48f, 0.0f,  -0.45f, -0.30f, 0.0f,
     // Треугольник 6: (V7, V6, V8)
    -0.45f, -0.30f, 0.0f,   0.08f, -0.48f, 0.0f,  -0.30f, -0.78f, 0.0f
    };

    // Отдельный цвет для каждого из 6 треугольников
    col = {
        1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f, // 1: Красный
        0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f, // 2: Зеленый
        0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f, // 3: Синий
        1.0f, 1.0f, 0.0f,  1.0f, 1.0f, 0.0f,  1.0f, 1.0f, 0.0f, // 4: Желтый
        1.0f, 0.0f, 1.0f,  1.0f, 0.0f, 1.0f,  1.0f, 0.0f, 1.0f, // 5: Пурпурный
        0.0f, 1.0f, 1.0f,  0.0f, 1.0f, 1.0f,  0.0f, 1.0f, 1.0f  // 6: Голубой
    };
}
// 
// Главная функция
// 
int main() {
    if (!glfwInit()) return -1; //[cite: 10]

    GLFWwindow* window = glfwCreateWindow(WinWidth, WinHeight, "Практическая работа № 2 - Вариант 2", NULL, NULL); //[cite: 10]
    if (!window) {
        glfwTerminate(); //[cite: 15]
        return -1;
    }

    glfwMakeContextCurrent(window); //[cite: 10]
    glewInit(); //[cite: 10]

    glfwSetWindowSizeCallback(window, glfw_window_size_callback); //[cite: 17]
    glfwSetKeyCallback(window, key_callback); //[cite: 18]

    programGouraud = createShaderProgram(vsSourceSmooth, fsSourceSmooth);
    programFlat = createShaderProgram(vsSourceFlat, fsSourceFlat);

    std::vector<float> ngonPos, ngonCol;
    generateNGon(7, 0.6f, ngonPos, ngonCol); // 7-угольник для Варианта 2[cite: 40]
    GLuint vaoNGon = createVAO(ngonPos, ngonCol);

    std::vector<float> fig1Pos, fig1Col;
    generateFigure1(fig1Pos, fig1Col);
    GLuint vaoFig1 = createVAO(fig1Pos, fig1Col);

    std::vector<float> fig2Pos, fig2Col;
    generateFigure2(fig2Pos, fig2Col);
    GLuint vaoFig2 = createVAO(fig2Pos, fig2Col);

    std::vector<float> fig3Pos, fig3Col;
    generateFigure3Triangles(fig3Pos, fig3Col);
    GLuint vaoFig3 = createVAO(fig3Pos, fig3Col);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f); //[cite: 14]

    while (!glfwWindowShouldClose(window)) { //[cite: 15]
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //[cite: 15]
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //[cite: 38]

        switch (currentTask) {
        case 1: { // Задание 1: Точки правильного 7-угольника[cite: 31, 32, 39]
            glUseProgram(programGouraud); //[cite: 14]
            glEnable(GL_POINT_SMOOTH); // Сглаживание точек[cite: 32]
            glEnable(GL_PROGRAM_POINT_SIZE); //[cite: 32]
            glPointSize(16.0f); // Размер точек[cite: 31]
            glBindVertexArray(vaoNGon); //[cite: 14]
            glDrawArrays(GL_POINTS, 0, 7); //[cite: 31]
            break;
        }
        case 2: { // Задание 2: Правильный 7-угольник линиями[cite: 33, 39]
            glUseProgram(programGouraud); //[cite: 14]
            glLineWidth(3.0f); // Толщина линий
            glEnable(GL_LINE_SMOOTH); // Сглаживание линий
            glBindVertexArray(vaoNGon); //[cite: 14]
            glDrawArrays(GL_LINE_LOOP, 0, 7); //
            break;
        }
        case 3: { // Задание 3: Фигура 1 — Разомкнутая ломаная[cite: 33, 39]
            glUseProgram(programGouraud); //[cite: 14]
            glLineWidth(3.0f); //[cite: 33]
            glBindVertexArray(vaoFig1); //[cite: 14]
            glDrawArrays(GL_LINE_STRIP, 0, fig1Pos.size() / 3); //[cite: 33]
            break;
        }
        case 4: { // Задание 4: Фигура 2 — Замкнутая ломаная[cite: 34, 39]
            glUseProgram(programGouraud); //[cite: 14]
            glLineWidth(3.0f); //[cite: 33]
            glBindVertexArray(vaoFig2); //[cite: 14]
            glDrawArrays(GL_LINE_LOOP, 0, fig2Pos.size() / 3); //
            break;
        }
        case 5: { // Задание 5: Фигура 2 — Разбиение на треугольники[cite: 34, 35, 39]
            glUseProgram(programGouraud); //[cite: 14]
            glBindVertexArray(vaoFig2); //[cite: 14]
            glDrawArrays(GL_TRIANGLE_FAN, 0, fig2Pos.size() / 3); //
            break;
        }
        case 6: { // Задание 6: Правильный 7-угольник веером треугольников[cite: 36, 39]
            glUseProgram(programGouraud); //[cite: 14]
            glBindVertexArray(vaoNGon); //[cite: 14]
            glDrawArrays(GL_TRIANGLE_FAN, 0, 7); //
            break;
        }
        case 7: { // Задание 7: Фигура 3 — Различные способы тонирования (Flat Shading)[cite: 37, 39]
            glUseProgram(programFlat); //[cite: 14, 37]
            glBindVertexArray(vaoFig3); //[cite: 14]
            glDrawArrays(GL_TRIANGLES, 0, fig3Pos.size() / 3); //[cite: 34]
            break;
        }
        case 8: { // Задание 8: Режимы отображения граней (glPolygonMode)[cite: 38, 39]
            glUseProgram(programGouraud); //[cite: 14]
            glBindVertexArray(vaoFig3); //[cite: 14]
            glPolygonMode(GL_FRONT, GL_FILL); //[cite: 38]
            glPolygonMode(GL_BACK, GL_LINE);  //[cite: 38]
            glDrawArrays(GL_TRIANGLES, 0, fig3Pos.size() / 3); //[cite: 34]
            break;
        }
        }

        glfwPollEvents(); //[cite: 15]
        glfwSwapBuffers(window); //[cite: 15]
    }

    glfwTerminate(); //[cite: 15]
    return 0;
}