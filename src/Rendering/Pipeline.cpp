#include "Pipeline.hpp"
#include <opencv2/core/mat.hpp>
#include "../Misc/Log.hpp"

using namespace tr;

Pipeline::Pipeline(
    const std::vector<std::shared_ptr<TextureMap>>& textureMaps,
    const std::vector<Shader*>& shaders,
    GLenum drawPrimitive,
    GLbitfield clearMask,
    GLuint depthBuffer,
    bool drawElements) :
    ID(glCreateProgram()),
    textureMaps(textureMaps),
    drawPrimitive(drawPrimitive),
    clearMask(clearMask),
    drawElements(drawElements) {

    glGenFramebuffers(1, &frameBuffer);
    bind(shaders, depthBuffer);
    link();
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(0);
}

Pipeline::~Pipeline() {
    glDeleteFramebuffers(1, &frameBuffer);
    glDeleteProgram(ID);
}

void Pipeline::link() {
    GLint result = GL_FALSE;
    int InfoLogLength;
    glLinkProgram(ID);
    glGetProgramiv(ID, GL_LINK_STATUS, &result);
    if (result != GL_TRUE) {
        glGetProgramiv(ID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        std::vector<char> programErrorMessage(InfoLogLength);
        glGetProgramInfoLog(ID, InfoLogLength, NULL, &programErrorMessage[0]);
        Logger::error(programErrorMessage.data());
    }
}

void Pipeline::bind(const std::vector<Shader*>& shaders, GLuint depthBuffer) {
    glUseProgram(ID);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

    //texturemaps
    uint size = (uint)textureMaps.size();
    GLenum* drawBuffers = new GLenum[size];
    for (uint i = 0; i < size; i++)
        drawBuffers[i] = textureMaps[i]->bind(i);
    if (size > 0)
        glDrawBuffers(size, drawBuffers);
    else
        glDrawBuffer(GL_NONE);
    delete[] drawBuffers;

    //depth
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

    //shaders
    for (auto& shader : shaders)
        glAttachShader(ID, shader->getID());
}
void Pipeline::render() {
    glUseProgram(ID);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    glClear(clearMask);
    glBindVertexArray(VAO);
    if (drawElements)
        glDrawElements(drawPrimitive, primitiveCount, GL_UNSIGNED_INT, 0);
    else
        glDrawArrays(drawPrimitive, 0, primitiveCount);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(0);
}

void Pipeline::uniform(const char* name, const mat4f& data) {
    glUseProgram(ID);
    GLuint id = glGetUniformLocation(ID, name);
    if (id == -1) {
        Logger::warning(
            "uniform mat4 variable "
            + std::string(name)
            + " not found in shader "
            + std::to_string(ID));
        return;
    }
    glUniformMatrix4fv(id, 1, GL_FALSE, data.data());
    glUseProgram(0);
}

void tr::Pipeline::uniform(const char* name, const vec4f& data) {
    glUseProgram(ID);
    GLuint id = glGetUniformLocation(ID, name);
    if (id == -1) {
        Logger::warning(
            "uniform vec4 variable "
            + std::string(name)
            + " not found in shader "
            + std::to_string(ID));
        return;
    }
    glUniform4fv(id, 4, data.data());
    glUseProgram(0);
}

void Pipeline::uniform(const char* name, const float value) {
    glUseProgram(ID);
    GLuint id = glGetUniformLocation(ID, name);
    if (id == -1) {
        Logger::warning(
            "uniform float variable "
            + std::string(name)
            + " not found in shader "
            + std::to_string(ID));
        return;
    }
    glUniform1f(id, value);
    glUseProgram(0);
}