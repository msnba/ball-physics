#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <iostream>
#include <vector>
#include <random>

int WIDTH = 800;
int HEIGHT = 600;

float ASPECT = (float)HEIGHT / (float)WIDTH;

float cursorX, cursorY; // in pixel coordinates
float lastCursorX, lastCursorY;
bool ballSelected = false;
bool mousePressed = false;

float deltaTime = 0.0, lastFrame = 0.0;

void CreateVV(GLuint &VAO, GLuint &VBO, const float *vertices, size_t vertCount);
glm::vec2 ConvertNDC(float x, float y);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);

class Ball

{
public:
    GLuint VAO, VBO;

    glm::vec3 position = glm::vec3(0, 0, 0); // placeholder
    glm::vec3 lastPosition = position;

    glm::vec3 velocity = glm::vec3(0, 0, 0);
    glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    size_t vertCount;

    float radius, originalRadius;
    float mass;
    float damping;

    Ball(glm::vec3 position, glm::vec3 velocity, float radius, float mass) : position(position), velocity(velocity), radius(radius), originalRadius(radius), mass(mass)
    {
        std::vector<float> vertices = GenerateCirc();
        vertCount = vertices.size() / 3;

        // mass = 1.0f;
        damping = .5f;

        CreateVV(VAO, VBO, vertices.data(), vertices.size()); // creates a VAO and VBO for each specific ball
    }

    Ball(glm::vec3 position, glm::vec3 velocity, float radius, float mass, glm::vec4 color) : Ball(position, velocity, radius, mass)
    {
        this->color = color;
    }

    std::vector<float> GenerateCirc()
    {
        std::vector<float> vertices;

        vertices.insert(vertices.end(), {0.0f, 0.0f, 0.0f});

        int quality = 50;

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
        this->lastPosition = position;
        this->position[0] += this->velocity[0] * deltaTime;
        this->position[1] += this->velocity[1] * deltaTime;
    }
    void UpdateVerts()
    {
        this->radius = originalRadius * ((float)HEIGHT/600.0f); // fixed height scaling issue by just scaling radius with height

        this->position = glm::vec3(position.x, position.y, position.z);

        // std::cout << WIDTH << " " << HEIGHT << std::endl;

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
        // check for wall clipping and apply transformations
        if (position.x - radius < -ASPECT) // LEFT
        {
            position.x = -ASPECT + radius;
            velocity.x = -velocity.x * damping;
        }
        if (position.x + radius > ASPECT) // RIGHT
        {
            position.x = ASPECT - radius;
            velocity.x = -velocity.x * damping;
        }
        if (position.y - radius < -1.0f) // BOTTOM
        {
            position.y = -1.0f + radius;
            velocity.y = -velocity.y * damping;
        }
        if (position.y + radius > 1.0f) // TOP
        {
            position.y = 1.0f - radius;
            velocity.y = -velocity.y * damping;
        }
    }
    void CheckBallCollision(Ball &otherBall)
    {

        if (this == &otherBall) // check mismatch
            return;

        glm::vec2 aPos = glm::vec2(this->position.x, this->position.y);
        glm::vec2 bPos = glm::vec2(otherBall.position.x, otherBall.position.y);

        glm::vec2 delta = aPos - bPos;
        float distance = glm::length(delta);

        if (distance == 0.0f)
        { // avoid div by 0
            delta = glm::vec2(1.0f, 0.0f);
            distance = 1.0f;
        }

        if (distance >= this->radius + otherBall.radius) // if they arent colliding
            return;

        glm::vec2 normal = delta / distance; // collision normal vector
        float overlap = this->radius + otherBall.radius - distance;
        float seperation = overlap * 0.5f;

        this->position += glm::vec3(normal.x / ASPECT, normal.y, 0.0f) * seperation;
        otherBall.position -= glm::vec3(normal.x / ASPECT, normal.y, 0.0f) * seperation;

        float va = glm::dot(glm::vec2(this->velocity.x * ASPECT, this->velocity.y), normal);         // initial velocity before collision
        float vb = glm::dot(glm::vec2(otherBall.velocity.x * ASPECT, otherBall.velocity.y), normal); // va = velocity a

        float va_f = (va * (this->mass - otherBall.mass) + 2 * otherBall.mass * vb) / (this->mass + otherBall.mass); // solve using 1-dim elastic collision equations assuming unequal masses
        float vb_f = (vb * (otherBall.mass - this->mass) + 2 * this->mass * va) / (this->mass + otherBall.mass);     // va_f = velocity a final

        glm::vec2 impulseA = (va_f - va) * normal; // change in momentum caused by collision
        glm::vec2 impulseB = (vb_f - vb) * normal; // corrects velocities along collision normal

        this->velocity += glm::vec3(impulseA.x / ASPECT, impulseA.y, 0.0f);
        otherBall.velocity += glm::vec3(impulseB.x / ASPECT, impulseB.y, 0.0f);
    }

    void ApplyResistance() // super simple air resistance, feels like the balls are on ice if this isnt applied
    {
        const float drag = 2.5f;

        this->velocity *= (1.0f - drag * deltaTime); // helps make the balls less jittery
    }
};

std::vector<Ball> balls = {};
Ball *selectedBall = nullptr;
glm::vec2 dragVelocity = glm::vec2(0.0f);

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
    WIDTH = width;
    HEIGHT = height;
    ASPECT = (float)WIDTH / (float)HEIGHT;

    for (auto &ball : balls)
    {
        ball.UpdateVerts();
    }
}

int main()
{
    if (!glfwInit())
    {
        std::cerr << "GLFW failed init!!" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);                 // version 3.3
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // using core means we'll only use/get features we actually need

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

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); // registering function to call every window resize

    glfwSetWindowSizeCallback(window, [](GLFWwindow *window, int width, int height)
                              {
                                  //   if (height != FIXED_HEIGHT)
                                  //       glfwSetWindowSize(window, width, FIXED_HEIGHT); // force back to fixed height
                              });

    glfwSetCursorPosCallback(window, [](GLFWwindow *window, double x, double y)
                             {
                                    lastCursorX = cursorX;
                                    lastCursorY = cursorY;
                                    cursorX = (float)x;
                                    cursorY = (float)y; });
    glfwSetMouseButtonCallback(window, [](GLFWwindow *win, int button, int action, int mods)
                               {
                                  if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
                                    mousePressed = true;

                                    glm::vec2 mouseNDC = ConvertNDC(cursorX,cursorY);

                                    for(auto &ball : balls){
                                        glm::vec2 center(ball.position.x,ball.position.y);

                                        if(glm::distance(mouseNDC,center)<=ball.radius){
                                            selectedBall = &ball;
                                            ballSelected = true;
                                            dragVelocity = glm::vec2(0.0f);
                                            break;
                                        }
                                    }
                                    if(selectedBall){
                                        // std::cout<<selectedBall->position.x<<std::endl;
                                    }
                                  }else if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE){
                                    mousePressed = false;

                                    if(ballSelected && selectedBall){
                                        selectedBall->velocity = glm::vec3(dragVelocity.x,dragVelocity.y,0.0f);
                                    }

                                    ballSelected = false;
                                    selectedBall = nullptr;
                                  } });

    Shader shader("basic.vert", "basic.frag"); //shader config
    shader.use(); 

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);

    for (int i = 0; i < 20; i++)
    {
        balls.push_back(Ball(glm::vec3(dis(gen) * 0.1f, dis(gen) * 0.1f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), 0.05f, 10.0f, glm::vec4(dis(gen), dis(gen), dis(gen), 1.0f)));
    }
    balls.push_back(Ball(glm::vec3(dis(gen) * 0.1f, dis(gen) * 0.1f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), 0.2f, 10.0f, glm::vec4(dis(gen), dis(gen), dis(gen), 5.0f)));

    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window)) // main render loop
    {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        if (deltaTime > 0.016f)
            deltaTime = 0.016f;

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();

        glm::mat4 projection = glm::ortho(-ASPECT, ASPECT, -1.0f, 1.0f, -1.0f, 1.0f);
        shader.setMat4("projection", glm::value_ptr(projection));

        for (auto &ball : balls)
        {
            ball.accel(0.0f, -9.8f * deltaTime);
            ball.ApplyResistance();
            ball.CheckWallCollision();
        }

        if (ballSelected && selectedBall != nullptr && mousePressed)
        {
            glm::vec2 currentMouseNDC = ConvertNDC(cursorX, cursorY);
            glm::vec2 lastMouseNDC = ConvertNDC(lastCursorX, lastCursorY);

            selectedBall->position = glm::vec3(currentMouseNDC, 0.0f);

            dragVelocity = (currentMouseNDC - lastMouseNDC) / deltaTime;
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
            ball.UpdatePos();

            glm::mat4 model = glm::translate(glm::mat4(1.0f), ball.position);
            shader.setMat4("model",glm::value_ptr(model));
            shader.setVec4("color",glm::value_ptr(ball.color));

            glBindVertexArray(ball.VAO);
            glDrawArrays(GL_TRIANGLE_FAN, 0, (GLsizei)ball.vertCount); // using triangle fan method of circle drawing
        }

        glfwSwapBuffers(window); // swap frame buffers every refresh, using double buffering
        glfwPollEvents();        // sync events
    }

    for (auto &ball : balls)
    {
        glDeleteVertexArrays(1, &ball.VAO);
        glDeleteBuffers(1, &ball.VBO);
    }

    glfwTerminate();
    return 0;
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

glm::vec2 ConvertNDC(float x, float y) // converts pixel values to ndc
{
    return glm::vec2(((2.0f * x) / WIDTH - 1.0f) * ASPECT, -(2.0f * y) / HEIGHT + 1.0f);
}