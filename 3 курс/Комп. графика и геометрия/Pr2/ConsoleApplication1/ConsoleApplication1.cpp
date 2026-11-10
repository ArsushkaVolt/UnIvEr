#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

const int N = 7;

int task = 1;
int triangulationMode = 1;
int faceMode = 1;

// Структура вершины: координаты и цвет
struct Vertex
{
    float x, y, z;
    float r, g, b;
};

// Создание вершины с заданным цветом (по умолчанию белый)
Vertex makeVertex(float x, float y,
    float r = 0.9f,
    float g = 0.9f,
    float b = 0.9f)
{
    return { x, y, 0.0f, r, g, b };
}

// Создание вершины со случайным, но ярким и насыщенным цветом для темного фона
Vertex randomVertex(float x, float y)
{
    return makeVertex(
        x, y,
        0.4f + float(rand() % 60) / 100.0f, // R: 0.4 - 1.0
        0.3f + float(rand() % 70) / 100.0f, // G: 0.3 - 1.0
        0.5f + float(rand() % 50) / 100.0f  // B: 0.5 - 1.0
    );
}

// Обработка нажатий клавиш
void key_callback(GLFWwindow* window, int key,
    int scancode, int action, int mods)
{
    if (action != GLFW_PRESS)
        return;

    // Переключение между заданиями
    if (key >= GLFW_KEY_1 && key <= GLFW_KEY_8)
    {
        task = key - GLFW_KEY_0;
    }

    // Переключение способов триангуляции в задании 5
    if (task == 5)
    {
        if (key == GLFW_KEY_Q)
            triangulationMode = 1;

        if (key == GLFW_KEY_W)
            triangulationMode = 2;

        if (key == GLFW_KEY_E)
            triangulationMode = 3;
    }

    // Переключение режимов отображения в задании 8
    if (task == 8)
    {
        if (key == GLFW_KEY_Q)
            faceMode = 1;

        if (key == GLFW_KEY_W)
            faceMode = 2;

        if (key == GLFW_KEY_E)
            faceMode = 3;
    }

    // Выход из программы
    if (key == GLFW_KEY_ESCAPE)
        glfwSetWindowShouldClose(window, true);
}

// Вершинный шейдер
const char* vertexShaderSource =
"#version 400\n"
"layout(location=0) in vec3 vp;\n"
"layout(location=1) in vec3 vc;\n"
"out vec3 color;\n"
"void main(){\n"
"    gl_Position = vec4(vp, 1.0);\n"
"    color = vc;\n"
"}\n";

// Фрагментный шейдер
const char* fragmentShaderSource =
"#version 400\n"
"in vec3 color;\n"
"out vec4 frag_colour;\n"
"void main(){\n"
"    frag_colour = vec4(color, 1.0);\n"
"}\n";

// Создание программы шейдеров
GLuint createShaderProgram()
{
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexShaderSource, nullptr);
    glCompileShader(vs);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fs);

    GLint success = GL_FALSE;

    glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
    if (!success)
        std::cerr << "Ошибка компиляции вершинного шейдера\n";

    glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
    if (!success)
        std::cerr << "Ошибка компиляции фрагментного шейдера\n";

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
        std::cerr << "Ошибка компоновки программы шейдеров\n";

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

// Передача вершин в буфер и отрисовка примитива
void drawVertices(GLuint vao, GLuint vbo,
    const Vertex* vertices, int count,
    GLenum primitive)
{
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(
        GL_ARRAY_BUFFER,
        count * sizeof(Vertex),
        vertices,
        GL_DYNAMIC_DRAW
    );

    glDrawArrays(primitive, 0, count);
}

int main()
{
    srand(static_cast<unsigned int>(time(nullptr)));

    // Инициализация GLFW
    if (!glfwInit())
    {
        std::cerr << "Не удалось запустить GLFW\n";
        return 1;
    }

    // Создание окна
    GLFWwindow* window = glfwCreateWindow(
        640, 480, "Graphic Lab 2 - Variant 2",
        nullptr, nullptr
    );

    if (window == nullptr)
    {
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);

    // Инициализация GLEW
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Не удалось запустить GLEW\n";
        glfwTerminate();
        return 1;
    }

    GLuint program = createShaderProgram();

    // Создание VAO и VBO
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // Атрибут 0 — координаты вершины
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE,
        sizeof(Vertex), reinterpret_cast<void*>(0)
    );

    // Атрибут 1 — цвет вершины
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, 3, GL_FLOAT, GL_FALSE,
        sizeof(Vertex),
        reinterpret_cast<void*>(3 * sizeof(float))
    );

    // Задания 1, 2 и 6: правильный семиугольник (сделаем его бирюзовым)
    Vertex heptagon[N];

    for (int i = 0; i < N; i++)
    {
        float angle = 2.0f * 3.14159265f * i / N;

        heptagon[i] = makeVertex(
            0.65f * cos(angle),
            0.65f * sin(angle),
            0.2f, 0.8f, 0.9f // Нежный бирюзовый цвет
        );
    }

    // Задание 3: открытая ломаная (сделаем желтоватой)
    Vertex figure1[] =
    {
        makeVertex(-0.80f,  0.30f, 1.0f, 0.8f, 0.2f),
        makeVertex(-0.55f, -0.70f, 1.0f, 0.8f, 0.2f),
        makeVertex(-0.30f, -0.20f, 1.0f, 0.8f, 0.2f),
        makeVertex(0.15f, -0.20f, 1.0f, 0.8f, 0.2f),
        makeVertex(0.05f,  0.45f, 1.0f, 0.8f, 0.2f),
        makeVertex(0.80f,  0.45f, 1.0f, 0.8f, 0.2f),
        makeVertex(0.15f, -0.85f, 1.0f, 0.8f, 0.2f)
    };

    // Задание 4: замкнутая ломаная (сделаем салатовой)
    Vertex figure2[] =
    {
        makeVertex(-0.90f,  0.65f, 0.3f, 1.0f, 0.4f),
        makeVertex(0.85f,  0.65f, 0.3f, 1.0f, 0.4f),
        makeVertex(0.15f,  0.10f, 0.3f, 1.0f, 0.4f),
        makeVertex(0.15f, -0.30f, 0.3f, 1.0f, 0.4f),
        makeVertex(0.65f, -0.30f, 0.3f, 1.0f, 0.4f),
        makeVertex(0.65f, -0.80f, 0.3f, 1.0f, 0.4f),
        makeVertex(-0.65f, -0.80f, 0.3f, 1.0f, 0.4f),
        makeVertex(-0.20f, -0.10f, 0.3f, 1.0f, 0.4f)
    };

    // Задание 5а: отдельные треугольники
    Vertex triangles[] =
    {
        randomVertex(-0.90f,  0.65f),
        randomVertex(0.85f,  0.65f),
        randomVertex(0.15f,  0.10f),

        randomVertex(-0.90f,  0.65f),
        randomVertex(0.15f,  0.10f),
        randomVertex(-0.20f, -0.10f),

        randomVertex(-0.20f, -0.10f),
        randomVertex(0.15f,  0.10f),
        randomVertex(0.15f, -0.30f),

        randomVertex(-0.20f, -0.10f),
        randomVertex(0.15f, -0.30f),
        randomVertex(-0.65f, -0.80f),

        randomVertex(0.15f, -0.30f),
        randomVertex(0.65f, -0.30f),
        randomVertex(0.65f, -0.80f),

        randomVertex(0.15f, -0.30f),
        randomVertex(0.65f, -0.80f),
        randomVertex(-0.65f, -0.80f)
    };

    // Задание 5б: лента треугольников
    Vertex strip[] =
    {
        randomVertex(-0.90f,  0.65f),
        randomVertex(0.85f,  0.65f),
        randomVertex(-0.20f, -0.10f),
        randomVertex(0.15f,  0.10f),
        randomVertex(-0.65f, -0.80f),
        randomVertex(0.15f, -0.30f),
        randomVertex(0.65f, -0.80f),
        randomVertex(0.65f, -0.30f)
    };

    // Задание 5в: веер треугольников
    Vertex fan[] =
    {
        randomVertex(-0.20f, -0.10f),
        randomVertex(-0.90f,  0.65f),
        randomVertex(0.85f,  0.65f),
        randomVertex(0.15f,  0.10f),
        randomVertex(0.15f, -0.30f),
        randomVertex(0.65f, -0.30f),
        randomVertex(0.65f, -0.80f),
        randomVertex(-0.65f, -0.80f)
    };

    // Задание 7: третья фигура
    Vertex figure3[] =
    {
        randomVertex(-0.263f,  0.835f),
        randomVertex(0.660f,  0.698f),
        randomVertex(0.756f, -0.129f),

        randomVertex(0.756f, -0.129f),
        randomVertex(0.250f, -0.680f),
        randomVertex(-0.058f,  0.422f),

        randomVertex(-0.263f,  0.835f),
        randomVertex(0.756f, -0.129f),
        randomVertex(-0.058f,  0.422f),

        randomVertex(-0.776f, -0.129f),
        randomVertex(-0.263f,  0.835f),
        randomVertex(-0.058f,  0.422f),

        randomVertex(-0.776f, -0.129f),
        randomVertex(-0.058f,  0.422f),
        randomVertex(-0.468f, -0.257f),

        randomVertex(-0.468f, -0.257f),
        randomVertex(0.044f, -0.394f),
        randomVertex(-0.263f, -0.670f),

        randomVertex(-0.468f, -0.257f),
        randomVertex(-0.263f, -0.670f),
        randomVertex(-0.776f, -0.129f)
    };

    // Темный цвет фона (темно-серый / графитовый)
    glClearColor(0.12f, 0.12f, 0.14f, 1.0f);

    glEnable(GL_PROGRAM_POINT_SIZE);
    glPointSize(10.0f);
    glLineWidth(3.0f);

    // Главный цикл программы
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        switch (task)
        {
        case 1:
            glEnable(GL_POINT_SMOOTH);
            glPointSize(10.0f);

            drawVertices(
                vao, vbo, heptagon, N, GL_POINTS
            );
            break;

        case 2:
            glLineWidth(3.0f);

            drawVertices(
                vao, vbo, heptagon, N, GL_LINE_LOOP
            );
            break;

        case 3:
            glLineWidth(3.0f);

            drawVertices(
                vao, vbo, figure1,
                sizeof(figure1) / sizeof(Vertex),
                GL_LINE_STRIP
            );
            break;

        case 4:
            glLineWidth(3.0f);

            drawVertices(
                vao, vbo, figure2,
                sizeof(figure2) / sizeof(Vertex),
                GL_LINE_LOOP
            );
            break;

        case 5:
            if (triangulationMode == 1)
            {
                drawVertices(
                    vao, vbo, triangles,
                    sizeof(triangles) / sizeof(Vertex),
                    GL_TRIANGLES
                );
            }
            else if (triangulationMode == 2)
            {
                drawVertices(
                    vao, vbo, strip,
                    sizeof(strip) / sizeof(Vertex),
                    GL_TRIANGLE_STRIP
                );
            }
            else
            {
                drawVertices(
                    vao, vbo, fan,
                    sizeof(fan) / sizeof(Vertex),
                    GL_TRIANGLE_FAN
                );
            }
            break;

        case 6:
            drawVertices(
                vao, vbo, heptagon, N, GL_TRIANGLE_FAN
            );
            break;

        case 7:
            drawVertices(
                vao, vbo, figure3,
                sizeof(figure3) / sizeof(Vertex),
                GL_TRIANGLES
            );
            break;

        case 8:
            if (faceMode == 1)
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
                glPointSize(8.0f);

                drawVertices(
                    vao, vbo, figure3,
                    sizeof(figure3) / sizeof(Vertex),
                    GL_TRIANGLES
                );
            }
            else if (faceMode == 2)
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

                drawVertices(
                    vao, vbo, figure3,
                    sizeof(figure3) / sizeof(Vertex),
                    GL_TRIANGLES
                );

                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                glLineWidth(2.0f);

                drawVertices(
                    vao, vbo, figure3,
                    sizeof(figure3) / sizeof(Vertex),
                    GL_TRIANGLES
                );
            }
            else
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                glLineWidth(2.0f);

                drawVertices(
                    vao, vbo, figure3,
                    sizeof(figure3) / sizeof(Vertex),
                    GL_TRIANGLES
                );
            }

            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            break;
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(program);

    glfwTerminate();
    return 0;
}