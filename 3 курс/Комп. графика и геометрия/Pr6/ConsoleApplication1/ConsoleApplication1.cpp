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
//  Практическая работа № 6, вариант 2
//  Радиус = 5, материалы: Бронза + Белая резина, 4 лампы
// ============================================================

const float RADIUS = 5.0f;
const int SECTOR_COUNT = 36;
const int STACK_COUNT = 18;

// -------------------- Материалы --------------------
struct Material {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;
};

Material bronze = {
    {0.2125f, 0.1275f, 0.054f},
    {0.714f,  0.4284f, 0.18144f},
    {0.393548f, 0.271906f, 0.166721f},
    25.6f
};

Material whiteRubber = {
    {0.05f, 0.05f, 0.05f},
    {0.5f,  0.5f,  0.5f},
    {0.7f,  0.7f,  0.7f},
    10.0f
};

// -------------------- Шейдеры (встроенные) --------------------
const char* vertexShaderSource = R"(
#version 400
in vec3 vertex_position;
in vec3 vertex_normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 normal;
out vec3 f_pos;

void main() {
    normal = mat3(transpose(inverse(model))) * vertex_normal;
    f_pos  = vec3(model * vec4(vertex_position, 1.0));
    gl_Position = projection * view * model * vec4(vertex_position, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 400

in vec3 normal;
in vec3 f_pos;
out vec4 frag_colour;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    float constant;
    float linear;
    float quadratic;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
    float constant;
    float linear;
    float quadratic;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform DirLight dirLight;
uniform PointLight pointLights[4];
uniform SpotLight  spotLights[4];
uniform int numPLights;
uniform int numSLights;
uniform vec3 campos;

vec3 CalcDirLight(DirLight light, vec3 norm, vec3 viewDir) {
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    vec3 ambient  = light.ambient  * material.ambient;
    vec3 diffuse  = light.diffuse  * diff * material.diffuse;
    vec3 specular = light.specular * spec * material.specular;
    return ambient + diffuse + specular;
}

vec3 CalcPointLight(PointLight light, vec3 norm, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
                               light.quadratic * distance * distance);

    vec3 ambient  = light.ambient  * material.ambient  * attenuation;
    vec3 diffuse  = light.diffuse  * diff * material.diffuse  * attenuation;
    vec3 specular = light.specular * spec * material.specular * attenuation;
    return ambient + diffuse + specular;
}

vec3 CalcSpotLight(SpotLight light, vec3 norm, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    float theta = dot(lightDir, normalize(-light.direction));

    if (theta > light.cutOff) {
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

        float distance = length(light.position - fragPos);
        float attenuation = 1.0 / (light.constant + light.linear * distance + 
                                   light.quadratic * distance * distance);

        float epsilon   = light.cutOff - light.outerCutOff;
        float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

        vec3 ambient  = light.ambient  * material.ambient;
        vec3 diffuse  = light.diffuse  * diff * material.diffuse  * attenuation * intensity;
        vec3 specular = light.specular * spec * material.specular * attenuation * intensity;
        return ambient + diffuse + specular;
    } else {
        return light.ambient * material.ambient;
    }
}

void main() {
    vec3 norm    = normalize(normal);
    vec3 viewDir = normalize(campos - f_pos);

    vec3 result = CalcDirLight(dirLight, norm, viewDir);

    for (int i = 0; i < numPLights; i++)
        result += CalcPointLight(pointLights[i], norm, f_pos, viewDir);

    for (int i = 0; i < numSLights; i++)
        result += CalcSpotLight(spotLights[i], norm, f_pos, viewDir);

    frag_colour = vec4(result, 1.0);
}
)";

// -------------------- Вспомогательные функции --------------------
GLuint CompileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[1024];
        glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
        std::cerr << "Ошибка компиляции шейдера:\n" << infoLog << std::endl;
    }
    return shader;
}

GLuint CreateProgram() {
    GLuint vs = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[1024];
        glGetProgramInfoLog(program, 1024, nullptr, infoLog);
        std::cerr << "Ошибка линковки:\n" << infoLog << std::endl;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}

void CreateSphere(std::vector<float>& vertices, std::vector<unsigned int>& indices) {
    vertices.clear();
    indices.clear();

    for (int i = 0; i <= STACK_COUNT; ++i) {
        float stackAngle = glm::pi<float>() / 2.0f - i * glm::pi<float>() / STACK_COUNT;
        float xy = RADIUS * cosf(stackAngle);
        float z = RADIUS * sinf(stackAngle);

        for (int j = 0; j <= SECTOR_COUNT; ++j) {
            float sectorAngle = j * 2.0f * glm::pi<float>() / SECTOR_COUNT;
            float x = xy * cosf(sectorAngle);
            float y = xy * sinf(sectorAngle);

            // позиция
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            // нормаль
            glm::vec3 n = glm::normalize(glm::vec3(x, y, z));
            vertices.push_back(n.x);
            vertices.push_back(n.y);
            vertices.push_back(n.z);
        }
    }

    for (int i = 0; i < STACK_COUNT; ++i) {
        int k1 = i * (SECTOR_COUNT + 1);
        int k2 = k1 + SECTOR_COUNT + 1;

        for (int j = 0; j < SECTOR_COUNT; ++j, ++k1, ++k2) {
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }
            if (i != STACK_COUNT - 1) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }
}

void SetMaterial(GLuint program, const Material& m) {
    glUniform3fv(glGetUniformLocation(program, "material.ambient"), 1, &m.ambient[0]);
    glUniform3fv(glGetUniformLocation(program, "material.diffuse"), 1, &m.diffuse[0]);
    glUniform3fv(glGetUniformLocation(program, "material.specular"), 1, &m.specular[0]);
    glUniform1f(glGetUniformLocation(program, "material.shininess"), m.shininess);
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

    GLFWwindow* window = glfwCreateWindow(1280, 720,
        "ПР №6, вариант 2 — Освещение (Бронза + Белая резина)", nullptr, nullptr);
    if (!window) {
        std::cerr << "Не удалось создать окно\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Ошибка GLEW\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.07f, 0.07f, 0.10f, 1.0f);

    GLuint program = CreateProgram();

    // ---- Сфера ----
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    CreateSphere(vertices, indices);

    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // ---- Источники света ----
    // Направленный (из таблицы варианта 2)
    glm::vec3 dirDirection = glm::normalize(glm::vec3(-0.35f, -1.0f, -0.25f));
    glm::vec3 dirAmbient = glm::vec3(0.1f, 0.4f, 0.1f);
    glm::vec3 dirDiffuse = glm::vec3(0.55f, 0.75f, 0.55f);
    glm::vec3 dirSpecular = glm::vec3(0.85f, 0.95f, 0.85f);

    // 3 точечных + 1 прожектор
    struct {
        glm::vec3 pos;
        glm::vec3 amb, dif, spe;
        float lin, quad;
    } points[3] = {
        { {-12.0f,  8.0f,  6.0f}, {0.12f,0.04f,0.04f}, {0.95f,0.35f,0.30f}, {1.0f,0.6f,0.55f}, 0.09f, 0.032f },
        { { 13.0f,  7.0f, -5.0f}, {0.04f,0.05f,0.14f}, {0.30f,0.45f,0.95f}, {0.55f,0.7f,1.0f}, 0.07f, 0.017f },
        { {  0.0f, 12.0f, 11.0f}, {0.10f,0.09f,0.04f}, {0.95f,0.85f,0.30f}, {1.0f,0.95f,0.7f}, 0.045f,0.0075f}
    };

    // Прожектор сверху
    glm::vec3 spotPos = { 0.0f, 16.0f, 0.0f };
    glm::vec3 spotDir = glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f));
    float cutOff = glm::cos(glm::radians(12.5f));
    float outerCutOff = glm::cos(glm::radians(18.0f));

    // ---- Камера ----
    glm::vec3 cameraPos = { 0.0f, 11.0f, 30.0f };
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1280.0f / 720.0f, 0.1f, 100.0f);

    // ---- Главный цикл ----
    while (!glfwWindowShouldClose(window)) {
        float t = (float)glfwGetTime();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(program);

        glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
        glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3fv(glGetUniformLocation(program, "campos"), 1, glm::value_ptr(cameraPos));

        // Направленный свет
        glUniform3fv(glGetUniformLocation(program, "dirLight.direction"), 1, &dirDirection[0]);
        glUniform3fv(glGetUniformLocation(program, "dirLight.ambient"), 1, &dirAmbient[0]);
        glUniform3fv(glGetUniformLocation(program, "dirLight.diffuse"), 1, &dirDiffuse[0]);
        glUniform3fv(glGetUniformLocation(program, "dirLight.specular"), 1, &dirSpecular[0]);

        // Точечные источники
        glUniform1i(glGetUniformLocation(program, "numPLights"), 3);
        for (int i = 0; i < 3; ++i) {
            std::string b = "pointLights[" + std::to_string(i) + "]";
            glUniform3fv(glGetUniformLocation(program, (b + ".position").c_str()), 1, &points[i].pos[0]);
            glUniform1f(glGetUniformLocation(program, (b + ".constant").c_str()), 1.0f);
            glUniform1f(glGetUniformLocation(program, (b + ".linear").c_str()), points[i].lin);
            glUniform1f(glGetUniformLocation(program, (b + ".quadratic").c_str()), points[i].quad);
            glUniform3fv(glGetUniformLocation(program, (b + ".ambient").c_str()), 1, &points[i].amb[0]);
            glUniform3fv(glGetUniformLocation(program, (b + ".diffuse").c_str()), 1, &points[i].dif[0]);
            glUniform3fv(glGetUniformLocation(program, (b + ".specular").c_str()), 1, &points[i].spe[0]);
        }

        // Прожектор
        glUniform1i(glGetUniformLocation(program, "numSLights"), 1);
        glUniform3fv(glGetUniformLocation(program, "spotLights[0].position"), 1, &spotPos[0]);
        glUniform3fv(glGetUniformLocation(program, "spotLights[0].direction"), 1, &spotDir[0]);
        glUniform1f(glGetUniformLocation(program, "spotLights[0].cutOff"), cutOff);
        glUniform1f(glGetUniformLocation(program, "spotLights[0].outerCutOff"), outerCutOff);
        glUniform1f(glGetUniformLocation(program, "spotLights[0].constant"), 1.0f);
        glUniform1f(glGetUniformLocation(program, "spotLights[0].linear"), 0.09f);
        glUniform1f(glGetUniformLocation(program, "spotLights[0].quadratic"), 0.032f);
        glUniform3fv(glGetUniformLocation(program, "spotLights[0].ambient"), 1, glm::value_ptr(glm::vec3(0.05f)));
        glUniform3fv(glGetUniformLocation(program, "spotLights[0].diffuse"), 1, glm::value_ptr(glm::vec3(0.95f, 0.95f, 0.9f)));
        glUniform3fv(glGetUniformLocation(program, "spotLights[0].specular"), 1, glm::value_ptr(glm::vec3(1.0f)));

        glBindVertexArray(VAO);

        // ===== Сфера 1 — Бронза (слева) =====
        SetMaterial(program, bronze);
        glm::mat4 model1(1.0f);
        model1 = glm::translate(model1, glm::vec3(-RADIUS, 0.0f, 0.0f));
        model1 = glm::translate(model1, glm::vec3(0.0f, 2.8f * sinf(t * 0.75f), 0.0f));
        model1 = glm::rotate(model1, t * 0.55f, glm::vec3(0, 1, 0));
        model1 = glm::rotate(model1, t * 0.25f, glm::vec3(1, 0, 0));
        glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, glm::value_ptr(model1));
        glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);

        // ===== Сфера 2 — Белая резина (справа) =====
        SetMaterial(program, whiteRubber);
        glm::mat4 model2(1.0f);
        model2 = glm::translate(model2, glm::vec3(+RADIUS, 0.0f, 0.0f));
        model2 = glm::translate(model2, glm::vec3(0.0f, 2.8f * cosf(t * 0.65f), 0.0f));
        model2 = glm::rotate(model2, -t * 0.45f, glm::vec3(0, 1, 0));
        model2 = glm::rotate(model2, t * 0.2f, glm::vec3(0, 0, 1));
        glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, glm::value_ptr(model2));
        glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(program);
    glfwTerminate();
    return 0;
}