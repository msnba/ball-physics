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
    float mass;

    Ball(glm::vec3 position, glm::vec3 velocity, float radius) : position(position), velocity(velocity), radius(radius)
    {
        std::vector<float> vertices = GenerateCirc();
        vertCount = vertices.size() / 3;

        mass = 1.0f;

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
    void CheckWallCollision()
    {
        const float damping = 0.7f;   // energy loss
        const float stiffness = 0.0f; // 0.0f means normal bounce, 1.0f means stop as soon as it hits the ground

        // check for wall clipping
        if (position.x - radius < -1.0f)
        {
            position.x = -1.0f + radius;
            if (velocity.x < 0.0f)
                velocity.x = -velocity.x * damping;
            return;
        }
        if (position.x + radius > 1.0f)
        {
            position.x = 1.0f - radius;
            if (velocity.x > 0.0f)
                velocity.x = -velocity.x * damping;
            return;
        }
        if (position.y - radius < -1.0f)
        {
            position.y = -1.0f + radius;
            if (velocity.y < 0.0f)
                velocity.y = -velocity.y * damping;
            return;
        }
        if (position.y + radius > 1.0f)
        {
            position.y = 1.0f - radius;
            if (velocity.y > 0.0f)
                velocity.y = -velocity.y * damping;
            return;
        }

        // collision
        float overlapLeft = (-mass + radius) - position.x;
        if (overlapLeft > 0.0f)
        {
            velocity.x += calcCorrectionImpulse(overlapLeft, stiffness, damping);

            if (velocity.x < 0.0f)
            {
                velocity.x = -velocity.x * damping;
            }
        }
        float overlapRight = (position.x + radius) - mass;
        if (overlapRight > 0.0f)
        {
            velocity.x -= calcCorrectionImpulse(overlapRight, stiffness, damping);
            if (velocity.x > 0.0f)
            {
                velocity.x = -velocity.x * damping;
            }
        }

        float overlapBottom = (-mass + radius) - position.y;
        if (overlapBottom > 0.0f)
        {
            velocity.y += calcCorrectionImpulse(overlapBottom, stiffness, damping);

            if (velocity.y < 0.0f)
            {
                velocity.y = -velocity.y * damping;
            }
        }

        float overlapTop = (position.y + radius) - mass;
        if (overlapTop > 0.0f)
        {
            velocity.y -= calcCorrectionImpulse(overlapTop, stiffness, damping);

            if (velocity.y > 0.0f)
            {
                velocity.y = -velocity.y * damping;
            }
        }
    }
    void CheckBallCollision(Ball &otherBall)
    {
        const float damping = .8f;
        const float aspect = (float)WIDTH / (float)HEIGHT;

        // -- BALL-TO-BALL COLLISIONS --
        if (this == &otherBall) // check mismatch
            return;

        const float thisRadiusX = this->radius / aspect;
        const float otherRadiusX = otherBall.radius / aspect;

        glm::vec3 collision = this->position - otherBall.position;
        float distance = glm::length(collision);

        if (distance == 0.0f)
        { // avoid div by 0
            collision = glm::vec3(1.0f, 0.0f, 0.0f);
            distance = 1.0f;
        }

        float collisionDistance; // collision distance based on visual radii
        if (abs(collision.x) > abs(collision.y))
        {
            collisionDistance = thisRadiusX + otherRadiusX;
        }
        else
        {
            collisionDistance = this->radius + otherBall.radius;
        }

        if (distance >= collisionDistance) // check for collision
            return;

        float overlap = collisionDistance - distance;
        collision = collision / distance; // collision vector normalization

        float seperation = overlap * 0.5f;
        this->position += collision * seperation;
        otherBall.position -= collision * seperation;

        float aci = glm::dot(this->velocity, collision); // initial velocity before collision
        float bci = glm::dot(otherBall.velocity, collision);

        float acf = (aci + bci - damping * (aci - bci)) * 0.5f; // solve using 1-dim elastic collision equations
        float bcf = (aci + bci + damping * (aci - bci)) * 0.5f; // assumming equal masses

        this->velocity += (acf - aci) * collision; // easier than using accel
        otherBall.velocity += (bcf - bci) * collision;
    }

    void ApplyResistance() // super simple air resistance, feels like the balls are on ice if this isnt applied
    {
        const float drag = 1.0f;

        this->velocity *= (1.0f - drag * deltaTime);
    }

    glm::vec3 GetPos() const
    {
        return this->position;
    }

private:
    float calcCorrectionImpulse(float overlap, float stiffness, float damping)
    {
        return -(1.0f / deltaTime) * overlap * stiffness - velocity.x * damping; // spring-damper system
        // this only works with velocity.x i have tried velocity.y it DOES NOT WORK
        // it simplifies to -velocity.x * damping so i don't know whats up
        // it is now a feature
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

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);

    for (int i = 0; i < 30; i++)
    {
        balls.push_back(Ball(glm::vec3(dis(gen) * 0.5f, dis(gen) * 0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), 0.1f, glm::vec4(dis(gen), dis(gen), dis(gen), 1.0f)));
    }

    float aspect = (float)WIDTH / (float)HEIGHT;

    while (!glfwWindowShouldClose(window)) // main render loop
    {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        if (deltaTime > 0.016f)
            deltaTime = 0.016f;

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);
        unsigned int transformLoc = glGetUniformLocation(shaderProgram, "transform");

        for (auto &ball : balls)
        {
            ball.accel(0.0f, -9.8f * deltaTime);
            ball.ApplyResistance();
            ball.CheckWallCollision();
        }

        for (size_t i = 0; i < balls.size(); i++)
        {
            for (size_t j = i + 1; j < balls.size(); j++)
            {
                balls[i].CheckBallCollision(balls[j]);
            }
        }

        for (auto &ball : balls)
        {
            // ball.UpdateVerts();
            ball.UpdatePos();

            glm::mat4 trans = glm::translate(glm::mat4(1.0f), ball.position);
            trans = glm::scale(trans, glm::vec3(1.0f / aspect, 1.0f, 1.0f));
            glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));
            glUniform4fv(colorLoc, 1, glm::value_ptr(ball.color));

            glBindVertexArray(ball.VAO);
            glDrawArrays(GL_TRIANGLE_FAN, 0, ball.vertCount); // using triangle fan method of circle drawing
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
    GLuint vShader = glCreateShader(GL_VERTEX_SHADER); // vertex shader
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

    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER); // fragment shader
    glShaderSource(fShader, 1, &fragmentSource, nullptr);
    glCompileShader(fShader);

    glGetShaderiv(fShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(fShader, 512, nullptr, infoLog);
        std::cerr << "F Shader compilation failed: " << infoLog << std::endl;
    }

    GLuint shaderProgram = glCreateProgram(); // creating shader program
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