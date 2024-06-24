//#include "stdafx.h"
#include "GLShader.h"
#define GLEW_STATIC
#include "GL/glew.h"

#include <fstream>
#include <iostream>

bool ValidateShader(GLuint shader) {
    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

    if (!compiled) {
        GLint infoLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

        if (infoLen > 1) {
            char* infoLog = new char[infoLen + 1];
            glGetShaderInfoLog(shader, infoLen, nullptr, infoLog);
            std::cout << "Error compiling shader: " << infoLog << std::endl;
            delete[] infoLog;
        }

        // Delete the shader object since it's unusable
        glDeleteShader(shader);
        return false;
    }

    return true;
}


bool GLShader::LoadVertexShader(const char* filename) {
    // Print the file path for debugging
    std::cout << "Loading vertex shader from: " << filename << std::endl;

    // Open the file
    std::ifstream fin(filename, std::ios::in | std::ios::binary);
    if (!fin) {
        std::cerr << "Failed to open vertex shader file: " << filename << std::endl;
        return false;
    }

    // Calculate the length of the file
    fin.seekg(0, std::ios::end);
    std::streamoff length = fin.tellg();
    if (length == -1) {
        std::cerr << "Failed to calculate length of vertex shader file: " << filename << std::endl;
        fin.close();
        return false;
    }
    fin.seekg(0, std::ios::beg);

    // Read the file content into a buffer
    char* buffer = new char[static_cast<size_t>(length) + 1];
    fin.read(buffer, length);
    if (!fin) {
        std::cerr << "Failed to read vertex shader file: " << filename << std::endl;
        delete[] buffer;
        fin.close();
        return false;
    }
    buffer[length] = '\0';

    // Close the file
    fin.close();

    // Create the shader object
    m_VertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(m_VertexShader, 1, &buffer, nullptr);

    // Compile the shader
    glCompileShader(m_VertexShader);

    // Clean up the buffer
    delete[] buffer;

    // Validate the shader
    return ValidateShader(m_VertexShader);
}

bool GLShader::LoadGeometryShader(const char* filename)
{
    // Load the file into memory
    std::ifstream fin(filename, std::ios::in | std::ios::binary);
    fin.seekg(0, std::ios::end);
    uint32_t length = (uint32_t)fin.tellg();
    fin.seekg(0, std::ios::beg);
    char* buffer = nullptr;
    buffer = new char[length + 1];
    buffer[length] = '\0';
    fin.read(buffer, length);

    // Create the shader object
    m_GeometryShader = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(m_GeometryShader, 1, &buffer, nullptr);
    // Compile it
    glCompileShader(m_GeometryShader);
    // Clean up
    delete[] buffer;
    fin.close(); // Not mandatory here

    // Validate the shader compilation status
    return ValidateShader(m_GeometryShader);
}

bool GLShader::LoadFragmentShader(const char* filename)
{
    std::ifstream fin(filename, std::ios::in | std::ios::binary);
    fin.seekg(0, std::ios::end);
    uint32_t length = (uint32_t)fin.tellg();
    fin.seekg(0, std::ios::beg);
    char* buffer = nullptr;
    buffer = new char[length + 1];
    buffer[length] = '\0';
    fin.read(buffer, length);

    // Create the shader object
    m_FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(m_FragmentShader, 1, &buffer, nullptr);
    // Compile it
    glCompileShader(m_FragmentShader);
    // Clean up
    delete[] buffer;
    fin.close(); // Not mandatory here

    // Validate the shader compilation status
    return ValidateShader(m_FragmentShader);
}

bool GLShader::Create()
{
    m_Program = glCreateProgram();
    glAttachShader(m_Program, m_VertexShader);
    if (m_GeometryShader)
        glAttachShader(m_Program, m_GeometryShader);
    glAttachShader(m_Program, m_FragmentShader);
    glLinkProgram(m_Program);

    int32_t linked = 0;
    int32_t infoLen = 0;
    // Check the linkage status
    glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);

    if (!linked)
    {
        glGetProgramiv(m_Program, GL_INFO_LOG_LENGTH, &infoLen);

        if (infoLen > 1)
        {
            char* infoLog = new char[infoLen + 1];
            glGetProgramInfoLog(m_Program, infoLen, NULL, infoLog);
            std::cout << "Program link error: " << infoLog << std::endl;
            delete(infoLog);
        }

        glDeleteProgram(m_Program);
        return false;
    }

    return true;
}

void GLShader::Destroy()
{
    glDetachShader(m_Program, m_VertexShader);
    glDetachShader(m_Program, m_FragmentShader);
    glDetachShader(m_Program, m_GeometryShader);
    glDeleteShader(m_GeometryShader);
    glDeleteShader(m_VertexShader);
    glDeleteShader(m_FragmentShader);
    glDeleteProgram(m_Program);
}
