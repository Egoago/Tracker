#include "Pipeline.h"
#include <opencv2/core/mat.hpp>
#include "../Misc/Log.h"

using namespace tr;

Pipeline::Pipeline(
    const char* name,
    const std::vector<TextureMap*>& textureMaps,
    GLenum drawPrimitive,
    GLbitfield clearMask,
    GLuint depthBuffer,
    bool drawElements) :
    textureMaps(textureMaps),
    drawPrimitive(drawPrimitive),
    clearMask(clearMask),
    drawElements(drawElements) {
    shader = new Shader(name);

    //Frame buffer
    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

    //Bind
    unsigned int size = (unsigned int)textureMaps.size();
    GLenum* drawBuffers = new GLenum[size];
    for (unsigned int i = 0; i < size; i++)
        drawBuffers[i] = textureMaps[i]->bind(i);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
    if (size > 0)
        glDrawBuffers(size, drawBuffers);
    else
        glDrawBuffer(GL_NONE);
    //glFlush();
    delete[] drawBuffers;
}

Pipeline::~Pipeline() {
    //TODO proper destructor fix
    glDeleteFramebuffers(1, &frameBuffer);
    delete shader;
}

void Pipeline::render() {
    shader->enable();
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    glClear(clearMask);
    glBindVertexArray(VAO);
    if (drawElements)
        glDrawElements(drawPrimitive, primitiveCount, GL_UNSIGNED_INT, 0);
    else
        glDrawArrays(drawPrimitive, 0, primitiveCount);
    shader->disable();
}