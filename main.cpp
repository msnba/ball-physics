#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <iostream>
#include <vector>
#include <random>

const int WIDTH = 800;
const int HEIGHT = 600;

const char *vertexShaderSource = R"glsl(#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 transform;

void main()
{
    gl_Position = transform * vec4(aPos, 1.0);
};)glsl";

const char *fragmentShaderSource = R"glsl(
#version 330 core
out vec4 FragColor;
uniform vec4 color;
void main()
{
    FragColor = color;
};)glsl";

float deltaTime = 0.0, lastFrame = 0.0;

GLuint CreateShaderP(const char *vertexSource, const char *fragmentSource);
void CreateVV(GLuint &VAO, GLuint &VBO, const float *vertices, size_t vertCount);

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

class Ball

{
public:
    GLuint VAO, VBO;

    glm::vec3 position = glm::vec3(0, 0, 0); // placeholder
    glm::vec3 velocity = glm::vec3(0, 0, 0);
    glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    size_t vertCount;

    float radius;
    float damping; // gravity damping

    Ball(glm::vec3 position, glm::vec3 velocity, float radius) : position(position), velocity(velocity), radius(radius)
    {
        std::vector<float> vertices = GenerateCirc();
        vertCount = vertices.size() / 3;

        damping = .8f; // energy loss

        CreateVV(VAO, VBO, vertices.data(), vertices.size()); // creates a VAO and VBO for each specific ball
    }

    Ball(glm::vec3 position, glm::vec3 velocity, float radius, glm::vec4 color) : Ball(position, velocity, radius)
    {
        this->color = color;
    }

    std::vector<float> GenerateCirc()
    {
        std::vector<float> vertices;

        vertices.insert(vertices.end(), {0.0f, 0.0f, 0.0f});

        int quality = 70;

        for (int i = 0; i <= quality; i++)
        {
            double angle = 2.0f * M_PI * i / quality; // radians * i/quality
            float x = radius * (float)cos(angle);
            float y = radius * (float)sin(angle);
            vertices.insert(vertices.end(), {x, y, 0.0f});
        }

        return vertices;
    }

    void UpdatePos()
    {
        this->position[0] += this->velocity[0] * deltaTime;
        this->position[1] += this->velocity[1] * deltaTime;
    }
    void UpdateVerts()
    {
        std::vector<float> vertices = GenerateCirc();
        vertCount = vertices.size() / 3;

        // update with new vertex data
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    }
    void accel(float x, float y)
    {
        this->velocity[0] += x;
        this->velocity[1] += y;
    }
    float CheckForCollision(Ball &otherBall) // kinda jittery but will fix later
    {
        const float epsilon = 0.0001f;

        // Left wall
        if (position.x - radius < -1.0f && velocity.x < 0.0f)
        {
            position.x = -1.0f + radius + epsilon;
            velocity.x = -damping * velocity.x;
        }
        // Right wall
        else if (position.x + radius > 1.0f && velocity.x > 0.0f)
        {
            position.x = 1.0f - radius - epsilon;
            velocity.x = -damping * velocity.x;
        }

        // Bottom wall
        if (position.y - radius < -1.0f && velocity.y < 0.0f)
        {
            position.y = -1.0f + radius + epsilon;
            velocity.y = -damping * velocity.y;
        }
        // Top wall
        else if (position.y + radius > 1.0f && velocity.y > 0.0f)
        {
            position.y = 1.0f - radius - epsilon;
            velocity.y = -damping * velocity.y;
        }

        return 0.0f;
    }

    glm::vec3 GetPos() const
    {
        return this->position;
    }
};

std::vector<Ball> balls = {}; // haha

int main()
{
    if (!glfwInit())
    {
        std::cerr << "GLFW failed init!! oh no" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);                 // version 3.3
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // using core means we'll only use/get features we actually need
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Window", NULL, NULL);

    if (!window)
    {
        std::cout << "Failed window creation!" << std::endl;
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed GLAD initialization!" << std::endl;
        return -1;
    }

    GLuint shaderProgram = CreateShaderP(vertexShaderSource, fragmentShaderSource);
    int colorLoc = glGetUniformLocation(shaderProgram, "color");

    glEnable(GL_DEPTH_TEST);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); // registering function to call every window resize

    // balls = {
    //     Ball(glm::vec3(-0.25f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), 1.0f, 0.05f, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)),
    //     Ball(glm::vec3(0.25f, 0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), 1.0f, 0.05f, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)),
    //     Ball(glm::vec3(-0.25f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), 1.0f, 0.05f, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f))};

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);

    for (int i = 0; i < 20; i++)
    {
        balls.push_back(Ball(glm::vec3(dis(gen) * 0.5f, dis(gen) * 0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), ((float)dis(gen) + 0.4f) * 0.1f, glm::vec4(dis(gen), dis(gen), dis(gen), 1.0f)));
    }

    float aspect = (float)WIDTH / (float)HEIGHT;
    int flip = -1;

    while (!glfwWindowShouldClose(window)) // main render loop
    {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);
        unsigned int transformLoc = glGetUniformLocation(shaderProgram, "transform");

        for (auto &ball : balls)
        {
            // ball.UpdateVerts();

            // logic down here

            if (abs(ball.velocity.x) < 0.05f) // wow great grav flip mechanism
                flip *= -1;

            ball.accel(flip * 9.8f * deltaTime, flip * 9.8f * deltaTime);
            for (auto &ball2 : balls)
            {
                if (&ball2 != &ball)
                {
                    ball.CheckForCollision(ball2);
                }
            }

            ball.UpdatePos();

            glm::mat4 trans = glm::translate(glm::mat4(1.0f), ball.position);
            trans = glm::scale(trans, glm::vec3(1.0f / aspect, 1.0f, 1.0f));
            glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));
            glUniform4fv(colorLoc, 1, glm::value_ptr(ball.color));

            glBindVertexArray(ball.VAO);
            glDrawArrays(GL_TRIANGLE_FAN, 0, ball.vertCount);
        }

        glfwSwapBuffers(window); // swap frame buffers every refresh, using double buffering
        glfwPollEvents();        // sync events
    }

    for (auto &ball : balls)
    {
        glDeleteVertexArrays(1, &ball.VAO);
        glDeleteBuffers(1, &ball.VBO);
    }

    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}

GLuint CreateShaderP(const char *vertexSource, const char *fragmentSource)
{
    GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vShader, 1, &vertexSource, nullptr);
    glCompileShader(vShader);

    GLint success;
    glGetShaderiv(vShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(vShader, 512, nullptr, infoLog);
        std::cerr << "V Shader compilation failed: " << infoLog << std::endl;
    }

    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fShader, 1, &fragmentSource, nullptr);
    glCompileShader(fShader);

    glGetShaderiv(fShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(fShader, 512, nullptr, infoLog);
        std::cerr << "F Shader compilation failed: " << infoLog << std::endl;
    }

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vShader);
    glAttachShader(shaderProgram, fShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader link failed: " << infoLog << std::endl;
    }

    int colorLoc = glGetUniformLocation(shaderProgram, "color");

    glDeleteShader(vShader);
    glDeleteShader(fShader);

    return shaderProgram;
}

void CreateVV(GLuint &VAO, GLuint &VBO, const float *vertices, size_t vertCount)
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, vertCount * sizeof(float), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}