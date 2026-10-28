#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>

int WinWidth = 800;
int WinHeight = 600;

// Переменные для Задания 6 (Масштаб куба)
float cubeScale = 1.0f;
int currentTask = 1; // Номер выполняемого задания (переключение на 1-6)

// Шейдеры
const char* vertexShaderSource = R"(
#version 400
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;

uniform mat4 M;
uniform mat4 P;

out vec3 fragColor;

void main() {
    fragColor = aColor;
    gl_Position = P * M * vec4(aPos, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 400
in vec3 fragColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(fragColor, 1.0);
}
)";

// Обработка клавиш (Задание 6 и переключение режимов)
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(window, true);
        
        // Клавиши 1-6 для выбора задания
        if (key == GLFW_KEY_1) currentTask = 1;
        if (key == GLFW_KEY_2) currentTask = 2;
        if (key == GLFW_KEY_3) currentTask = 3;
        if (key == GLFW_KEY_4) currentTask = 4;
        if (key == GLFW_KEY_5) currentTask = 5;
        if (key == GLFW_KEY_6) currentTask = 6;

        // Задание 6: управление масштабом куба клавишами UP / DOWN
        if (key == GLFW_KEY_UP) cubeScale += 0.1f;
        if (key == GLFW_KEY_DOWN) {
            cubeScale -= 0.1f;
            if (cubeScale < 0.1f) cubeScale = 0.1f;
        }
    }
}

GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    return shader;
}

GLuint createShaderProgram() {
    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    return program;
}

int main() {
    if (!glfwInit()) return -1;

    GLFWwindow* window = glfwCreateWindow(WinWidth, WinHeight, "Practical 4 - Variant 2", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    glewInit();

    GLuint shaderProgram = createShaderProgram();

    // -------------------------------------------------------------
    // Данные для Заданий 1 и 2 (Плоские фигуры в пространстве)
    // -------------------------------------------------------------
    std::vector<float> shapesVertices = {
        // Треугольник 1 (Красный)
         0.0f,  0.0f,  0.3f,   1.0f, 0.0f, 0.0f,
         0.4f,  0.0f,  0.5f,   1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, -1.0f,   1.0f, 0.0f, 0.0f,

        // Четырехугольник -> 2 треугольника (Зеленый)
        -0.5f, -0.5f, -0.5f,   0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 0.0f,
         0.8f,  0.5f, -0.5f,   0.0f, 1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,   0.0f, 1.0f, 0.0f,
         0.8f,  0.5f, -0.5f,   0.0f, 1.0f, 0.0f,
         0.5f, -0.8f, -0.5f,   0.0f, 1.0f, 0.0f,

        // Треугольник 2 (Синий)
        -1.0f, -1.0f,  1.0f,   0.0f, 0.0f, 1.0f,
        -1.0f,  1.0f,  1.0f,   0.0f, 0.0f, 1.0f,
         1.0f,  0.0f, -1.0f,   0.0f, 0.0f, 1.0f
    };

    GLuint vaoShapes, vboShapes;
    glGenVertexArrays(1, &vaoShapes);
    glGenBuffers(1, &vboShapes);
    glBindVertexArray(vaoShapes);
    glBindBuffer(GL_ARRAY_BUFFER, vboShapes);
    glBufferData(GL_ARRAY_BUFFER, shapesVertices.size() * sizeof(float), shapesVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // -------------------------------------------------------------
    // Данные куба (Задания 3, 4, 5, 6) — 12 треугольников, каждая грань со своим цветом
    // -------------------------------------------------------------
    std::vector<float> cubeVertices = {
        // Передняя грань (Красная)
        -1.0f,-1.0f, 1.0f,  1,0,0,   1.0f,-1.0f, 1.0f,  1,0,0,   1.0f, 1.0f, 1.0f,  1,0,0,
        -1.0f,-1.0f, 1.0f,  1,0,0,   1.0f, 1.0f, 1.0f,  1,0,0,  -1.0f, 1.0f, 1.0f,  1,0,0,
        // Задняя грань (Зеленая)
        -1.0f,-1.0f,-1.0f,  0,1,0,   1.0f, 1.0f,-1.0f,  0,1,0,   1.0f,-1.0f,-1.0f,  0,1,0,
        -1.0f,-1.0f,-1.0f,  0,1,0,  -1.0f, 1.0f,-1.0f,  0,1,0,   1.0f, 1.0f,-1.0f,  0,1,0,
        // Верхняя грань (Синяя)
        -1.0f, 1.0f,-1.0f,  0,0,1,  -1.0f, 1.0f, 1.0f,  0,0,1,   1.0f, 1.0f, 1.0f,  0,0,1,
        -1.0f, 1.0f,-1.0f,  0,0,1,   1.0f, 1.0f, 1.0f,  0,0,1,   1.0f, 1.0f,-1.0f,  0,0,1,
        // Нижняя грань (Желтая)
        -1.0f,-1.0f,-1.0f,  1,1,0,   1.0f,-1.0f, 1.0f,  1,1,0,  -1.0f,-1.0f, 1.0f,  1,1,0,
        -1.0f,-1.0f,-1.0f,  1,1,0,   1.0f,-1.0f,-1.0f,  1,1,0,   1.0f,-1.0f, 1.0f,  1,1,0,
        // Правая грань (Фиолетовая)
         1.0f,-1.0f,-1.0f,  1,0,1,   1.0f, 1.0f, 1.0f,  1,0,1,   1.0f,-1.0f, 1.0f,  1,0,1,
         1.0f,-1.0f,-1.0f,  1,0,1,   1.0f, 1.0f,-1.0f,  1,0,1,   1.0f, 1.0f, 1.0f,  1,0,1,
        // Левая грань (Голубая)
        -1.0f,-1.0f,-1.0f,  0,1,1,  -1.0f,-1.0f, 1.0f,  0,1,1,  -1.0f, 1.0f, 1.0f,  0,1,1,
        -1.0f,-1.0f,-1.0f,  0,1,1,  -1.0f, 1.0f, 1.0f,  0,1,1,  -1.0f, 1.0f,-1.0f,  0,1,1
    };

    GLuint vaoCube, vboCube;
    glGenVertexArrays(1, &vaoCube);
    glGenBuffers(1, &vboCube);
    glBindVertexArray(vaoCube);
    glBindBuffer(GL_ARRAY_BUFFER, vboCube);
    glBufferData(GL_ARRAY_BUFFER, cubeVertices.size() * sizeof(float), cubeVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glUseProgram(shaderProgram);
    GLuint uM = glGetUniformLocation(shaderProgram, "M");
    GLuint uP = glGetUniformLocation(shaderProgram, "P");

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 M = glm::mat4(1.0f);
        glm::mat4 P = glm::mat4(1.0f);

        switch (currentTask) {
        case 1:
            // Задание 1: Фигуры БЕЗ буфера глубины[cite: 55]
            glDisable(GL_DEPTH_TEST);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            P = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, -2.0f, 2.0f);
            glUniformMatrix4fv(uM, 1, GL_FALSE, &M[0][0]);
            glUniformMatrix4fv(uP, 1, GL_FALSE, &P[0][0]);
            glBindVertexArray(vaoShapes);
            glDrawArrays(GL_TRIANGLES, 0, 12);
            break;

        case 2:
            // Задание 2: Фигуры С буфером глубины[cite: 52, 55]
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            P = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, -2.0f, 2.0f);
            glUniformMatrix4fv(uM, 1, GL_FALSE, &M[0][0]);
            glUniformMatrix4fv(uP, 1, GL_FALSE, &P[0][0]);
            glBindVertexArray(vaoShapes);
            glDrawArrays(GL_TRIANGLES, 0, 12);
            break;

        case 3:
            // Задание 3: Параллельная проекция и параметры Варианта 2[cite: 54, 55, 56]
            glEnable(GL_DEPTH_TEST);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            // Видовые параметры: left=-0.9, right=0.8, bottom=-0.8, top=0.9, near=5, far=10[cite: 54, 56]
            P = glm::ortho(-0.9f, 0.8f, -0.8f, 0.9f, 5.0f, 10.0f);
            M = glm::translate(M, glm::vec3(0.0f, 0.0f, -7.5f));// Сдвиг куба в зону видимости
            M = glm::scale(M, glm::vec3(0.3f)); 
            glUniformMatrix4fv(uM, 1, GL_FALSE, &M[0][0]);
            glUniformMatrix4fv(uP, 1, GL_FALSE, &P[0][0]);
            glBindVertexArray(vaoCube);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            break;

        case 4:
            // Задание 4: Каркасная перспективная проекция[cite: 38, 53, 56]
            glEnable(GL_DEPTH_TEST);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Каркас[cite: 38]
            P = glm::frustum(-0.9f, 0.8f, -0.8f, 0.9f, 5.0f, 10.0f); // Перспективная проекция[cite: 55, 56]
            M = glm::translate(M, glm::vec3(0.0f, 0.0f, -7.5f));
            glUniformMatrix4fv(uM, 1, GL_FALSE, &M[0][0]);
            glUniformMatrix4fv(uP, 1, GL_FALSE, &P[0][0]);
            glBindVertexArray(vaoCube);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            break;

        case 5:
            // Задание 5: Поворот осей OX: 45°, OY: 45°[cite: 46, 56]
            glEnable(GL_DEPTH_TEST);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            // Расширяем видовые параметры (ortho), чтобы куб не отсекался при повороте[cite: 56]
            P = glm::ortho(-2.5f, 2.5f, -2.5f, 2.5f, 1.0f, 15.0f);
            M = glm::translate(M, glm::vec3(0.0f, 0.0f, -7.5f));
            M = glm::rotate(M, glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Поворот OX[cite: 46, 56]
            M = glm::rotate(M, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Поворот OY[cite: 46, 56]
            glUniformMatrix4fv(uM, 1, GL_FALSE, &M[0][0]);
            glUniformMatrix4fv(uP, 1, GL_FALSE, &P[0][0]);
            glBindVertexArray(vaoCube);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            break;

        case 6:
            // Задание 6: Масштабирование объекта клавишами UP/DOWN[cite: 47, 56]
            glEnable(GL_DEPTH_TEST);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            P = glm::ortho(-3.0f, 3.0f, -3.0f, 3.0f, 1.0f, 15.0f);
            M = glm::translate(M, glm::vec3(0.0f, 0.0f, -7.5f));
            M = glm::rotate(M, glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            M = glm::rotate(M, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            M = glm::scale(M, glm::vec3(cubeScale)); // Изменение масштаба[cite: 47]
            glUniformMatrix4fv(uM, 1, GL_FALSE, &M[0][0]);
            glUniformMatrix4fv(uP, 1, GL_FALSE, &P[0][0]);
            glBindVertexArray(vaoCube);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            break;
        }

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}