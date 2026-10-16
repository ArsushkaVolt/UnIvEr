#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>
#include <cmath>

using namespace std;

// Глобальные переменные размеров окна и текущего задания
int WinWidth = 1024;
int WinHeight = 768;
int current_task = 3; // По умолчанию сразу откроется Задание 3 с новой фигурой


// ШЕЙДЕРЫ 

const char* vertex_shader_text =
"#version 400\n"
"in vec3 vertex_position;\n"
"in vec3 vertex_color;\n"
"uniform mat4 MVP;\n"
"out vec3 color;\n"
"void main() {\n"
"    color = vertex_color;\n"
"    gl_Position = MVP * vec4(vertex_position, 1.0);\n"
"}\n";

const char* fragment_shader_text =
"#version 400\n"
"in vec3 color;\n"
"out vec4 frag_color;\n"
"void main() {\n"
"    frag_color = vec4(color, 1.0);\n"
"}\n";


// ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ И ИНИЦИАЛИЗАЦИЯ

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, 1);
    }
    if (action == GLFW_PRESS)
    {
        if (key == GLFW_KEY_1) { current_task = 1; cout << "Active: Zadanie 1 (Pravilnyy 7-ugolnik)" << endl; }
        if (key == GLFW_KEY_2) { current_task = 2; cout << "Active: Zadanie 2 (Affinnie preobrazovaniya)" << endl; }
        if (key == GLFW_KEY_3) { current_task = 3; cout << "Active: Zadanie 3 (Figura iz treh treugolnikov)" << endl; }
    }
}

void window_size_callback(GLFWwindow* window, int width, int height)
{
    WinWidth = width;
    WinHeight = height;
}

GLFWwindow* InitAll(int w, int h)
{
    if (!glfwInit())
    {
        cerr << "ERROR: could not start GLFW3\n";
        return nullptr;
    }
    GLFWwindow* window = glfwCreateWindow(w, h, "Prakticheskaya rabota 3 - Variant 2", NULL, NULL);
    if (!window)
    {
        cerr << "ERROR: could not open window with GLFW3\n";
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window);
    glfwSetWindowSizeCallback(window, window_size_callback);
    glfwSetKeyCallback(window, key_callback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        cerr << "ERROR: could not start GLEW\n";
        return nullptr;
    }
    return window;
}

void EndAll()
{
    glfwTerminate();
}


// КЛАСС МОДЕЛИ

class Model
{
public:
    Model(GLFWwindow* w) {
        glGenVertexArrays(1, &vao);
        window = w;
    }
    ~Model() {}

    void render(glm::mat4 MVPMatrix, GLuint mode = GL_TRIANGLES)
    {
        glUseProgram(shader_programme);
        GLuint MVP = glGetUniformLocation(shader_programme, "MVP");
        glUniformMatrix4fv(MVP, 1, GL_FALSE, &MVPMatrix[0][0]);

        glBindVertexArray(vao);
        if (indices_count > 0)
            glDrawElements(mode, indices_count, GL_UNSIGNED_INT, 0);
        else
            glDrawArrays(mode, 0, verteces_count);
    }

    void load_coords(glm::vec3* verteces, size_t count)
    {
        verteces_count = count;
        GLuint coords_vbo = 0;
        glGenBuffers(1, &coords_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, coords_vbo);
        glBufferData(GL_ARRAY_BUFFER, count * sizeof(glm::vec3), verteces, GL_STATIC_DRAW);

        glBindVertexArray(vao);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(0);
    }

    void load_colors(glm::vec3* colors, size_t count)
    {
        GLuint colors_vbo = 0;
        glGenBuffers(1, &colors_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, colors_vbo);
        glBufferData(GL_ARRAY_BUFFER, count * sizeof(glm::vec3), colors, GL_STATIC_DRAW);

        glBindVertexArray(vao);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(1);
    }

    void load_indices(GLuint* indices, size_t count)
    {
        indices_count = count;
        glBindVertexArray(vao);
        GLuint elementbuffer;
        glGenBuffers(1, &elementbuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(GLuint), indices, GL_STATIC_DRAW);
    }

    void load_shaders_from_memory()
    {
        shader_programme = glCreateProgram();

        GLuint vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, &vertex_shader_text, NULL);
        glCompileShader(vs);

        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, &fragment_shader_text, NULL);
        glCompileShader(fs);

        glAttachShader(shader_programme, vs);
        glAttachShader(shader_programme, fs);

        glBindAttribLocation(shader_programme, 0, "vertex_position");
        glBindAttribLocation(shader_programme, 1, "vertex_color");

        glLinkProgram(shader_programme);
    }

private:
    GLuint vao = -1;
    size_t verteces_count = 0;
    size_t indices_count = 0;
    GLuint shader_programme = -1;
    GLFWwindow* window;
};


//  (MAIN)

int main() {
    GLFWwindow* window = InitAll(WinWidth, WinHeight);
    if (window == nullptr) {
        EndAll();
        return -1;
    }

    cout << "=== VARIANT 2 PRACTICAL WORK 3 ===" << endl;
    cout << "Use keys [1], [2], [3] to switch between tasks." << endl;

   
    // ДАННЫЕ ДЛЯ ЗАДАНИЯ 1: Правильный 7-угольник (N = 7)
  
    int N = 7;
    vector<glm::vec3> hept_points;
    vector<glm::vec3> hept_colors;
    float radius = 0.5f;
    for (int i = 0; i < N; ++i) {
        float angle = 2.0f * 3.14159265f * i / N;
        hept_points.push_back(glm::vec3(radius * cos(angle), radius * sin(angle), 0.0f));
        hept_colors.push_back(glm::vec3((float)i / N, 1.0f - (float)i / N, 0.5f));
    }
    Model modelHept(window);
    modelHept.load_coords(hept_points.data(), hept_points.size());
    modelHept.load_colors(hept_colors.data(), hept_colors.size());

    vector<GLuint> hept_loop_indices;
    for (GLuint i = 0; i < N; ++i) hept_loop_indices.push_back(i);
    modelHept.load_indices(hept_loop_indices.data(), hept_loop_indices.size());
    modelHept.load_shaders_from_memory();

  
    // ДАННЫЕ ДЛЯ ЗАДАНИЯ 2: Преобразования по Варианту 2
    
    glm::vec3 tri_pts[] = { {0.0f, 0.2f, 0.0f}, {-0.2f, -0.2f, 0.0f}, {0.2f, -0.2f, 0.0f} };
    glm::vec3 tri_cols[] = { {1,0,0}, {0,1,0}, {0,0,1} };
    GLuint tri_ind[] = { 0, 1, 2 };
    Model modelTri(window);
    modelTri.load_coords(tri_pts, 3);
    modelTri.load_colors(tri_cols, 3);
    modelTri.load_indices(tri_ind, 3);
    modelTri.load_shaders_from_memory();

    glm::vec3 line_pts[] = { {-0.4f, 0.0f, 0.0f}, {0.4f, 0.0f, 0.0f} };
    glm::vec3 line_cols[] = { {1,1,0}, {1,0,1} };
    Model modelLine(window);
    modelLine.load_coords(line_pts, 2);
    modelLine.load_colors(line_cols, 2);
    modelLine.load_shaders_from_memory();

    glm::vec3 rect_pts[] = { {-0.2f, -0.1f, 0.0f}, {0.2f, -0.1f, 0.0f}, {0.2f, 0.1f, 0.0f}, {-0.2f, 0.1f, 0.0f} };
    glm::vec3 rect_cols[] = { {0,1,1}, {1,0.5f,0}, {0.5f,0,0.5f}, {0.2f,0.8f,0.2f} };
    GLuint rect_ind[] = { 0, 1, 2, 2, 3, 0 };
    Model modelRect(window);
    modelRect.load_coords(rect_pts, 4);
    modelRect.load_colors(rect_cols, 4);
    modelRect.load_indices(rect_ind, 6);
    modelRect.load_shaders_from_memory();

    
    // ДАННЫЕ ДЛЯ ЗАДАНИЯ 3: Фигура из трёх треугольников 
 
    glm::vec3 three_tri_pts[] = {
        // Треугольник 1 (Верхний)
        { 0.0f,  0.0f, 0.0f}, // Центр
        {-0.3f,  0.5f, 0.0f},
        { 0.3f,  0.5f, 0.0f},

        // Треугольник 2 (Левый нижний)
        { 0.0f,  0.0f, 0.0f}, // Центр
        {-0.5f, -0.3f, 0.0f},
        {-0.2f, -0.8f, 0.0f},

        // Треугольник 3 (Правый нижний)
        { 0.0f,  0.0f, 0.0f}, // Центр
        { 0.2f, -0.8f, 0.0f},
        { 0.5f, -0.3f, 0.0f}
    };
    glm::vec3 three_tri_cols[] = {
        {1,1,1}, {1,0.5f,0}, {1,0.5f,0},
        {1,1,1}, {0,1,0}, {0,1,0},
        {1,1,1}, {0.2f,0.6f,1}, {0.2f,0.6f,1}
    };
    GLuint three_tri_ind[] = {
        0, 1, 2,  // Верхний треугольник
        3, 4, 5,  // Левый нижний треугольник
        6, 7, 8   // Правый нижний треугольник
    };
    Model modelThreeTri(window);
    modelThreeTri.load_coords(three_tri_pts, 9);
    modelThreeTri.load_colors(three_tri_cols, 9);
    modelThreeTri.load_indices(three_tri_ind, 9);
    modelThreeTri.load_shaders_from_memory();

    // Настройка параметров рендеринга OpenGL
    glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    // Главный цикл отрисовки
    while (!glfwWindowShouldClose(window))
    {
        glViewport(0, 0, WinWidth, WinHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 identity(1.0f);

        if (current_task == 1)
        {
            // ЗАДАНИЕ 1: Вывод правильного 7-угольника
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glLineWidth(2.5f);
            modelHept.render(identity, GL_LINE_LOOP);
        }
        else if (current_task == 2)
        {
            // ЗАДАНИЕ 2: Аффинные преобразования (Вариант 2)
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            // 1. Треугольник: Масштабирование и сдвиг
            glm::mat4 mTri = glm::translate(identity, glm::vec3(-0.5f, 0.5f, 0.0f));
            mTri = glm::scale(mTri, glm::vec3(1.5f, 0.5f, 1.0f));
            modelTri.render(mTri, GL_TRIANGLES);

            // 2. Линия: Поворот на угол alpha = -70 градусов
            glm::mat4 mLine = glm::rotate(identity, glm::radians(-70.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            glLineWidth(3.0f);
            modelLine.render(mLine, GL_LINES);

            // 3. Прямоугольник: Поворот на угол beta = 45 градусов вокруг заданной точки
            glm::vec3 pivot(0.3f, -0.3f, 0.0f);
            glm::mat4 mRect = glm::translate(identity, pivot);
            mRect = glm::rotate(mRect, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            mRect = glm::translate(mRect, -pivot);
            modelRect.render(mRect, GL_TRIANGLES);
        }
        else if (current_task == 3)
        {
            // ЗАДАНИЕ 3: Фигура из трех треугольников (как на вашем фото)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Каркасный режим для контуров
            glLineWidth(2.5f);
            glm::mat4 mThreeTri = glm::scale(identity, glm::vec3(1.0f, 1.0f, 1.0f));
            modelThreeTri.render(mThreeTri, GL_TRIANGLES);
        }

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    EndAll();
    return 0;
}