// Практическая работа № 4 — Вариант 2
// Трехмерные построения. Буфер глубины. Видовые параметры.
// Параллельная и перспективная проекции.

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

// ======================== Шейдеры ========================
const char* vertex_shader_src = R"(
#version 400
layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_colour;

uniform mat4 M;
uniform mat4 P;

out vec3 colour;

void main() {
    colour = vertex_colour;
    gl_Position = P * M * vec4(vertex_position, 1.0);
}
)";

const char* fragment_shader_src = R"(
#version 400
in vec3 colour;
out vec4 frag_colour;

void main() {
    frag_colour = vec4(colour, 1.0);
}
)";

// ======================== Глобальные переменные ========================
int   WinWidth = 900;
int   WinHeight = 700;

float scale = 1.0f;
bool  use_perspective = false;
bool  wireframe = false;
bool  depth_test = true;
bool  wide_view = true;
int   mode = 1;

// ======================== Колбэки ========================
void window_size_callback(GLFWwindow* window, int width, int height) {
    WinWidth = width;
    WinHeight = height;
    glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;

    switch (key) {
    case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window, 1);
        break;

    case GLFW_KEY_UP:
        scale += 0.1f;
        if (scale > 3.0f) scale = 3.0f;
        break;

    case GLFW_KEY_DOWN:
        scale -= 0.1f;
        if (scale < 0.2f) scale = 0.2f;
        break;

    case GLFW_KEY_1:
        mode = 1;
        break;

    case GLFW_KEY_2:
        mode = 2;
        break;

    case GLFW_KEY_D:
        depth_test = !depth_test;
        std::cout << "Depth test: " << (depth_test ? "ON" : "OFF") << std::endl;
        break;

    case GLFW_KEY_P:
        use_perspective = !use_perspective;
        std::cout << "Projection: " << (use_perspective ? "Perspective" : "Ortho") << std::endl;
        break;

    case GLFW_KEY_W:
        wireframe = !wireframe;
        std::cout << "Wireframe: " << (wireframe ? "ON" : "OFF") << std::endl;
        break;

    case GLFW_KEY_V:
        wide_view = !wide_view;
        std::cout << "View volume: " << (wide_view ? "WIDE" : "VARIANT") << std::endl;
        break;

    case GLFW_KEY_R:
        scale = 1.0f;
        break;
    }
}

// ======================== Вспомогательные функции ========================
GLuint compile_shader(GLenum type, const char* src) {
    GLuint sh = glCreateShader(type);
    glShaderSource(sh, 1, &src, nullptr);
    glCompileShader(sh);

    GLint success;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(sh, 512, nullptr, log);
        std::cerr << "Shader compile error:\n" << log << std::endl;
    }
    return sh;
}

GLuint create_program() {
    GLuint vs = compile_shader(GL_VERTEX_SHADER, vertex_shader_src);
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fragment_shader_src);

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);

    GLint success;
    glGetProgramiv(prog, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(prog, 512, nullptr, log);
        std::cerr << "Program link error:\n" << log << std::endl;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

void create_vao(GLuint& vao, const GLfloat* points, const GLfloat* colors, int vertex_count) {
    GLuint vbo_pos, vbo_col;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo_pos);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
    glBufferData(GL_ARRAY_BUFFER, vertex_count * 3 * sizeof(GLfloat), points, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &vbo_col);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_col);
    glBufferData(GL_ARRAY_BUFFER, vertex_count * 3 * sizeof(GLfloat), colors, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

// ======================== Данные геометрии (Вариант 2) ========================

GLfloat figures_points[] = {
    // 1. Четырёхугольник
    -0.5f, -0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,
     0.8f,  0.5f, -0.5f,

    -0.5f, -0.5f, -0.5f,
     0.8f,  0.5f, -0.5f,
     0.5f, -0.8f, -0.5f,

     // 2. Треугольник 1 (зелёный)
     -1.0f, -1.0f,  1.0f,
     -1.0f,  1.0f,  1.0f,
      1.0f,  0.0f, -1.0f,

      // 3. Треугольник 2 (голубой)
      -0.7f, -0.7f, -0.7f,
       0.0f,  0.0f,  0.0f,
       0.0f,  0.0f,  1.0f
};

GLfloat figures_colors[] = {
    // Четырёхугольник — оранжевый
    1.0f, 0.4f, 0.0f,  1.0f, 0.4f, 0.0f,  1.0f, 0.4f, 0.0f,
    1.0f, 0.4f, 0.0f,  1.0f, 0.4f, 0.0f,  1.0f, 0.4f, 0.0f,

    // Треугольник 1 — зелёный
    0.1f, 0.9f, 0.2f,  0.1f, 0.9f, 0.2f,  0.1f, 0.9f, 0.2f,

    // Треугольник 2 — голубой
    0.1f, 0.6f, 1.0f,  0.1f, 0.6f, 1.0f,  0.1f, 0.6f, 1.0f
};

GLfloat cube_points[] = {
    // Передняя
    -1,-1, 1,   1,-1, 1,   1, 1, 1,
    -1,-1, 1,   1, 1, 1,  -1, 1, 1,
    // Задняя
    -1,-1,-1,  -1, 1,-1,   1, 1,-1,
    -1,-1,-1,   1, 1,-1,   1,-1,-1,
    // Левая
    -1,-1,-1,  -1,-1, 1,  -1, 1, 1,
    -1,-1,-1,  -1, 1, 1,  -1, 1,-1,
    // Правая
     1,-1,-1,   1, 1,-1,   1, 1, 1,
     1,-1,-1,   1, 1, 1,   1,-1, 1,
     // Верхняя
     -1, 1,-1,  -1, 1, 1,   1, 1, 1,
     -1, 1,-1,   1, 1, 1,   1, 1,-1,
     // Нижняя
     -1,-1,-1,   1,-1,-1,   1,-1, 1,
     -1,-1,-1,   1,-1, 1,  -1,-1, 1
};

GLfloat cube_colors[] = {
    // Перед — красный
    1,0.2f,0.2f, 1,0.2f,0.2f, 1,0.2f,0.2f,
    1,0.2f,0.2f, 1,0.2f,0.2f, 1,0.2f,0.2f,
    // Зад — зелёный
    0.2f,1,0.2f, 0.2f,1,0.2f, 0.2f,1,0.2f,
    0.2f,1,0.2f, 0.2f,1,0.2f, 0.2f,1,0.2f,
    // Лево — синий
    0.2f,0.4f,1, 0.2f,0.4f,1, 0.2f,0.4f,1,
    0.2f,0.4f,1, 0.2f,0.4f,1, 0.2f,0.4f,1,
    // Право — жёлтый
    1,1,0.2f, 1,1,0.2f, 1,1,0.2f,
    1,1,0.2f, 1,1,0.2f, 1,1,0.2f,
    // Верх — голубой
    0.2f,1,1, 0.2f,1,1, 0.2f,1,1,
    0.2f,1,1, 0.2f,1,1, 0.2f,1,1,
    // Низ — пурпурный
    1,0.2f,1, 1,0.2f,1, 1,0.2f,1,
    1,0.2f,1, 1,0.2f,1, 1,0.2f,1
};

// ======================== MAIN ========================
int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to init GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(WinWidth, WinHeight,
        "Практическая работа 4 — Вариант 2", nullptr, nullptr);

    if (!window) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetWindowSizeCallback(window, window_size_callback);
    glfwSetKeyCallback(window, key_callback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to init GLEW\n";
        return -1;
    }

    GLuint program = create_program();
    glUseProgram(program);

    GLint loc_M = glGetUniformLocation(program, "M");
    GLint loc_P = glGetUniformLocation(program, "P");

    GLuint vao_figures, vao_cube;
    create_vao(vao_figures, figures_points, figures_colors, 12);
    create_vao(vao_cube, cube_points, cube_colors, 36);

    glClearColor(0.15f, 0.15f, 0.18f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    std::cout << "=============================================\n";
    std::cout << "  Практическая работа № 4  —  Вариант 2\n";
    std::cout << "=============================================\n";
    std::cout << "  1          — фигуры\n";
    std::cout << "  2          — куб\n";
    std::cout << "  D          — буфер глубины\n";
    std::cout << "  P          — орто / перспектива\n";
    std::cout << "  W          — каркас / заливка\n";
    std::cout << "  V          — широкий объём / параметры варианта\n";
    std::cout << "  UP / DOWN  — масштаб\n";
    std::cout << "  R          — сброс масштаба\n";
    std::cout << "  ESC        — выход\n";
    std::cout << "=============================================\n";

    while (!glfwWindowShouldClose(window)) {
        glViewport(0, 0, WinWidth, WinHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (wireframe)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // ---------- Матрица проекции ----------
        glm::mat4 P;
        if (use_perspective) {
            P = glm::perspective(glm::radians(45.0f),
                (float)WinWidth / (float)WinHeight, 0.1f, 100.0f);
        }
        else {
            if (wide_view) {
                P = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, -10.0f, 20.0f);
            }
            else {
                // Параметры варианта 2
                P = glm::ortho(-0.9f, 0.8f, -0.8f, 0.9f, 5.0f, 10.0f);
            }
        }

        // ---------- Матрица модели ----------
        glm::mat4 M = glm::mat4(1.0f);

        if (mode == 1) {
            // Поворот, чтобы все три фигуры были хорошо видны
            M = glm::rotate(M, glm::radians(35.0f), glm::vec3(0, 1, 0));
            M = glm::rotate(M, glm::radians(25.0f), glm::vec3(1, 0, 0));
        }
        else {
            M = glm::scale(M, glm::vec3(scale, scale, scale));
            M = glm::rotate(M, glm::radians(45.0f), glm::vec3(1, 0, 0));
            M = glm::rotate(M, glm::radians(45.0f), glm::vec3(0, 1, 0));
        }

        glUniformMatrix4fv(loc_M, 1, GL_FALSE, glm::value_ptr(M));
        glUniformMatrix4fv(loc_P, 1, GL_FALSE, glm::value_ptr(P));

        // ---------- Отрисовка ----------
        // ---------- Отрисовка ----------
        if (depth_test)
            glEnable(GL_DEPTH_TEST);
        else
            glDisable(GL_DEPTH_TEST);

        if (mode == 1) {
            glBindVertexArray(vao_figures);
            glDrawArrays(GL_TRIANGLES, 0, 12);
        }
        else {
            glBindVertexArray(vao_cube);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vao_figures);
    glDeleteVertexArrays(1, &vao_cube);
    glDeleteProgram(program);

    glfwTerminate();
    return 0;
}