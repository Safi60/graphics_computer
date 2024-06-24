#define _USE_MATH_DEFINES
#include <cmath>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include "GLShader.h"
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

// Aliases for GLM types
using glm::mat4;
using glm::vec2;
using glm::vec3;

// External optimus settings
extern "C" {
    uint32_t NvOptimusEnablement = 0x00000001;
}

// Type alias for color
using Color = vec3;

// Struct for 2D vertex
struct Vertex2 {
    vec2 position;
    Color color;
    vec2 texCoords;
};

// Struct for 3D vertex
struct Vertex3 {
    vec3 position;
    vec3 normal;
    vec2 texCoords;
};

// Constants
const float PI = static_cast<float>(M_PI);
const float DEG_TO_RAD = PI / 180;
const float RAD_TO_DEG = 180 / PI;
const float EPSILON = 0.01f;
const float MOVEMENT_SPEED = 0.1f;

// Function for cotangent
float cotan(float x) {
    return cos(x) / sin(x);
}

// LookAt function
mat4 LookAt(vec3 position, vec3 target, vec3 up) {
    vec3 forward = glm::normalize(position - target);
    vec3 right = glm::normalize(glm::cross(up, forward));
    vec3 up2 = glm::cross(forward, right);
    return {
        right.x, up2.x, forward.x, 0,
        right.y, up2.y, forward.y, 0,
        right.z, up2.z, forward.z, 0,
        -glm::dot(right, position), -glm::dot(up2, position), -glm::dot(forward, position), 1
    };
}

// Rotation functions
mat4 RotateX(float angle) {
    float rad = angle * DEG_TO_RAD;
    return {
        1, 0, 0, 0,
        0, cos(rad), -sin(rad), 0,
        0, sin(rad), cos(rad), 0,
        0, 0, 0, 1
    };
}

mat4 RotateY(float angle) {
    float rad = angle * DEG_TO_RAD;
    return {
        cos(rad), 0, sin(rad), 0,
        0, 1, 0, 0,
        -sin(rad), 0, cos(rad), 0,
        0, 0, 0, 1
    };
}

mat4 RotateZ(float angle) {
    float rad = angle * DEG_TO_RAD;
    return {
        cos(rad), -sin(rad), 0, 0,
        sin(rad), cos(rad), 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
}

class Application;

class Obj {
public:
    Application& app;
    GLShader shader;
    GLuint buffers[3] = { 0, 0, 0 };
    GLuint vao = 0;
    GLuint texture = 0;
    int numOfIndices = 0;
    tinyobj::material_t material;
    vec3 scale = { 1, 1, 1 };
    float angle = 0;
    vec3 translation = { 0, 0, 0 };
    std::string name;

    explicit Obj(Application& app, const std::string& name = "") : app(app), name(name) {}

    void initialize(const char* shaderFileV, const char* shaderFileF, const std::string& objFile, const char* textureFile);
    void render();
    void destroy();
    inline uint32_t getProgram() {
        return this->shader.GetProgram();
    }
};

class Application {
public:
    int width;
    int height;
    GLShader basicShader;
    GLuint pausedBuffers[2] = { 0, 0 };
    GLuint pausedVao = 0;
    GLuint pausedTexture = 0;
    GLFWwindow* window = nullptr;
    double lastMouseX = 0;
    double lastMouseY = 0;
    float cameraPhi = PI / 2;
    float cameraTheta = 0;
    float cameraR = 50;
    vec3 target = { 0, 15, 0 };
    vec3 cameraPosition = { 0, 0, 0 };
    mat4 camera = {};
    mat4 projection = {};
    bool canMove = false;
    GLFWcursor* handCursor = nullptr;

    std::vector<Obj> objects;

    Application(int width, int height) : width(width), height(height) {}

    inline void setSize(int width, int height) {
        this->width = width;
        this->height = height;
    }

    bool initialize(GLFWwindow* window);
    void renderPaused();
    void render();
    void deinitialize();
    inline uint32_t getBasicProgram() {
        return this->basicShader.GetProgram();
    }
};

// Implementation of Obj methods
void Obj::initialize(const char* shaderFileV, const char* shaderFileF, const std::string& objFile, const char* textureFile) {
    // File paths
    std::string vertexShaderPath = std::string("C:\\Users\\Safi\\Desktop\\computer-grphics-ESIEE\\OpenGL-OBJ-Renderer\\Projet\\shaders\\") + shaderFileV;
    std::string fragmentShaderPath = std::string("C:\\Users\\Safi\\Desktop\\computer-grphics-ESIEE\\OpenGL-OBJ-Renderer\\Projet\\shaders\\") + shaderFileF;
    std::string objFilePath = std::string("C:\\Users\\Safi\\Desktop\\computer-grphics-ESIEE\\OpenGL-OBJ-Renderer\\Projet\\Obj\\") + objFile;
    std::string textureFilePath = std::string("C:\\Users\\Safi\\Desktop\\computer-grphics-ESIEE\\OpenGL-OBJ-Renderer\\Projet\\Obj\\") + textureFile;

    // Load shaders
    std::cout << "Trying to open vertex shader file: " << vertexShaderPath << std::endl;
    this->shader.LoadVertexShader(vertexShaderPath.c_str());
    std::cout << "Trying to open fragment shader file: " << fragmentShaderPath << std::endl;
    this->shader.LoadFragmentShader(fragmentShaderPath.c_str());
    this->shader.Create();

    // Load OBJ file
    std::cout << "Trying to open OBJ file: " << objFilePath << std::endl;
    tinyobj::ObjReader reader;
    if (!reader.ParseFromFile(objFilePath)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader(" << objFilePath << "): " << reader.Error();
        }
        exit(1);
    }
    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader(" << objFilePath << "): " << reader.Warning();
    }

    // Process OBJ data
    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();
    size_t verticesCount = 0;
    for (const auto& shape : shapes)
        for (const auto& num_face_vertice : shape.mesh.num_face_vertices)
            verticesCount += size_t(num_face_vertice);
    this->numOfIndices = int(verticesCount);

    std::vector<float> objVertices(8 * verticesCount);
    std::vector<int> objIndices(verticesCount);

    size_t vertex_offset = 0;
    size_t index_offset = 0;
    for (const auto& shape : shapes) {
        size_t shape_index_offset = 0;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
            auto fv = size_t(shape.mesh.num_face_vertices[f]);
            for (size_t v = 0; v < fv; v++) {
                tinyobj::index_t idx = shape.mesh.indices[shape_index_offset + v];
                objVertices[8 * vertex_offset + 0] = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                objVertices[8 * vertex_offset + 1] = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
                objVertices[8 * vertex_offset + 2] = -attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                if (idx.normal_index >= 0) {
                    objVertices[8 * vertex_offset + 3] = attrib.normals[3 * size_t(idx.normal_index) + 0];
                    objVertices[8 * vertex_offset + 4] = attrib.normals[3 * size_t(idx.normal_index) + 2];
                    objVertices[8 * vertex_offset + 5] = -attrib.normals[3 * size_t(idx.normal_index) + 1];
                }
                if (idx.texcoord_index >= 0) {
                    objVertices[8 * vertex_offset + 6] = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                    objVertices[8 * vertex_offset + 7] = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
                }
                objIndices[index_offset] = int(vertex_offset);
                vertex_offset++;
                index_offset++;
            }
            shape_index_offset += fv;
        }
    }
    this->material = materials[0];

    // Set up OpenGL buffers and arrays
    uint32_t prog = this->getProgram();
    glGenBuffers(3, this->buffers);
    glGenVertexArrays(1, &this->vao);
    glBindVertexArray(this->vao);
    glBindBuffer(GL_ARRAY_BUFFER, this->buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, objVertices.size() * sizeof(float), objVertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->buffers[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, objIndices.size() * sizeof(int), objIndices.data(), GL_STATIC_DRAW);
    const int32_t PROG_POSITION = glGetAttribLocation(prog, "position");
    const int32_t PROG_NORMAL = glGetAttribLocation(prog, "normal");
    const int32_t PROG_TEX_COORDS = glGetAttribLocation(prog, "texCoords");
    glEnableVertexAttribArray(PROG_POSITION);
    glEnableVertexAttribArray(PROG_NORMAL);
    glEnableVertexAttribArray(PROG_TEX_COORDS);
    glVertexAttribPointer(PROG_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3), (void*)offsetof(Vertex3, position));
    glVertexAttribPointer(PROG_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3), (void*)offsetof(Vertex3, normal));
    glVertexAttribPointer(PROG_TEX_COORDS, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex3), (void*)offsetof(Vertex3, texCoords));
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Load texture
    int w, h;
    uint8_t* data = stbi_load(textureFilePath.c_str(), &w, &h, nullptr, STBI_rgb_alpha);
    if (!data) {
        std::cerr << "Failed to load texture: " << textureFilePath << std::endl;
        exit(1);
    }
    glGenTextures(1, &this->texture);
    glBindTexture(GL_TEXTURE_2D, this->texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
}

void Obj::render() {
    auto time = static_cast<float>(glfwGetTime());
    uint32_t prog = this->getProgram();
    glUseProgram(prog);

    const int32_t PROG_TIME = glGetUniformLocation(prog, "time");
    const int32_t PROG_SAMPLER = glGetUniformLocation(prog, "sampler_");
    const int32_t PROG_LIGHT_DIRECTION = glGetUniformLocation(prog, "light.direction");
    const int32_t PROG_LIGHT_AMBIENT_COLOR = glGetUniformLocation(prog, "light.ambientColor");
    const int32_t PROG_LIGHT_DIFFUSE_COLOR = glGetUniformLocation(prog, "light.diffuseColor");
    const int32_t PROG_LIGHT_SPECULAR_COLOR = glGetUniformLocation(prog, "light.specularColor");
    const int32_t PROG_MATERIAL_AMBIENT_COLOR = glGetUniformLocation(prog, "material.ambientColor");
    const int32_t PROG_MATERIAL_DIFFUSE_COLOR = glGetUniformLocation(prog, "material.diffuseColor");
    const int32_t PROG_MATERIAL_SPECULAR_COLOR = glGetUniformLocation(prog, "material.specularColor");
    const int32_t PROG_SHININESS = glGetUniformLocation(prog, "shininess");

    glUniform1f(PROG_TIME, time);
    glUniform1i(PROG_SAMPLER, 0);
    glUniform3f(PROG_LIGHT_DIRECTION, 1, -1, -1);
    glUniform3f(PROG_LIGHT_AMBIENT_COLOR, 0.1, 0.1, 0.1);
    glUniform3f(PROG_LIGHT_DIFFUSE_COLOR, 1, 1, 1);
    glUniform3f(PROG_LIGHT_SPECULAR_COLOR, 0.5, 0.5, 0.5);
    glUniform3f(PROG_MATERIAL_AMBIENT_COLOR, this->material.ambient[0], this->material.ambient[1], this->material.ambient[2]);
    glUniform3f(PROG_MATERIAL_DIFFUSE_COLOR, this->material.diffuse[0], this->material.diffuse[1], this->material.diffuse[2]);
    glUniform3f(PROG_MATERIAL_SPECULAR_COLOR, this->material.specular[0], this->material.specular[1], this->material.specular[2]);
    glUniform1f(PROG_SHININESS, this->material.shininess);

    mat4 scaleMatrix = {
        this->scale.x, 0, 0, 0,
        0, this->scale.y, 0, 0,
        0, 0, this->scale.z, 0,
        0, 0, 0, 1,
    };

    mat4 rotationMatrix = glm::mat4(1.0f);
    if (this->name == "map") {
        mat4 rotationMatrixX = RotateX(this->angle);
        mat4 rotationMatrixY = RotateY(180); // Rotate 180 degrees to flip
        mat4 rotationMatrixZ = RotateZ(180); // Rotate 180 degrees to correct upside-down
        rotationMatrix = rotationMatrixZ * rotationMatrixY * rotationMatrixX;
    }
    else if (this->name == "mrbean" || this->name == "botle") {
        rotationMatrix = RotateX(-90);  // Rotate to make them stand up
    }
    else {
        rotationMatrix = glm::rotate(glm::mat4(1.0f), this->angle * DEG_TO_RAD, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    mat4 translationMatrix = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        this->translation.x, this->translation.y, this->translation.z, 1,
    };

    const int32_t PROG_VIEW = glGetUniformLocation(prog, "view");
    glUniform3f(PROG_VIEW, this->app.cameraPosition.x, this->app.cameraPosition.y, this->app.cameraPosition.z);

    mat4 transform = translationMatrix * rotationMatrix * scaleMatrix;
    mat4 transformNormal = glm::transpose(glm::inverse(transform));
    mat4 transformWithProjection = this->app.projection * this->app.camera * transform;

    const int32_t PROG_TRANSFORM_NORMAL = glGetUniformLocation(prog, "transformNormal");
    glUniformMatrix4fv(PROG_TRANSFORM_NORMAL, 1, GL_FALSE, glm::value_ptr(transformNormal));
    const int32_t PROG_TRANSFORM_WITH_PROJECTION = glGetUniformLocation(prog, "transformWithProjection");
    glUniformMatrix4fv(PROG_TRANSFORM_WITH_PROJECTION, 1, GL_FALSE, glm::value_ptr(transformWithProjection));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->texture);
    glBindVertexArray(this->vao);
    glDrawElements(GL_TRIANGLES, this->numOfIndices, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

void Obj::destroy() {
    glDeleteBuffers(2, this->buffers);
    glDeleteVertexArrays(1, &this->vao);
    glDeleteTextures(1, &this->texture);
    this->shader.Destroy();
}

// Implementation of Application methods
bool Application::initialize(GLFWwindow* window) {
    this->window = window;
    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_FRAMEBUFFER_SRGB);
    std::cout << "Graphic card: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "OpenGL: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLEW: " << glewGetString(GLEW_VERSION) << std::endl;
    std::cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    std::cout << "Extensions: " << glGetString(GL_EXTENSIONS) << std::endl;

    this->basicShader.LoadVertexShader("C:\\Users\\Safi\\Desktop\\computer-grphics-ESIEE\\OpenGL-OBJ-Renderer\\Projet\\shaders\\basic.vs.glsl");
    this->basicShader.LoadFragmentShader("C:\\Users\\Safi\\Desktop\\computer-grphics-ESIEE\\OpenGL-OBJ-Renderer\\Projet\\shaders\\basic.fs.glsl");
    this->basicShader.Create();
    uint32_t basic = this->getBasicProgram();

    // Initialize objects
    Obj table(*this);
    table.initialize("3d.vs.glsl", "3d.fs.glsl", "Meshes\\dinertable.obj", "Textures\\table.png");
    table.translation = { 0, 0, 0 };
    table.scale = { 1, 1, 1 };
    this->objects.push_back(table);

    Obj bottle(*this, "botle");
    bottle.initialize("3d.vs.glsl", "3d.fs.glsl", "Meshes/Botle.obj", "Textures/Bottle.png");
    bottle.translation = { -20, 50, 10 };
    bottle.scale = { 1, 1, 1 };
    this->objects.push_back(bottle);

    Obj nolegs(*this);
    nolegs.initialize("3d.vs.glsl", "3d.fs.glsl", "Meshes/nolegs.obj", "Textures/nolegs.png");
    nolegs.translation = { 10, 115, 10 };
    nolegs.scale = { 3, 3, 3 };
    this->objects.push_back(nolegs);

    Obj pirate(*this);
    pirate.initialize("3d.vs.glsl", "3d_blink.fs.glsl", "Meshes/Stylized_pirate_scene.obj", "Textures/Barrel_BaseColor_2K.png");
    pirate.translation = { -110, -10, -20 };
    pirate.scale = { 1.5, 1.5, 1.5 };
    this->objects.push_back(pirate);

    Obj mrbean(*this, "mrbean");
    mrbean.initialize("3d.vs.glsl", "3d.fs.glsl", "Meshes/Mr_Bean_Pirate.obj", "Textures/Tex_0013_0.png");
    mrbean.translation = { 0, 0, -50 };
    mrbean.scale = { 80, 80, 80 };
    this->objects.push_back(mrbean);

    Obj map(*this, "map");
    map.initialize("3d.vs.glsl", "3d.fs.glsl", "Meshes/Wooden.obj", "Textures/WoodenTexture.png");
    map.translation = { 40, 56, 10 };
    map.scale = { 3, 3, 3 };
    map.angle = 90;
    this->objects.push_back(map);

    // Set up paused screen
    const Vertex2 pausedVertex[] = {
        { { -0.265f, +0.8f }, { 1, 1, 1 }, { 0, 1 } },
        { { -0.265f, +0.7f }, { 1, 1, 1 }, { 0, 0 } },
        { { +0.265f, +0.7f }, { 1, 1, 1 }, { 1, 0 } },
        { { +0.265f, +0.8f }, { 1, 1, 1 }, { 1, 1 } },
    };
    const unsigned int pausedIndices[] = { 0, 1, 2, 0, 2, 3 };

    glGenBuffers(2, this->pausedBuffers);
    glGenVertexArrays(1, &this->pausedVao);
    glBindVertexArray(this->pausedVao);
    glBindBuffer(GL_ARRAY_BUFFER, this->pausedBuffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex2) * 4, pausedVertex, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->pausedBuffers[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 6, pausedIndices, GL_STATIC_DRAW);
    const int32_t BASIC_POSITION = glGetAttribLocation(basic, "position");
    const int32_t BASIC_COLOR = glGetAttribLocation(basic, "color");
    const int32_t BASIC_TEX_COORDS = glGetAttribLocation(basic, "texCoords");
    glEnableVertexAttribArray(BASIC_POSITION);
    glEnableVertexAttribArray(BASIC_COLOR);
    glEnableVertexAttribArray(BASIC_TEX_COORDS);
    glVertexAttribPointer(BASIC_POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2), (void*)offsetof(Vertex2, position));
    glVertexAttribPointer(BASIC_COLOR, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex2), (void*)offsetof(Vertex2, color));
    glVertexAttribPointer(BASIC_TEX_COORDS, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2), (void*)offsetof(Vertex2, texCoords));
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Load paused screen texture
    int w, h;
    uint8_t* data = stbi_load("C:\\Users\\Safi\\Desktop\\computer-grphics-ESIEE\\OpenGL-OBJ-Renderer\\Projet\\paused.png", &w, &h, nullptr, STBI_rgb_alpha);
    if (!data) return false;
    glGenTextures(1, &this->pausedTexture);
    glBindTexture(GL_TEXTURE_2D, this->pausedTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    this->handCursor = glfwCreateStandardCursor(GLFW_HAND_CURSOR);

    // Set GLFW callbacks
    glfwSetKeyCallback(this->window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));
            app->canMove = !app->canMove;
        }
        });
    glfwSetMouseButtonCallback(this->window, [](GLFWwindow* window, int button, int action, int mods) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));
            app->canMove = true;
        }
        });
    glfwSetScrollCallback(this->window, [](GLFWwindow* window, double xoffset, double yoffset) {
        auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if (app->canMove)
            app->cameraR = glm::clamp(app->cameraR - static_cast<float>(yoffset) * 0.5f, 1.f, 500.f);
        });
    glfwGetCursorPos(this->window, &this->lastMouseX, &this->lastMouseY);
    this->canMove = true;

    return true;
}

void Application::renderPaused() {
    uint32_t basic = this->getBasicProgram();
    glUseProgram(basic);
    const int32_t BASIC_TIME = glGetUniformLocation(basic, "time");
    const int32_t BASIC_SAMPLER = glGetUniformLocation(basic, "sampler_");
    glUniform1f(BASIC_TIME, 0);
    glUniform1i(BASIC_SAMPLER, 0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->pausedTexture);
    glBindVertexArray(this->pausedVao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    glDisable(GL_BLEND);
}

void Application::render() {
    bool clicked = glfwGetMouseButton(this->window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    glfwSetCursor(this->window, clicked ? this->handCursor : nullptr);

    double mouseX, mouseY;
    glfwGetCursorPos(this->window, &mouseX, &mouseY);
    auto motionX = clicked ? static_cast<float>(this->lastMouseX - mouseX) : 0;
    auto motionY = clicked ? static_cast<float>(this->lastMouseY - mouseY) : 0;
    this->lastMouseX = mouseX;
    this->lastMouseY = mouseY;

    vec3 movement = { 0, 0, 0 };
    if (this->canMove) {
        if (glfwGetKey(this->window, GLFW_KEY_UP) == GLFW_PRESS)
            movement.x -= MOVEMENT_SPEED;
        if (glfwGetKey(this->window, GLFW_KEY_DOWN) == GLFW_PRESS)
            movement.x += MOVEMENT_SPEED;
        if (glfwGetKey(this->window, GLFW_KEY_SEMICOLON) == GLFW_PRESS)
            movement.y -= MOVEMENT_SPEED;
        if (glfwGetKey(this->window, GLFW_KEY_SPACE) == GLFW_PRESS)
            movement.y += MOVEMENT_SPEED;
        if (glfwGetKey(this->window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            movement.z -= MOVEMENT_SPEED;
        if (glfwGetKey(this->window, GLFW_KEY_LEFT) == GLFW_PRESS)
            movement.z += MOVEMENT_SPEED;
    }

    if (this->canMove) {
        this->cameraPhi = glm::mod(this->cameraPhi - motionX * 0.005f + PI, 2 * PI) - PI;
        this->cameraTheta = glm::clamp(this->cameraTheta - motionY * 0.005f, -PI / 2 + EPSILON, PI / 2 - EPSILON);
    }
    glm::mat3 movementRotation = {
        cos(this->cameraPhi), 0, sin(this->cameraPhi),
        0, 1, 0,
        -sin(this->cameraPhi), 0, cos(this->cameraPhi),
    };
    this->target += movementRotation * movement;

    float aspect = static_cast<float>(this->width) / static_cast<float>(this->height);
    float near = 0.01, far = 500;
    float fovY = 55 * DEG_TO_RAD;
    float f = cotan(fovY / 2);
    this->projection = {
        f / aspect, 0, 0, 0,
        0, f, 0, 0,
        0, 0, (far + near) / (near - far), -1,
        0, 0, 2 * near * far / (near - far), 0,
    };
    vec3 rawCameraPosition = {
        this->cameraR * cos(this->cameraTheta) * cos(this->cameraPhi),
        this->cameraR * sin(this->cameraTheta),
        this->cameraR * cos(this->cameraTheta) * sin(this->cameraPhi)
    };
    this->cameraPosition = this->target + rawCameraPosition;
    this->camera = LookAt(this->cameraPosition, this->target, { 0, 1, 0 });

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (Obj& object : this->objects)
        object.render();

    if (!this->canMove)
        this->renderPaused();
}

void Application::deinitialize() {
    for (Obj& object : this->objects)
        object.destroy();

    glDeleteBuffers(2, this->pausedBuffers);
    glDeleteVertexArrays(1, &this->pausedVao);
    glDeleteTextures(1, &this->pausedTexture);
    this->basicShader.Destroy();

    glfwDestroyCursor(this->handCursor);
}

int main() {
    Application app(1280, 960);
    GLFWwindow* window;

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    window = glfwCreateWindow(app.width, app.height, "Safi HANIFA, Steeve RABEHANTA, Yanis AIT TAOUIT & Charles BATCHAEV - ESIEE Paris E4FI GR3 - 2023-2024", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwSetWindowUserPointer(window, &app);

    glfwMakeContextCurrent(window);

    glewInit();

    if (!app.initialize(window)) {
        glfwTerminate();
        return -1;
    }

    while (!glfwWindowShouldClose(window)) {
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        app.setSize(width, height);
        app.render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    app.deinitialize();
    glfwTerminate();
    return 0;
}
