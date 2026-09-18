#include <GL/glew.h>  // Подключаем GLEW (загрузчик расширений OpenGL)
#include <GLFW/glfw3.h> // Подключаем GLFW (управление окнами и вводом)
#include <iostream>


// 1. ШЕЙДЕРЫ (GLSL)

// Вершинный шейдер: обрабатывает геометрию (вершины)
const char* vertexShaderSource = "#version 460 core\n"
"layout (location = 0) in vec3 aPos;\n"   // Атрибут 0: Координаты вершины (X, Y, Z)
"layout (location = 1) in vec3 aColor;\n" // Атрибут 1: Цвет вершины (R, G, B)
"out vec3 ourColor;\n"                   // Выходной интерполируемый цвет для фрагментного шейдера
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos, 1.0);\n"     // Преобразуем vec3 в vec4 (w = 1.0) и передаем в OpenGL
"   ourColor = aColor;\n"                // Пробрасываем цвет дальше по конвейеру
"}\n";

// Фрагментный шейдер: вычисляет итоговый цвет каждого пикселя
const char* fragmentShaderSource = "#version 460 core\n"
"in vec3 ourColor;\n"                   // Входной цвет от вершинного шейдера
"out vec4 FragColor;\n"                 // Итоговый цвет пикселя (RGBA)
"void main()\n"
"{\n"
"   FragColor = vec4(ourColor, 1.0f);\n" // Задаем цвет с альфа-каналом (прозрачностью) = 1.0
"}\n";

int main() {
   
    // 2. ИНИЦИАЛИЗАЦИЯ И НАСТРОЙКА ОКНА (GLFW)
   

 
    if (!glfwInit()) return -1;

    // Настраиваем профиль и версию OpenGL (4.6 Core Profile)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Создаем окно приложения размером 800x600 пикселей
    GLFWwindow* window = glfwCreateWindow(800, 600, "Colored Triangles", NULL, NULL);
    if (!window) {
        glfwTerminate(); // Завершаем работу, если окно не создалось
        return -1;
    }

    // Делаем контекст созданного окна текущим для вызывающих потоков
    glfwMakeContextCurrent(window);

  
    // 3. ИНИЦИАЛИЗАЦИЯ ЗАГРУЗЧИКА ФУНКЦИЙ (GLEW)
  

    glewExperimental = GL_TRUE; // Разрешаем использование соврменных функций OpenGL
    if (glewInit() != GLEW_OK) {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // Устанавливаем область вывода (Viewport) под размеры созданного окна
    glViewport(0, 0, 800, 600);

  
    // 4. КОМПИЛЯЦИЯ И СБОРКА ШЕЙДЕРОВ
   

    // Создаем и компилируем вершинный шейдер
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // Создаем и компилируем фрагментный шейдер
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // Создаем шейдерную программу и связываем шейдеры в единое целое
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Удаляем исходные объекты шейдеров, так как они уже скомпонованы в программу
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);


    // 5. ПОДГОТОВКА ДАННЫХ И БУФЕРОВ (VAO, VBO)
  

    // Массив вершин: 9 вершин (по 3 на треугольник).
    // Каждая вершина описывается 6 числами: PosX, PosY, PosZ, ColorR, ColorG, ColorB
    GLfloat vertices[] = {
        // --- 1-й треугольник (Красный) ---
        // Позиция            // Цвет (RGB)
        -0.3f,  0.1f, 0.0f,   1.0f, 0.0f, 0.0f,
         0.3f,  0.1f, 0.0f,   1.0f, 0.0f, 0.0f,
         0.0f,  0.7f, 0.0f,   1.0f, 0.0f, 0.0f,

         // --- 2-й треугольник (Зеленый) ---
         -0.8f, -0.7f, 0.0f,   0.0f, 1.0f, 0.0f,
         -0.2f, -0.7f, 0.0f,   0.0f, 1.0f, 0.0f,
         -0.5f, -0.1f, 0.0f,   0.0f, 1.0f, 0.0f,

         // --- 3-й треугольник (Синий) ---
          0.2f, -0.7f, 0.0f,   0.0f, 0.0f, 1.0f,
          0.8f, -0.7f, 0.0f,   0.0f, 0.0f, 1.0f,
          0.5f, -0.1f, 0.0f,   0.0f, 0.0f, 1.0f
    };

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO); // Генерируем ID для Vertex Array Object (VAO)
    glGenBuffers(1, &VBO);      // Генерируем ID для Vertex Buffer Object (VBO)

    // Привязываем VAO для записи конфигурации атрибутов
    glBindVertexArray(VAO);

    // Привязываем VBO и копируем данные массива в видеопамять
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Вычисляем шаг (stride) — разница в байтах между начальными элементами двух вершин
    GLsizei stride = 6 * sizeof(float);

    // Настраиваем layout 0 (Координаты): 3 элемента float, отступ от начала = 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    // Настраиваем layout 1 (Цвет): 3 элемента float, смещение в вершине = 3 * sizeof(float)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Отвязываем буферы для предотвращения случайных изменений
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

 
    // 6. ГЛАВНЫЙ ИГРОВОЙ ЦИКЛ (RENDER LOOP)
   

    while (!glfwWindowShouldClose(window)) {
        // Очищаем экран фоновым темным цветом
        glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Активируем шейдерную программу
        glUseProgram(shaderProgram);

        // Привязываем сохраненную конфигурацию вершин
        glBindVertexArray(VAO);

        // Отрисовываем 9 вершин как треугольники
        glDrawArrays(GL_TRIANGLES, 0, 9);

        // Меняем задний буфер кадра с передним (double buffering)
        glfwSwapBuffers(window);

        // Обрабатываем события ввода (клавиатура, мышь, закрытие окна)
        glfwPollEvents();
    }

  
    // 7. ОСВОБОЖДЕНИЕ РЕСУРСОВ
     

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    // Завершаем работу GLFW и освобождаем все ресурсы
    glfwTerminate();
    return 0;
}